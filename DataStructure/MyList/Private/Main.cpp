#include "MyList.h"
#include "TestUtils.h"

#include <algorithm>
#include <chrono>
#include <list>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

using test_support::require;
using test_support::TestRunner;

template <typename T>
void require_equal(const MyLinkedList<T>& actual, const std::list<T>& expected) {
    require(actual.size() == static_cast<int>(expected.size()), "size mismatch");
    auto actual_it = actual.cbegin();
    auto expected_it = expected.cbegin();
    int index = 0;
    while (expected_it != expected.cend()) {
        require(*actual_it == *expected_it,
                "element mismatch at index " + std::to_string(index));
        ++actual_it;
        ++expected_it;
        ++index;
    }
    require(!(actual_it != actual.cend()), "custom list contains extra elements");
}

template <typename T>
typename MyLinkedList<T>::template iterator<false> iterator_at(
    MyLinkedList<T>& values, int position) {
    auto it = values.begin();
    for (int index = 0; index < position; ++index) ++it;
    return it;
}

struct TrackedValue {
    inline static int alive = 0;
    inline static int copies = 0;
    int value = 0;

    explicit TrackedValue(int value = 0) : value(value) { ++alive; }
    TrackedValue(const TrackedValue& other) : value(other.value) {
        ++alive;
        ++copies;
    }
    TrackedValue& operator=(const TrackedValue& other) {
        value = other.value;
        ++copies;
        return *this;
    }
    ~TrackedValue() { --alive; }
    bool operator==(const TrackedValue& other) const { return value == other.value; }

    static void reset() {
        alive = 0;
        copies = 0;
    }
};

void test_default_and_fill_constructors() {
    MyLinkedList<int> empty;
    require(empty.empty(), "new list must be empty");
    require(empty.size() == 0, "new list size must be zero");
    require(!(empty.begin() != empty.end()), "empty begin/end mismatch");

    MyLinkedList<int> default_values(3);
    require_equal(default_values, std::list<int>(3));

    MyLinkedList<int> filled(4, 7);
    require_equal(filled, std::list<int>(4, 7));
}

void test_push_pop_and_access() {
    MyLinkedList<int> actual;
    std::list<int> expected;
    for (int value = 0; value < 50; ++value) {
        if (value % 2 == 0) {
            actual.push_front(value);
            expected.push_front(value);
        } else {
            actual.push_back(value);
            expected.push_back(value);
        }
    }
    require_equal(actual, expected);
    require(actual.front() == expected.front(), "front mismatch");
    require(actual.back() == expected.back(), "back mismatch");

    for (int count = 0; count < 10; ++count) {
        actual.pop_front();
        expected.pop_front();
        actual.pop_back();
        expected.pop_back();
    }
    require_equal(actual, expected);
    actual.clear();
    require(actual.empty(), "clear did not empty list");
}

void test_insert_and_erase() {
    MyLinkedList<std::string> actual;
    std::list<std::string> expected;
    for (const auto& value : {"alpha", "beta", "gamma"}) {
        actual.push_back(value);
        expected.push_back(value);
    }

    actual.insert(iterator_at(actual, 1), "inserted");
    expected.insert(std::next(expected.begin(), 1), "inserted");
    actual.insert(actual.end(), "tail");
    expected.insert(expected.end(), "tail");
    require_equal(actual, expected);

    actual.erase(iterator_at(actual, 2));
    expected.erase(std::next(expected.begin(), 2));
    actual.erase(actual.end());
    require_equal(actual, expected);
}

void test_forward_reverse_and_const_iteration() {
    MyLinkedList<int> values;
    for (int value = 1; value <= 5; ++value) values.push_back(value);

    std::vector<int> forward;
    for (auto it = values.begin(); it != values.end(); ++it) forward.push_back(*it);
    require(forward == std::vector<int>({1, 2, 3, 4, 5}), "forward iteration mismatch");

    std::vector<int> reverse;
    for (auto it = values.rbegin(); it != values.rend(); ++it) reverse.push_back(*it);
    require(reverse == std::vector<int>({5, 4, 3, 2, 1}), "reverse iteration mismatch");

    const MyLinkedList<int>& const_values = values;
    int sum = 0;
    for (auto it = const_values.cbegin(); it != const_values.cend(); ++it) sum += *it;
    require(sum == 15, "const iteration mismatch");
    require(const_values.front() == 1 && const_values.back() == 5,
            "const front/back mismatch");

    std::ostringstream output;
    output << values;
    require(output.str() == "1 2 3 4 5 ", "stream output mismatch");
}

void test_copy_semantics() {
    MyLinkedList<std::string> original;
    original.push_back("alpha");
    original.push_back("beta");

    MyLinkedList<std::string> copied(original);
    copied.front() = "changed";
    require(original.front() == "alpha", "copy constructor shared nodes");

    MyLinkedList<std::string> assigned;
    assigned.push_back("old");
    assigned = original;
    require_equal(assigned, std::list<std::string>({"alpha", "beta"}));

    assigned = assigned;
    require_equal(assigned, std::list<std::string>({"alpha", "beta"}));
}

void test_move_semantics() {
    MyLinkedList<std::string> source;
    source.push_back("alpha");
    source.push_back("beta");
    MyLinkedList<std::string> moved(std::move(source));
    require_equal(moved, std::list<std::string>({"alpha", "beta"}));
    require(source.empty(), "move-constructed source is not empty");

    MyLinkedList<std::string> assigned;
    assigned.push_back("old");
    assigned = std::move(moved);
    require_equal(assigned, std::list<std::string>({"alpha", "beta"}));
    require(moved.empty(), "move-assigned source is not empty");

    MyLinkedList<std::string> empty_source;
    assigned = std::move(empty_source);
    require(assigned.empty(), "moving an empty list did not produce an empty list");
}

void test_non_trivial_lifetime() {
    TrackedValue::reset();
    {
        MyLinkedList<TrackedValue> values;
        TrackedValue first(1);
        TrackedValue second(2);
        values.push_back(first);
        values.push_back(second);
        values.erase(values.begin());
        require(values.size() == 1 && values.front().value == 2,
                "tracked value mismatch");
        require(TrackedValue::copies >= 2, "values were not copied into nodes");
    }
    require(TrackedValue::alive == 0, "tracked object lifetime leak");
}

void test_against_std_list() {
    MyLinkedList<int> actual;
    std::list<int> expected;
    std::mt19937 random(20260902);

    for (int step = 0; step < 1000; ++step) {
        const int operation = static_cast<int>(random() % 6);
        if (operation == 0 || expected.empty()) {
            const int value = static_cast<int>(random() % 10000);
            actual.push_back(value);
            expected.push_back(value);
        } else if (operation == 1) {
            const int value = static_cast<int>(random() % 10000);
            actual.push_front(value);
            expected.push_front(value);
        } else if (operation == 2) {
            actual.pop_back();
            expected.pop_back();
        } else if (operation == 3) {
            actual.pop_front();
            expected.pop_front();
        } else if (operation == 4) {
            const int position = static_cast<int>(random() % (expected.size() + 1));
            const int value = static_cast<int>(random() % 10000);
            actual.insert(iterator_at(actual, position), value);
            expected.insert(std::next(expected.begin(), position), value);
        } else {
            const int position = static_cast<int>(random() % expected.size());
            actual.erase(iterator_at(actual, position));
            expected.erase(std::next(expected.begin(), position));
        }
        require_equal(actual, expected);
    }
}

struct BenchmarkResult {
    std::string name;
    long long average_microseconds = 0;
    long long median_microseconds = 0;
    long long minimum_microseconds = 0;
    long long maximum_microseconds = 0;
};

template <typename Measurement>
BenchmarkResult measure_repeated(
    const std::string& name, int repetitions, Measurement measurement) {
    std::vector<long long> elapsed_times;
    elapsed_times.reserve(static_cast<std::size_t>(repetitions));
    long long total = 0;
    long long minimum = std::numeric_limits<long long>::max();
    long long maximum = 0;

    for (int repeat = 0; repeat < repetitions; ++repeat) {
        const auto begin = std::chrono::steady_clock::now();
        measurement();
        const auto end = std::chrono::steady_clock::now();
        const auto elapsed =
            std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
        elapsed_times.push_back(elapsed);
        total += elapsed;
        minimum = std::min(minimum, elapsed);
        maximum = std::max(maximum, elapsed);
    }

    std::sort(elapsed_times.begin(), elapsed_times.end());
    const std::size_t middle = elapsed_times.size() / 2;
    const long long median = elapsed_times.size() % 2 == 1
        ? elapsed_times[middle]
        : (elapsed_times[middle - 1] + elapsed_times[middle]) / 2;
    return {name, total / repetitions, median, minimum, maximum};
}

template <typename T>
void run_benchmark(const std::vector<T>& input, int repetitions, const std::string& type_name) {
    const auto custom = measure_repeated("MyLinkedList", repetitions, [&input] {
        MyLinkedList<T> values;
        for (const auto& value : input) values.push_back(value);
    });
    const auto standard = measure_repeated("std::list (MSVC STL)", repetitions, [&input] {
        std::list<T> values;
        for (const auto& value : input) values.push_back(value);
    });

    std::cout << "Benchmark configuration\n"
              << "  Current build     : " << test_support::build_configuration() << " x64\n"
              << "  Recommended build : Release x64\n"
              << "  Operation         : sequential push_back\n"
              << "  Element type      : " << type_name << '\n'
              << "  Elements per case : " << input.size() << '\n'
              << "  Measurements      : " << repetitions << " per case\n"
              << "  Time unit         : microseconds (us)\n\n"
              << "Benchmark results\n";

    for (const auto& result : {custom, standard}) {
        std::cout << "\n" << result.name << '\n'
                  << "  Average time : " << result.average_microseconds << " us\n"
                  << "  Median time  : " << result.median_microseconds << " us\n"
                  << "  Minimum time : " << result.minimum_microseconds << " us\n"
                  << "  Maximum time : " << result.maximum_microseconds << " us\n";
    }
}

int run_tests() {
    TestRunner runner(8);
    std::cout << "MyLinkedList validity tests\n\n";
    runner.run("Default and fill constructors", test_default_and_fill_constructors);
    runner.run("Push, pop, clear and element access", test_push_pop_and_access);
    runner.run("Insert and erase with std::string", test_insert_and_erase);
    runner.run("Forward, reverse and const iteration", test_forward_reverse_and_const_iteration);
    runner.run("Copy construction, assignment and self-assignment", test_copy_semantics);
    runner.run("Move construction, assignment and empty source", test_move_semantics);
    runner.run("Non-trivial object lifetime and destruction", test_non_trivial_lifetime);
    runner.run("1,000 random operations against std::list", test_against_std_list);
    return runner.report();
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--benchmark") {
        const int count = argc > 2 ? std::stoi(argv[2]) : 100000;
        const int repetitions = argc > 3 ? std::stoi(argv[3]) : 10;
        const std::string type = argc > 4 ? argv[4] : "int";
        if (count <= 0 || repetitions <= 0) {
            std::cerr << "Element count and repetitions must be positive integers.\n";
            return 1;
        }
        if (type == "int") {
            run_benchmark(test_support::make_int_input(count), repetitions, "int");
        } else if (type == "string") {
            run_benchmark(test_support::make_string_input(count, 20260902), repetitions,
                          "std::string (24 random characters)");
        } else {
            std::cerr << "Element type must be 'int' or 'string'.\n";
            return 1;
        }
        return 0;
    }
    return run_tests();
}
