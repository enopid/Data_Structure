#include "MyVector.h"

#include <chrono>
#include <functional>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace {

class TestRunner {
public:
    explicit TestRunner(int total) : total_(total) {}

    void run(const std::string& name, const std::function<void()>& test) {
        ++executed_;
        try {
            test();
            ++passed_;
            std::cout << "[PASS " << executed_ << '/' << total_ << "] " << name << '\n';
        } catch (const std::exception& error) {
            ++failed_;
            std::cout << "[FAIL " << executed_ << '/' << total_ << "] " << name
                      << " - " << error.what() << '\n';
        }
    }

    int report() const {
        std::cout << "\n" << passed_ << " passed, " << failed_ << " failed\n";
        return failed_ == 0 ? 0 : 1;
    }

private:
    int total_ = 0;
    int executed_ = 0;
    int passed_ = 0;
    int failed_ = 0;
};

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

template <typename T>
void require_equal(MyVector<T>& actual, const std::vector<T>& expected) {
    require(actual.size() == static_cast<int>(expected.size()), "size mismatch");
    for (int i = 0; i < actual.size(); ++i) {
        require(actual[i] == expected[static_cast<std::size_t>(i)],
                "element mismatch at index " + std::to_string(i));
    }
}

struct TrackedValue {
    inline static int alive = 0;
    inline static int copies = 0;
    inline static int moves = 0;
    int value = 0;

    explicit TrackedValue(int value = 0) : value(value) { ++alive; }
    TrackedValue(const TrackedValue& other) : value(other.value) {
        ++alive;
        ++copies;
    }
    TrackedValue(TrackedValue&& other) noexcept : value(other.value) {
        other.value = -1;
        ++alive;
        ++moves;
    }
    TrackedValue& operator=(const TrackedValue& other) {
        value = other.value;
        ++copies;
        return *this;
    }
    TrackedValue& operator=(TrackedValue&& other) noexcept {
        value = other.value;
        other.value = -1;
        ++moves;
        return *this;
    }
    ~TrackedValue() { --alive; }
    bool operator==(const TrackedValue& other) const { return value == other.value; }
    friend std::ostream& operator<<(std::ostream& stream, const TrackedValue& tracked) {
        return stream << tracked.value;
    }

    static void reset() {
        alive = 0;
        copies = 0;
        moves = 0;
    }
};

void test_default_state() {
    MyVector<int> values;
    require(values.empty(), "new vector must be empty");
    require(values.size() == 0, "new vector size must be zero");
    require(values.capacity() >= 1, "new vector must have initial storage");
    require(values.data() != nullptr, "data pointer must be valid");
}

void test_push_and_access() {
    MyVector<int> actual;
    std::vector<int> expected;
    for (int i = 0; i < 100; ++i) {
        actual.push_back(i * 3);
        expected.push_back(i * 3);
    }
    require_equal(actual, expected);
    require(actual.front() == expected.front(), "front mismatch");
    require(actual.back() == expected.back(), "back mismatch");
    for (int i = 0; i < 25; ++i) {
        actual.pop_back();
        expected.pop_back();
    }
    require_equal(actual, expected);
}

void test_capacity_operations() {
    MyVector<int> values(4, 7);
    values.reserve(32);
    require(values.capacity() >= 32, "reserve did not increase capacity");
    require(values.size() == 4, "reserve changed size");
    values.resize(10, 9);
    require(values.size() == 10, "resize did not grow size");
    for (int i = 0; i < 4; ++i) require(values[i] == 7, "resize changed old value");
    for (int i = 4; i < 10; ++i) require(values[i] == 9, "resize fill value mismatch");
    values.resize(3);
    values.shrink_to_fit();
    require(values.size() == 3 && values.capacity() == 3, "shrink_to_fit mismatch");
    values.clear();
    require(values.empty(), "clear did not empty vector");
}

void test_insert_and_erase() {
    MyVector<std::string> actual(3, "base");
    std::vector<std::string> expected(3, "base");
    actual.insert(1, "inserted");
    expected.insert(expected.begin() + 1, "inserted");
    actual.erase(2);
    expected.erase(expected.begin() + 2);
    require_equal(actual, expected);
}

void test_copy_and_move() {
    MyVector<std::string> original;
    original.push_back("alpha");
    original.push_back("beta");
    MyVector<std::string> copied(original);
    copied[0] = "changed";
    require(original[0] == "alpha", "copy constructor shared storage");
    MyVector<std::string> assigned;
    assigned = original;
    require(assigned.size() == 2 && assigned[1] == "beta", "copy assignment mismatch");
    MyVector<std::string> moved(std::move(copied));
    require(moved.size() == 2 && moved[0] == "changed", "move constructor mismatch");
    require(copied.size() == 0, "moved-from vector is not empty");
    MyVector<std::string> move_assigned;
    move_assigned = std::move(assigned);
    require(move_assigned.size() == 2, "move assignment mismatch");
    require(assigned.size() == 0, "move-assigned source is not empty");
}

void test_non_trivial_lifetime() {
    TrackedValue::reset();
    {
        MyVector<TrackedValue> values;
        TrackedValue first(1);
        TrackedValue second(2);
        values.push_back(first);
        values.push_back(second);
        values.reserve(16);
        values.erase(0);
        require(values.size() == 1 && values[0].value == 2, "tracked value mismatch");
        require(TrackedValue::moves > 0, "reallocation did not move values");
    }
    require(TrackedValue::alive == 0, "tracked object lifetime leak");
}

void test_against_std_vector() {
    MyVector<int> actual;
    std::vector<int> expected;
    std::mt19937 random(20260827);
    for (int step = 0; step < 1000; ++step) {
        const int operation = static_cast<int>(random() % 4);
        if (operation == 0 || expected.empty()) {
            const int value = static_cast<int>(random() % 10000);
            actual.push_back(value);
            expected.push_back(value);
        } else if (operation == 1) {
            actual.pop_back();
            expected.pop_back();
        } else if (operation == 2) {
            const int position = static_cast<int>(random() % expected.size());
            const int value = static_cast<int>(random() % 10000);
            actual.insert(position, value);
            expected.insert(expected.begin() + position, value);
        } else {
            const int position = static_cast<int>(random() % expected.size());
            actual.erase(position);
            expected.erase(expected.begin() + position);
        }
        require_equal(actual, expected);
    }
}

void test_growth_factor_change() {
    MyVector<int>::set_growth_factor(1.5);
    MyVector<int> default_growth;
    for (int i = 0; i < 5; ++i) default_growth.push_back(i);
    require(default_growth.capacity() == 6,
            "growth factor 1.5 must produce capacity 6 after five pushes");

    MyVector<int>::set_growth_factor(2.0);
    MyVector<int> doubled_growth;
    for (int i = 0; i < 5; ++i) doubled_growth.push_back(i);
    require(doubled_growth.capacity() == 8,
            "growth factor 2.0 must produce capacity 8 after five pushes");

    MyVector<int>::set_growth_factor(1.5);
}

struct BenchmarkResult {
    std::string name;
    long long average_microseconds;
    long long median_microseconds;
    long long minimum_microseconds;
    long long maximum_microseconds;
    int reallocations;
    int final_capacity;
};

template <typename T, typename Push>
BenchmarkResult measure(const std::string& name, const std::vector<T>& input, Push push) {
    MyVector<T> values;
    int reallocations = 0;
    int previous_capacity = values.capacity();
    const auto begin = std::chrono::steady_clock::now();
    for (const auto& value : input) {
        push(values, value);
        if (values.capacity() != previous_capacity) {
            previous_capacity = values.capacity();
            ++reallocations;
        }
    }
    const auto end = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    return {name, elapsed, elapsed, elapsed, elapsed,
            reallocations,
            values.capacity()};
}

template <typename Measurement>
BenchmarkResult measure_repeated(int repetitions, Measurement measurement) {
    BenchmarkResult summary;
    long long total = 0;
    std::vector<long long> elapsed_times;
    elapsed_times.reserve(static_cast<std::size_t>(repetitions));
    summary.minimum_microseconds = std::numeric_limits<long long>::max();
    summary.maximum_microseconds = 0;
    for (int i = 0; i < repetitions; ++i) {
        const auto result = measurement();
        if (i == 0) {
            summary.name = result.name;
            summary.reallocations = result.reallocations;
            summary.final_capacity = result.final_capacity;
        }
        total += result.average_microseconds;
        elapsed_times.push_back(result.average_microseconds);
        summary.minimum_microseconds = std::min(summary.minimum_microseconds, result.average_microseconds);
        summary.maximum_microseconds = std::max(summary.maximum_microseconds, result.average_microseconds);
    }
    summary.average_microseconds = total / repetitions;
    std::sort(elapsed_times.begin(), elapsed_times.end());
    const auto middle = elapsed_times.size() / 2;
    summary.median_microseconds = elapsed_times.size() % 2 == 1
        ? elapsed_times[middle]
        : (elapsed_times[middle - 1] + elapsed_times[middle]) / 2;
    return summary;
}

template <typename T>
BenchmarkResult measure_std_vector(const std::vector<T>& input) {
    std::vector<T> standard;
    int reallocations = 0;
    std::size_t previous_capacity = standard.capacity();
    const auto begin = std::chrono::steady_clock::now();
    for (const auto& value : input) {
        standard.push_back(value);
        if (standard.capacity() != previous_capacity) {
            previous_capacity = standard.capacity();
            ++reallocations;
        }
    }
    const auto end = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    return {"std::vector (MSVC STL)", elapsed, elapsed, elapsed, elapsed,
            reallocations, static_cast<int>(standard.capacity())};
}

template <typename T>
void run_benchmark(const std::vector<T>& input, int repetitions, const std::string& type_name) {
#ifdef NDEBUG
    constexpr const char* build_configuration = "Release";
#else
    constexpr const char* build_configuration = "Debug";
#endif

    MyVector<T>::set_growth_factor(1.5);
    const auto default_growth = measure_repeated(repetitions, [&input] {
        return measure("MyVector Geometric Growth (factor 1.5 default)", input,
            [](MyVector<T>& values, const T& value) { values.push_back(value); });
    });
    MyVector<T>::set_growth_factor(2.0);
    const auto doubled_growth = measure_repeated(repetitions, [&input] {
        return measure("MyVector Geometric Growth (factor 2.0 custom)", input,
            [](MyVector<T>& values, const T& value) { values.push_back(value); });
    });
    const auto linear = measure_repeated(repetitions, [&input] {
        return measure("MyVector Linear Growth (+10 capacity)", input,
            [](MyVector<T>& values, const T& value) { values.linear_push_back(value); });
    });
    const auto std_vector = measure_repeated(repetitions, [&input] {
        return measure_std_vector(input);
    });
    MyVector<T>::set_growth_factor(1.5);

    std::cout << "Benchmark configuration\n"
              << "  Current build     : " << build_configuration << " x64\n"
              << "  Recommended build : Release x64\n"
              << "  Element type      : " << type_name << '\n'
              << "  Elements per case : " << input.size() << '\n'
              << "  Measurements      : " << repetitions << " per case\n"
              << "  Time unit         : microseconds (us)\n\n"
              << "Benchmark results\n";

    int case_number = 0;
    for (const auto& result : {default_growth, doubled_growth, linear, std_vector}) {
        std::cout << "\n[Case " << ++case_number << "] " << result.name << '\n'
                  << "  Elements inserted : " << input.size() << '\n'
                  << "  Average time      : " << result.average_microseconds << " us\n"
                  << "  Median time       : " << result.median_microseconds << " us\n"
                  << "  Minimum time      : " << result.minimum_microseconds << " us\n"
                  << "  Maximum time      : " << result.maximum_microseconds << " us\n"
                  << "  Reallocations     : " << result.reallocations << " times\n"
                  << "  Final capacity    : " << result.final_capacity << '\n';
    }
}

std::vector<int> make_int_input(int count) {
    std::vector<int> input;
    input.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) input.push_back(i);
    return input;
}

std::vector<std::string> make_string_input(int count) {
    constexpr char characters[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    constexpr int string_length = 24;
    std::mt19937 random(20260827);
    std::uniform_int_distribution<std::size_t> pick(0, sizeof(characters) - 2);
    std::vector<std::string> input;
    input.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        std::string value;
        value.reserve(string_length);
        for (int j = 0; j < string_length; ++j) value.push_back(characters[pick(random)]);
        input.push_back(std::move(value));
    }
    return input;
}

int run_tests() {
    TestRunner runner(8);
    std::cout << "MyVector validity tests\n\n";
    runner.run("Default state (size, capacity, empty, data)", test_default_state);
    runner.run("Push, pop and element access", test_push_and_access);
    runner.run("Capacity operations (reserve, resize, shrink, clear)", test_capacity_operations);
    runner.run("Insert and erase with std::string", test_insert_and_erase);
    runner.run("Copy and move semantics", test_copy_and_move);
    runner.run("Non-trivial object lifetime and destruction", test_non_trivial_lifetime);
    runner.run("1,000 random operations against std::vector", test_against_std_vector);
    runner.run("Growth factor change (1.5 versus 2.0)", test_growth_factor_change);
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
            run_benchmark(make_int_input(count), repetitions, "int");
        } else if (type == "string") {
            run_benchmark(make_string_input(count), repetitions, "std::string (24 random characters)");
        } else {
            std::cerr << "Element type must be 'int' or 'string'.\n";
            return 1;
        }
        return 0;
    }
    return run_tests();
}
