#include "MyUnorderedSet.h"
#include "MyUnorderedMap.h"
#include "MyUnorderedMultiSet.h"
#include "MyUnorderedMultiMap.h"
#include "../../Common/TestUtils.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {

using test_support::require;

void test_set_unique_and_collision() {
    MyUnorderedSet<int> values;
    values.Insert(1);
    values.Insert(9); // Both keys initially map to the same bucket.
    values.Insert(1);
    require(values.Size() == 2, "set accepted a duplicate key");
    require(values.Find(1) != nullptr && values.Find(9) != nullptr, "set lookup failed");
    values.Remove(1);
    require(values.Find(1) == nullptr && values.Find(9) != nullptr, "collision removal failed");
    require(values.Size() == 1, "set size after removal is wrong");
}

void test_multiset_duplicates() {
    MyUnorderedMultiSet<int> values;
    values.Insert(5);
    values.Insert(5);
    values.Insert(13);
    require(values.Size() == 3, "multiset rejected a duplicate");
    values.Remove(5);
    require(values.Size() == 1 && values.Find(5) == nullptr, "multiset did not remove all matches");
    require(values.Find(13) != nullptr, "multiset removed a colliding key");
}

void test_map_access() {
    MyUnorderedMap<int, std::string> values;
    values.Insert({2, "two"});
    values.Insert({2, "duplicate"});
    require(values.Size() == 1 && values.at(2) == "two", "map duplicate changed the value");
    values[3] = "three";
    require(values.Size() == 2 && values.at(3) == "three", "map subscript failed");
    bool threw = false;
    try { values.at(99); } catch (const std::runtime_error&) { threw = true; }
    require(threw, "map at did not reject a missing key");
}

void test_multimap_duplicates() {
    MyUnorderedMultiMap<int, std::string> values;
    values.Insert({4, "first"});
    values.Insert({4, "second"});
    values.Insert({12, "collision"});
    require(values.Size() == 3 && values.Find(4) != nullptr, "multimap insertion failed");
    values.Remove(4);
    require(values.Size() == 1 && values.Find(4) == nullptr, "multimap did not remove all matches");
    require(values.Find(12) != nullptr, "multimap removed a colliding key");
}

void test_rehash_and_reserve() {
    MyUnorderedSet<int> values;
    for (int key = 0; key < 128; ++key) values.Insert(key);
    values.Reserve(512);
    values.Rehash(1024);
    require(values.Size() == 128, "rehash changed the element count");
    for (int key = 0; key < 128; ++key)
        require(values.Find(key) != nullptr, "rehash lost a key");
}

void test_iteration() {
    MyUnorderedSet<int> values;
    values.Insert(1);
    values.Insert(9);
    values.Insert(17);
    int count = 0;
    int sum = 0;
    for (auto it = values.begin(); it != values.end(); ++it) {
        ++count;
        sum += *it;
    }
    require(count == 3 && sum == 27, "iteration did not visit every element");
}

void test_copy() {
    MyUnorderedMap<int, std::string> source;
    source.Insert({1, "one"});
    source.Insert({9, "nine"});
    MyUnorderedMap<int, std::string> copy(source);
    source.Remove(1);
    require(copy.Size() == 2 && copy.at(1) == "one", "copy shares elements with source");

    MyUnorderedMap<int, std::string> assigned;
    assigned = copy;
    assigned = assigned;
    require(assigned.Size() == 2 && assigned.at(9) == "nine", "copy assignment failed");
}

void test_move_and_reuse() {
    MyUnorderedSet<int> source;
    source.Insert(1);
    source.Insert(9);
    MyUnorderedSet<int> moved(std::move(source));
    require(moved.Size() == 2 && moved.Find(9) != nullptr, "move construction lost elements");
    require(source.Size() == 0, "move-constructed source is not empty");
    source.Insert(17);
    require(source.Find(17) != nullptr, "move-constructed source cannot be reused");

    MyUnorderedSet<int> assigned;
    assigned.Insert(100);
    assigned = std::move(moved);
    require(assigned.Size() == 2 && assigned.Find(1) != nullptr, "move assignment lost elements");
    require(moved.Size() == 0, "move-assigned source is not empty");
    assigned = std::move(assigned);
    require(assigned.Size() == 2, "self move assignment changed the object");
    moved.Insert(25);
    require(moved.Find(25) != nullptr, "move-assigned source cannot be reused");
}

void test_empty_move() {
    MyUnorderedSet<int> source;
    MyUnorderedSet<int> moved(std::move(source));
    require(moved.Size() == 0 && !(moved.begin() != moved.end()), "empty move construction failed");
    moved.Insert(1);
    source.Insert(2);
    require(moved.Find(1) != nullptr && source.Find(2) != nullptr, "empty moved objects cannot be reused");

    MyUnorderedSet<int> empty;
    moved = std::move(empty);
    require(moved.Size() == 0 && !(moved.begin() != moved.end()), "empty move assignment failed");
    moved.Insert(3);
    empty.Insert(4);
    require(moved.Find(3) != nullptr && empty.Find(4) != nullptr, "empty move assignment broke reuse");
}

struct LookupTimes {
    long long average_us;
    long long median_us;
};

template<typename Lookup>
LookupTimes measure_lookup(int elements, int repetitions, Lookup lookup) {
    std::vector<long long> times;
    times.reserve(repetitions);
    volatile long long observed = 0;
    for (int repeat = 0; repeat < repetitions; ++repeat) {
        const auto start = std::chrono::steady_clock::now();
        for (int key = 0; key < elements; ++key) {
            observed += lookup(key) ? 1 : 0;
            observed += lookup(elements + key) ? 1 : 0;
        }
        const auto end = std::chrono::steady_clock::now();
        times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    }
    require(observed == static_cast<long long>(elements) * repetitions, "benchmark lookup result mismatch");
    std::sort(times.begin(), times.end());
    const long long total = std::accumulate(times.begin(), times.end(), 0LL);
    const int middle = repetitions / 2;
    const long long median = repetitions % 2
        ? times[middle] : (times[middle - 1] + times[middle]) / 2;
    return {total / repetitions, median};
}

unsigned custom_bucket_count(unsigned requested) {
    unsigned buckets = 8;
    while (buckets < requested) buckets <<= 1;
    return buckets;
}

void run_load_factor_benchmark(int elements, int repetitions) {
    const float factors[] = {0.25f, 0.5f, 1.0f, 2.0f};
    std::cout << "Load-factor lookup benchmark\n"
              << "  Build           : " << test_support::build_configuration() << " x64\n"
              << "  Elements        : " << elements << '\n'
              << "  Repetitions     : " << repetitions << " per container/factor\n"
              << "  Workload        : " << elements << " hits + " << elements << " misses per repetition\n"
              << "  Timed operation : Find / unordered_set::find only\n"
              << "  Time unit       : microseconds (us)\n"
              << "  Columns         : target LF, actual LF, buckets, average us, median us\n\n";

    for (float factor : factors) {
        const unsigned requested = static_cast<unsigned>(std::ceil(static_cast<double>(elements) / factor));
        const unsigned custom_buckets = custom_bucket_count(requested);

        MyUnorderedSet<int> custom;
        custom.DisableAutoReHash();
        custom.Rehash(requested);
        std::unordered_set<int> standard;
        standard.max_load_factor(factor);
        standard.rehash(requested);
        for (int key = 0; key < elements; ++key) {
            custom.Insert(key);
            standard.insert(key);
        }
        const auto custom_times = measure_lookup(elements, repetitions,
            [&custom](int key) { return custom.Find(key) != nullptr; });
        const auto standard_times = measure_lookup(elements, repetitions,
            [&standard](int key) { return standard.find(key) != standard.end(); });

        std::cout << "Target load factor: " << factor << '\n'
                  << "  MyUnorderedSet     : " << factor << ", "
                  << static_cast<float>(elements) / custom_buckets << ", "
                  << custom_buckets << ", " << custom_times.average_us << ", "
                  << custom_times.median_us << '\n'
                  << "  std::unordered_set : " << factor << ", "
                  << standard.load_factor() << ", " << standard.bucket_count() << ", "
                  << standard_times.average_us << ", " << standard_times.median_us << "\n\n";
    }
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "benchmark") {
        const int elements = argc > 2 ? std::stoi(argv[2]) : 8192;
        const int repetitions = argc > 3 ? std::stoi(argv[3]) : 15;
        if (elements < 1 || repetitions < 1 || elements > 1000000 || repetitions > 10000) {
            std::cerr << "Usage: MyUnorderedHash.exe benchmark [elements: 1..1000000] [repetitions: 1..10000]\n";
            return 1;
        }
        run_load_factor_benchmark(elements, repetitions);
        return 0;
    }
    test_support::TestRunner runner(9);
    runner.run("Set uniqueness and bucket collision", test_set_unique_and_collision);
    runner.run("MultiSet duplicate removal", test_multiset_duplicates);
    runner.run("Map access and missing key", test_map_access);
    runner.run("MultiMap duplicate removal", test_multimap_duplicates);
    runner.run("Rehash and reserve", test_rehash_and_reserve);
    runner.run("Iteration across inserted elements", test_iteration);
    runner.run("Copy construction, assignment, and self-assignment", test_copy);
    runner.run("Move construction, assignment, and reuse", test_move_and_reuse);
    runner.run("Empty move construction and assignment", test_empty_move);
    return runner.report();
}
