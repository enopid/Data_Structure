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
#include <random>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {

using test_support::require;

void test_set_unique_and_collision() {
    MyUnorderedSet<int> values;
    values.insert(1);
    values.insert(9); // Both keys initially map to the same bucket.
    values.insert(1);
    require(values.size() == 2, "set accepted a duplicate key");
    require(values.find(1) != nullptr && values.find(9) != nullptr, "set lookup failed");
    values.remove(1);
    require(values.find(1) == nullptr && values.find(9) != nullptr, "collision removal failed");
    require(values.size() == 1, "set size after removal is wrong");
}

void test_multiset_duplicates() {
    MyUnorderedMultiSet<int> values;
    values.insert(5);
    values.insert(5);
    values.insert(13);
    require(values.size() == 3, "multiset rejected a duplicate");
    values.remove(5);
    require(values.size() == 1 && values.find(5) == nullptr, "multiset did not remove all matches");
    require(values.find(13) != nullptr, "multiset removed a colliding key");
}

void test_map_access() {
    MyUnorderedMap<int, std::string> values;
    values.insert({2, "two"});
    values.insert({2, "duplicate"});
    require(values.size() == 1 && values.at(2) == "two", "map duplicate changed the value");
    values[3] = "three";
    require(values.size() == 2 && values.at(3) == "three", "map subscript failed");
    bool threw = false;
    try { values.at(99); } catch (const std::runtime_error&) { threw = true; }
    require(threw, "map at did not reject a missing key");
}

void test_multimap_duplicates() {
    MyUnorderedMultiMap<int, std::string> values;
    values.insert({4, "first"});
    values.insert({4, "second"});
    values.insert({12, "collision"});
    require(values.size() == 3 && values.find(4) != nullptr, "multimap insertion failed");
    values.remove(4);
    require(values.size() == 1 && values.find(4) == nullptr, "multimap did not remove all matches");
    require(values.find(12) != nullptr, "multimap removed a colliding key");
}

void test_rehash_and_reserve() {
    MyUnorderedSet<int> values;
    for (int key = 0; key < 128; ++key) values.insert(key);
    values.reserve(512);
    values.rehash(1024);
    require(values.size() == 128, "rehash changed the element count");
    for (int key = 0; key < 128; ++key)
        require(values.find(key) != nullptr, "rehash lost a key");
}

void test_iteration() {
    MyUnorderedSet<int> values;
    values.insert(1);
    values.insert(9);
    values.insert(17);
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
    source.insert({1, "one"});
    source.insert({9, "nine"});
    MyUnorderedMap<int, std::string> copy(source);
    source.remove(1);
    require(copy.size() == 2 && copy.at(1) == "one", "copy shares elements with source");

    MyUnorderedMap<int, std::string> assigned;
    assigned = copy;
    assigned = assigned;
    require(assigned.size() == 2 && assigned.at(9) == "nine", "copy assignment failed");
}

void test_move_and_reuse() {
    MyUnorderedSet<int> source;
    source.insert(1);
    source.insert(9);
    MyUnorderedSet<int> moved(std::move(source));
    require(moved.size() == 2 && moved.find(9) != nullptr, "move construction lost elements");
    require(source.size() == 0, "move-constructed source is not empty");
    source.insert(17);
    require(source.find(17) != nullptr, "move-constructed source cannot be reused");

    MyUnorderedSet<int> assigned;
    assigned.insert(100);
    assigned = std::move(moved);
    require(assigned.size() == 2 && assigned.find(1) != nullptr, "move assignment lost elements");
    require(moved.size() == 0, "move-assigned source is not empty");
    assigned = std::move(assigned);
    require(assigned.size() == 2, "self move assignment changed the object");
    moved.insert(25);
    require(moved.find(25) != nullptr, "move-assigned source cannot be reused");
}

void test_empty_move() {
    MyUnorderedSet<int> source;
    MyUnorderedSet<int> moved(std::move(source));
    require(moved.size() == 0 && !(moved.begin() != moved.end()), "empty move construction failed");
    moved.insert(1);
    source.insert(2);
    require(moved.find(1) != nullptr && source.find(2) != nullptr, "empty moved objects cannot be reused");

    MyUnorderedSet<int> empty;
    moved = std::move(empty);
    require(moved.size() == 0 && !(moved.begin() != moved.end()), "empty move assignment failed");
    moved.insert(3);
    empty.insert(4);
    require(moved.find(3) != nullptr && empty.find(4) != nullptr, "empty move assignment broke reuse");
}

void test_multiset_bucket_becomes_empty() {
    MyUnorderedMultiSet<int> values;
    values.insert(5);
    values.insert(5);
    values.remove(5);
    require(values.size() == 0, "removing every duplicate did not empty the table");
    require(values.find(5) == nullptr, "removed duplicate key is still present");
    require(!(values.begin() != values.end()), "empty multiset iteration is invalid");
}

void test_randomized_against_std_unordered_set() {
    MyUnorderedSet<int> actual;
    std::unordered_set<int> expected;
    std::mt19937 random(20260925);

    const auto verify_all = [&] {
        require(actual.size() == static_cast<int>(expected.size()), "randomized size mismatch");
        std::unordered_set<int> observed;
        int visited = 0;
        for (auto it = actual.begin(); it != actual.end(); ++it) {
            observed.insert(*it);
            ++visited;
        }
        require(visited == actual.size(), "iteration count differs from Size");
        require(observed == expected, "randomized contents mismatch");
    };

    for (int step = 0; step < 50000; ++step) {
        const int base = static_cast<int>(random() % 4096);
        const int key = base + static_cast<int>(random() % 16) * 65536;
        const int operation = static_cast<int>(random() % 4);
        if (operation == 0) {
            actual.insert(key);
            expected.insert(key);
        } else if (operation == 1) {
            actual.remove(key);
            expected.erase(key);
        } else if (operation == 2) {
            require((actual.find(key) != nullptr) == (expected.find(key) != expected.end()),
                    "randomized Find mismatch");
        } else if (step % 997 == 0) {
            const unsigned requested = static_cast<unsigned>(expected.size() * 2 + 8);
            actual.reserve(requested);
            expected.reserve(requested);
        } else {
            const int missing = key + 1;
            require((actual.find(missing) != nullptr) == (expected.find(missing) != expected.end()),
                    "randomized missing-key lookup mismatch");
        }

        require(actual.size() == static_cast<int>(expected.size()), "randomized size mismatch");
        if (step % 100 == 0) verify_all();
    }
    verify_all();
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
        custom.disable_auto_rehash();
        custom.rehash(requested);
        std::unordered_set<int> standard;
        standard.max_load_factor(factor);
        standard.rehash(requested);
        for (int key = 0; key < elements; ++key) {
            custom.insert(key);
            standard.insert(key);
        }
        const auto custom_times = measure_lookup(elements, repetitions,
            [&custom](int key) { return custom.find(key) != nullptr; });
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

struct PhaseTimes {
    long long insert_us = 0;
    long long find_us = 0;
    long long remove_us = 0;

    long long total_us() const { return insert_us + find_us + remove_us; }
};

struct BenchmarkSummary {
    PhaseTimes average;
    PhaseTimes median;
};

void insert_value(MyUnorderedSet<int>& values, int key) { values.insert(key); }
void insert_value(std::unordered_set<int>& values, int key) { values.insert(key); }
bool contains_value(MyUnorderedSet<int>& values, int key) { return values.find(key) != nullptr; }
bool contains_value(std::unordered_set<int>& values, int key) {
    return values.find(key) != values.end();
}
void remove_value(MyUnorderedSet<int>& values, int key) { values.remove(key); }
void remove_value(std::unordered_set<int>& values, int key) { values.erase(key); }
int container_size(MyUnorderedSet<int>& values) { return values.size(); }
int container_size(std::unordered_set<int>& values) { return static_cast<int>(values.size()); }

template<typename Set>
PhaseTimes run_benchmark_once(const std::vector<int>& keys) {
    Set values;
    PhaseTimes times;

    auto start = std::chrono::steady_clock::now();
    for (int key : keys) insert_value(values, key);
    auto end = std::chrono::steady_clock::now();
    times.insert_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    require(container_size(values) == static_cast<int>(keys.size()),
            "benchmark insertion failed");

    start = std::chrono::steady_clock::now();
    int found = 0;
    for (int key : keys) found += contains_value(values, key);
    end = std::chrono::steady_clock::now();
    times.find_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    require(found == static_cast<int>(keys.size()), "benchmark lookup failed");

    start = std::chrono::steady_clock::now();
    for (int key : keys) remove_value(values, key);
    end = std::chrono::steady_clock::now();
    times.remove_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    require(container_size(values) == 0, "benchmark removal failed");
    return times;
}

PhaseTimes median_phase_times(const std::vector<PhaseTimes>& values) {
    const auto median_of = [&values](auto member) {
        std::vector<long long> samples;
        samples.reserve(values.size());
        for (const auto& value : values) samples.push_back(value.*member);
        std::sort(samples.begin(), samples.end());
        const std::size_t middle = samples.size() / 2;
        return samples.size() % 2 == 1
            ? samples[middle]
            : (samples[middle - 1] + samples[middle]) / 2;
    };
    return {
        median_of(&PhaseTimes::insert_us),
        median_of(&PhaseTimes::find_us),
        median_of(&PhaseTimes::remove_us)
    };
}

template<typename Set>
BenchmarkSummary measure_benchmark(const std::vector<int>& keys, int repetitions) {
    std::vector<PhaseTimes> samples;
    samples.reserve(static_cast<std::size_t>(repetitions));
    PhaseTimes total;
    for (int repeat = 0; repeat < repetitions; ++repeat) {
        const auto current = run_benchmark_once<Set>(keys);
        samples.push_back(current);
        total.insert_us += current.insert_us;
        total.find_us += current.find_us;
        total.remove_us += current.remove_us;
    }
    return {
        {total.insert_us / repetitions, total.find_us / repetitions,
         total.remove_us / repetitions},
        median_phase_times(samples)
    };
}

void print_benchmark_result(const char* name, const BenchmarkSummary& result) {
    std::cout << name << '\n'
              << "  Average (us) : insert=" << result.average.insert_us
              << ", find=" << result.average.find_us
              << ", remove=" << result.average.remove_us
              << ", total=" << result.average.total_us() << '\n'
              << "  Median  (us) : insert=" << result.median.insert_us
              << ", find=" << result.median.find_us
              << ", remove=" << result.median.remove_us
              << ", total=" << result.median.total_us() << "\n\n";
}

void run_benchmark(int elements, int repetitions) {
    std::vector<int> keys(static_cast<std::size_t>(elements));
    std::iota(keys.begin(), keys.end(), 0);
    std::mt19937 random(20260925);
    std::shuffle(keys.begin(), keys.end(), random);

    std::cout << "Unordered-set insert/find/remove benchmark\n"
              << "  Build       : " << test_support::build_configuration() << " x64\n"
              << "  Elements    : " << elements << '\n'
              << "  Repetitions : " << repetitions << " per container\n"
              << "  Workload    : insert all, find all, remove all\n"
              << "  Time unit   : microseconds (us)\n\n";

    print_benchmark_result(
        "MyUnorderedSet", measure_benchmark<MyUnorderedSet<int>>(keys, repetitions));
    print_benchmark_result(
        "std::unordered_set", measure_benchmark<std::unordered_set<int>>(keys, repetitions));
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc > 1 && (std::string(argv[1]) == "benchmark"
        || std::string(argv[1]) == "load-factor")) {
        const bool load_factor = std::string(argv[1]) == "load-factor";
        const int elements = argc > 2 ? std::stoi(argv[2]) : (load_factor ? 8192 : 100000);
        const int repetitions = argc > 3 ? std::stoi(argv[3]) : 15;
        if (elements < 1 || repetitions < 1 || elements > 1000000 || repetitions > 10000) {
            std::cerr << "Usage: MyUnorderedHash.exe [benchmark|load-factor] "
                         "[elements: 1..1000000] [repetitions: 1..10000]\n";
            return 1;
        }
        if (load_factor) run_load_factor_benchmark(elements, repetitions);
        else run_benchmark(elements, repetitions);
        return 0;
    }
    test_support::TestRunner runner(11);
    runner.run("Set uniqueness and bucket collision", test_set_unique_and_collision);
    runner.run("MultiSet duplicate removal", test_multiset_duplicates);
    runner.run("Map access and missing key", test_map_access);
    runner.run("MultiMap duplicate removal", test_multimap_duplicates);
    runner.run("Rehash and reserve", test_rehash_and_reserve);
    runner.run("Iteration across inserted elements", test_iteration);
    runner.run("Copy construction, assignment, and self-assignment", test_copy);
    runner.run("Move construction, assignment, and reuse", test_move_and_reuse);
    runner.run("Empty move construction and assignment", test_empty_move);
    runner.run("MultiSet removal that empties a bucket", test_multiset_bucket_becomes_empty);
    runner.run("50,000 random operations against std::unordered_set",
               test_randomized_against_std_unordered_set);
    return runner.report();
}
