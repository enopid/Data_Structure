#include "MyOrderedSet.h"
#include "MyOrderedMap.h"
#include "MyOrderedMultiSet.h"
#include "MyOrderedMultiMap.h"
#include "MyUnorderedSet.h"
#include "../../Common/TestUtils.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {

using test_support::require;

template<typename Tree>
std::vector<int> collect_keys(Tree& tree) {
    std::vector<int> result;
    for (auto it = tree.begin(); it != tree.end(); ++it) result.push_back(*it);
    return result;
}

void test_unique_insertion_and_find() {
    MyOrderedSet<int> values;
    for (int value : {7, 3, 9, 1, 5, 8, 10, 3}) values.Insert(value);
    require(values.Size() == 7, "set accepted a duplicate key");
    require(collect_keys(values) == std::vector<int>({1, 3, 5, 7, 8, 9, 10}),
            "in-order iteration is not sorted");
    require(*values.Find(5) == 5, "Find failed for an existing key");
    require(values.Find(99) == values.end(), "Find returned a node for a missing key");
}

void test_bidirectional_iteration() {
    MyOrderedSet<int> values;
    for (int value : {4, 2, 6, 1, 3, 5, 7}) values.Insert(value);
    auto it = values.end();
    std::vector<int> reversed;
    for (int count = 0; count < values.Size(); ++count) {
        --it;
        reversed.push_back(*it);
    }
    require(reversed == std::vector<int>({7, 6, 5, 4, 3, 2, 1}),
            "reverse iteration order is wrong");

    auto post = values.begin();
    auto old = post++;
    require(*old == 1 && *post == 2, "post-increment semantics are wrong");
}

void test_removal_cases() {
    MyOrderedSet<int> values;
    std::set<int> expected;
    for (int value : {20, 10, 30, 5, 15, 25, 40, 1, 7, 12, 17, 22, 27, 35, 50}) {
        values.Insert(value);
        expected.insert(value);
    }
    for (int value : {1, 5, 30, 20, 50, 999}) {
        values.Remove(value);
        expected.erase(value);
        require(values.Size() == static_cast<int>(expected.size()), "size changed incorrectly after removal");
        require(collect_keys(values) == std::vector<int>(expected.begin(), expected.end()),
                "removal damaged tree ordering or connectivity");
    }
}

void test_randomized_against_std_set() {
    MyOrderedSet<int> actual;
    std::set<int> expected;
    std::mt19937 random(20260924);
    std::uniform_int_distribution<int> value_distribution(-10000, 10000);
    for (int operation = 0; operation < 30000; ++operation) {
        const int value = value_distribution(random);
        const int action = static_cast<int>(random() % 3);
        if (action == 0) {
            actual.Insert(value);
            expected.insert(value);
        } else if (action == 1) {
            actual.Remove(value);
            expected.erase(value);
        } else {
            require((actual.Find(value) != actual.end()) == (expected.find(value) != expected.end()),
                    "randomized Find mismatch");
        }
        require(actual.Size() == static_cast<int>(expected.size()), "randomized size mismatch");
        if (operation % 100 == 0) {
            require(collect_keys(actual) == std::vector<int>(expected.begin(), expected.end()),
                    "randomized contents mismatch");
        }
    }
    require(collect_keys(actual) == std::vector<int>(expected.begin(), expected.end()),
            "final randomized contents mismatch");
}

void test_multi_containers() {
    MyOrderedMultiSet<int> values;
    for (int value : {2, 1, 2, 3, 2}) values.Insert(value);
    require(values.Size() == 5, "multiset rejected duplicate keys");
    require(collect_keys(values) == std::vector<int>({1, 2, 2, 2, 3}), "multiset iteration order is wrong");
    values.Remove(2);
    require(values.Size() == 4, "multiset Remove did not remove exactly one element");

    MyOrderedMultiMap<int, std::string> map;
    map.Insert({2, "first"});
    map.Insert({2, "second"});
    map.Insert({1, "one"});
    require(map.Size() == 3, "multimap rejected duplicate keys");
}

void test_map_iteration() {
    MyOrderedMap<int, std::string> values;
    values.Insert({3, "three"});
    values.Insert({1, "one"});
    values.Insert({2, "two"});
    values.Insert({2, "duplicate"});
    std::vector<int> keys;
    for (auto it = values.begin(); it != values.end(); ++it) keys.push_back(it->first);
    require(keys == std::vector<int>({1, 2, 3}), "map iteration order is wrong");
    require(values.Size() == 3, "map accepted a duplicate key");
    require(values.Find(2)->second == "two", "map duplicate replaced the original value");
}

void test_copy_and_move() {
    MyOrderedSet<int> source;
    for (int value : {5, 1, 9, 3, 7}) source.Insert(value);
    MyOrderedSet<int> copy(source);
    source.Remove(5);
    require(copy.Size() == 5 && collect_keys(copy) == std::vector<int>({1, 3, 5, 7, 9}),
            "copy construction did not create an independent tree");

    MyOrderedSet<int> assigned;
    assigned = copy;
    assigned = assigned;
    require(collect_keys(assigned) == collect_keys(copy), "copy assignment failed");

    MyOrderedSet<int> moved(std::move(source));
    require(source.Size() == 0 && moved.Size() == 4, "move construction size is wrong");
    source.Insert(11);
    require(*source.Find(11) == 11, "move-constructed source cannot be reused");

    assigned = std::move(moved);
    require(moved.Size() == 0 && assigned.Size() == 4, "move assignment failed");
    moved.Insert(13);
    require(*moved.Find(13) == 13, "move-assigned source cannot be reused");
}

void test_clear_and_empty_iteration() {
    MyOrderedSet<int> values;
    require(values.begin() == values.end(), "empty begin and end differ");
    values.Insert(1);
    values.Insert(2);
    values.Clear();
    require(values.Size() == 0 && values.begin() == values.end(), "Clear did not empty the tree");
    values.Insert(3);
    require(values.Size() == 1 && *values.begin() == 3, "tree cannot be reused after Clear");
}

struct PhaseTimes {
    long long insert_us = 0;
    long long find_us = 0;
    long long remove_us = 0;
    long long reinsert_us = 0;
    long long iterate_us = 0;

    long long total_us() const {
        return insert_us + find_us + remove_us + reinsert_us + iterate_us;
    }
};

struct BenchmarkSummary {
    PhaseTimes average;
    PhaseTimes median;
};

long long elapsed_us(std::chrono::steady_clock::time_point start,
                     std::chrono::steady_clock::time_point end) {
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

PhaseTimes divide_times(const PhaseTimes& value, long long divisor) {
    return {
        value.insert_us / divisor,
        value.find_us / divisor,
        value.remove_us / divisor,
        value.reinsert_us / divisor,
        value.iterate_us / divisor
    };
}

PhaseTimes median_times(std::vector<PhaseTimes> values) {
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
        median_of(&PhaseTimes::remove_us),
        median_of(&PhaseTimes::reinsert_us),
        median_of(&PhaseTimes::iterate_us)
    };
}

template<typename RunOnce>
BenchmarkSummary measure_benchmark(int repetitions, RunOnce run_once) {
    std::vector<PhaseTimes> samples;
    samples.reserve(static_cast<std::size_t>(repetitions));
    PhaseTimes total;
    for (int repeat = 0; repeat < repetitions; ++repeat) {
        const PhaseTimes current = run_once();
        samples.push_back(current);
        total.insert_us += current.insert_us;
        total.find_us += current.find_us;
        total.remove_us += current.remove_us;
        total.reinsert_us += current.reinsert_us;
        total.iterate_us += current.iterate_us;
    }
    return {divide_times(total, repetitions), median_times(std::move(samples))};
}

PhaseTimes run_my_ordered_set(const std::vector<int>& keys) {
    MyOrderedSet<int> values;
    auto start = std::chrono::steady_clock::now();
    for (int key : keys) values.Insert(key);
    auto end = std::chrono::steady_clock::now();
    PhaseTimes times;
    times.insert_us = elapsed_us(start, end);

    start = std::chrono::steady_clock::now();
    int found = 0;
    for (int key : keys) found += values.Find(key) != values.end();
    end = std::chrono::steady_clock::now();
    times.find_us = elapsed_us(start, end);
    require(found == static_cast<int>(keys.size()), "MyOrderedSet benchmark lookup failed");

    start = std::chrono::steady_clock::now();
    for (std::size_t index = 0; index < keys.size(); index += 2) values.Remove(keys[index]);
    end = std::chrono::steady_clock::now();
    times.remove_us = elapsed_us(start, end);

    start = std::chrono::steady_clock::now();
    for (std::size_t index = 0; index < keys.size(); index += 2) values.Insert(keys[index]);
    end = std::chrono::steady_clock::now();
    times.reinsert_us = elapsed_us(start, end);

    start = std::chrono::steady_clock::now();
    long long checksum = 0;
    for (auto it = values.begin(); it != values.end(); ++it) checksum += *it;
    end = std::chrono::steady_clock::now();
    times.iterate_us = elapsed_us(start, end);
    const long long expected = static_cast<long long>(keys.size() - 1) * keys.size() / 2;
    require(values.Size() == static_cast<int>(keys.size()) && checksum == expected,
            "MyOrderedSet benchmark final state is invalid");
    return times;
}

PhaseTimes run_std_set(const std::vector<int>& keys) {
    std::set<int> values;
    auto start = std::chrono::steady_clock::now();
    for (int key : keys) values.insert(key);
    auto end = std::chrono::steady_clock::now();
    PhaseTimes times;
    times.insert_us = elapsed_us(start, end);

    start = std::chrono::steady_clock::now();
    int found = 0;
    for (int key : keys) found += values.find(key) != values.end();
    end = std::chrono::steady_clock::now();
    times.find_us = elapsed_us(start, end);
    require(found == static_cast<int>(keys.size()), "std::set benchmark lookup failed");

    start = std::chrono::steady_clock::now();
    for (std::size_t index = 0; index < keys.size(); index += 2) values.erase(keys[index]);
    end = std::chrono::steady_clock::now();
    times.remove_us = elapsed_us(start, end);

    start = std::chrono::steady_clock::now();
    for (std::size_t index = 0; index < keys.size(); index += 2) values.insert(keys[index]);
    end = std::chrono::steady_clock::now();
    times.reinsert_us = elapsed_us(start, end);

    start = std::chrono::steady_clock::now();
    long long checksum = std::accumulate(values.begin(), values.end(), 0LL);
    end = std::chrono::steady_clock::now();
    times.iterate_us = elapsed_us(start, end);
    const long long expected = static_cast<long long>(keys.size() - 1) * keys.size() / 2;
    require(values.size() == keys.size() && checksum == expected, "std::set benchmark final state is invalid");
    return times;
}

void print_benchmark_result(const char* name, const BenchmarkSummary& result) {
    std::cout << name << '\n'
              << "  Average (us) : insert=" << result.average.insert_us
              << ", find=" << result.average.find_us
              << ", remove=" << result.average.remove_us
              << ", reinsert=" << result.average.reinsert_us
              << ", iterate=" << result.average.iterate_us
              << ", total=" << result.average.total_us() << '\n'
              << "  Median  (us) : insert=" << result.median.insert_us
              << ", find=" << result.median.find_us
              << ", remove=" << result.median.remove_us
              << ", reinsert=" << result.median.reinsert_us
              << ", iterate=" << result.median.iterate_us
              << ", total=" << result.median.total_us() << "\n\n";
}

void run_benchmark(int elements, int repetitions) {
    std::vector<int> keys(static_cast<std::size_t>(elements));
    std::iota(keys.begin(), keys.end(), 0);
    std::mt19937 random(20260924);
    std::shuffle(keys.begin(), keys.end(), random);

    std::cout << "Ordered-set mixed workload benchmark\n"
              << "  Build       : " << test_support::build_configuration() << " x64\n"
              << "  Elements    : " << elements << '\n'
              << "  Repetitions : " << repetitions << " per container\n"
              << "  Workload    : insert all, find all, remove half, reinsert half, iterate all\n"
              << "  Time unit   : microseconds (us)\n\n";

    const auto custom = measure_benchmark(repetitions, [&keys] { return run_my_ordered_set(keys); });
    const auto standard = measure_benchmark(repetitions, [&keys] { return run_std_set(keys); });
    print_benchmark_result("MyOrderedSet", custom);
    print_benchmark_result("std::set", standard);
}

struct ComparisonTimes {
    long long insert_us = 0;
    long long find_us = 0;
    long long remove_us = 0;

    long long total_us() const { return insert_us + find_us + remove_us; }
};

struct ComparisonSummary {
    ComparisonTimes average;
    ComparisonTimes median;
};

void insert_value(MyOrderedSet<int>& values, int key) { values.Insert(key); }
void insert_value(MyUnorderedSet<int>& values, int key) { values.Insert(key); }
void insert_value(std::set<int>& values, int key) { values.insert(key); }
void insert_value(std::unordered_set<int>& values, int key) { values.insert(key); }

bool contains_value(MyOrderedSet<int>& values, int key) { return values.Find(key) != values.end(); }
bool contains_value(MyUnorderedSet<int>& values, int key) { return values.Find(key) != nullptr; }
bool contains_value(std::set<int>& values, int key) { return values.find(key) != values.end(); }
bool contains_value(std::unordered_set<int>& values, int key) {
    return values.find(key) != values.end();
}

void remove_value(MyOrderedSet<int>& values, int key) { values.Remove(key); }
void remove_value(MyUnorderedSet<int>& values, int key) { values.Remove(key); }
void remove_value(std::set<int>& values, int key) { values.erase(key); }
void remove_value(std::unordered_set<int>& values, int key) { values.erase(key); }

int container_size(MyOrderedSet<int>& values) { return values.Size(); }
int container_size(MyUnorderedSet<int>& values) { return values.Size(); }
int container_size(std::set<int>& values) { return static_cast<int>(values.size()); }
int container_size(std::unordered_set<int>& values) { return static_cast<int>(values.size()); }

template<typename Set>
ComparisonTimes run_comparison_once(const std::vector<int>& keys) {
    Set values;
    ComparisonTimes times;

    auto start = std::chrono::steady_clock::now();
    for (int key : keys) insert_value(values, key);
    auto end = std::chrono::steady_clock::now();
    times.insert_us = elapsed_us(start, end);
    require(container_size(values) == static_cast<int>(keys.size()),
            "comparison benchmark insertion failed");

    start = std::chrono::steady_clock::now();
    int found = 0;
    for (int key : keys) found += contains_value(values, key);
    end = std::chrono::steady_clock::now();
    times.find_us = elapsed_us(start, end);
    require(found == static_cast<int>(keys.size()), "comparison benchmark lookup failed");

    start = std::chrono::steady_clock::now();
    for (int key : keys) remove_value(values, key);
    end = std::chrono::steady_clock::now();
    times.remove_us = elapsed_us(start, end);
    require(container_size(values) == 0, "comparison benchmark removal failed");
    return times;
}

ComparisonTimes median_comparison_times(const std::vector<ComparisonTimes>& values) {
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
        median_of(&ComparisonTimes::insert_us),
        median_of(&ComparisonTimes::find_us),
        median_of(&ComparisonTimes::remove_us)
    };
}

template<typename Set>
ComparisonSummary measure_comparison(const std::vector<int>& keys, int repetitions) {
    std::vector<ComparisonTimes> samples;
    samples.reserve(static_cast<std::size_t>(repetitions));
    ComparisonTimes total;
    for (int repeat = 0; repeat < repetitions; ++repeat) {
        const auto current = run_comparison_once<Set>(keys);
        samples.push_back(current);
        total.insert_us += current.insert_us;
        total.find_us += current.find_us;
        total.remove_us += current.remove_us;
    }
    return {
        {total.insert_us / repetitions, total.find_us / repetitions,
         total.remove_us / repetitions},
        median_comparison_times(samples)
    };
}

void print_comparison_result(const char* name, const ComparisonSummary& result) {
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

void run_ordered_unordered_comparison(int elements, int repetitions) {
    std::vector<int> keys(static_cast<std::size_t>(elements));
    std::iota(keys.begin(), keys.end(), 0);
    std::mt19937 random(20260925);
    std::shuffle(keys.begin(), keys.end(), random);

    std::cout << "Ordered versus unordered set benchmark\n"
              << "  Build       : " << test_support::build_configuration() << " x64\n"
              << "  Elements    : " << elements << '\n'
              << "  Repetitions : " << repetitions << " per container\n"
              << "  Workload    : insert all, find all, remove all\n"
              << "  Time unit   : microseconds (us)\n\n";

    const auto my_ordered = measure_comparison<MyOrderedSet<int>>(keys, repetitions);
    const auto my_unordered = measure_comparison<MyUnorderedSet<int>>(keys, repetitions);
    const auto std_ordered = measure_comparison<std::set<int>>(keys, repetitions);
    const auto std_unordered = measure_comparison<std::unordered_set<int>>(keys, repetitions);

    print_comparison_result("MyOrderedSet", my_ordered);
    print_comparison_result("MyUnorderedSet", my_unordered);
    print_comparison_result("std::set", std_ordered);
    print_comparison_result("std::unordered_set", std_unordered);
}

void run_ordered_benchmark(int elements, int repetitions) {
    std::vector<int> keys(static_cast<std::size_t>(elements));
    std::iota(keys.begin(), keys.end(), 0);
    std::mt19937 random(20260925);
    std::shuffle(keys.begin(), keys.end(), random);

    std::cout << "Ordered-set insert/find/remove benchmark\n"
              << "  Build       : " << test_support::build_configuration() << " x64\n"
              << "  Elements    : " << elements << '\n'
              << "  Repetitions : " << repetitions << " per container\n"
              << "  Workload    : insert all, find all, remove all\n"
              << "  Time unit   : microseconds (us)\n\n";

    print_comparison_result(
        "MyOrderedSet", measure_comparison<MyOrderedSet<int>>(keys, repetitions));
    print_comparison_result(
        "std::set", measure_comparison<std::set<int>>(keys, repetitions));
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc > 1 && (std::string(argv[1]) == "benchmark"
        || std::string(argv[1]) == "compare"
        || std::string(argv[1]) == "mixed")) {
        const std::string mode = argv[1];
        const int elements = argc > 2 ? std::stoi(argv[2]) : 100000;
        const int repetitions = argc > 3 ? std::stoi(argv[3]) : 15;
        if (elements < 1 || elements > 1000000 || repetitions < 1 || repetitions > 10000) {
            std::cerr << "Usage: MyOrderedTree.exe [benchmark|compare|mixed] "
                         "[elements: 1..1000000] [repetitions: 1..10000]\n";
            return 1;
        }
        if (mode == "compare") run_ordered_unordered_comparison(elements, repetitions);
        else if (mode == "mixed") run_benchmark(elements, repetitions);
        else run_ordered_benchmark(elements, repetitions);
        return 0;
    }

    test_support::TestRunner runner(8);
    runner.run("Unique insertion, sorted iteration, and Find", test_unique_insertion_and_find);
    runner.run("Bidirectional iterator movement", test_bidirectional_iteration);
    runner.run("Leaf, one-child, two-child, root, and missing-key removal", test_removal_cases);
    runner.run("30,000 random operations against std::set", test_randomized_against_std_set);
    runner.run("MultiSet and MultiMap duplicate handling", test_multi_containers);
    runner.run("Map insertion and ordered iteration", test_map_iteration);
    runner.run("Copy, move, self-assignment, and moved-from reuse", test_copy_and_move);
    runner.run("Clear and empty iteration", test_clear_and_empty_iteration);
    return runner.report();
}
