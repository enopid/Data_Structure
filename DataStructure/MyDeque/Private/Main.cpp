#include "MyDeque.h"
#include "TestUtils.h"

#include <algorithm>
#include <chrono>
#include <deque>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

using test_support::require;
using test_support::TestRunner;

template <typename T, int ChunkSize>
void require_equal(const MyDeque<T, ChunkSize>& actual,
                   const std::deque<T>& expected) {
    require(actual.size() == static_cast<int>(expected.size()), "size mismatch");
    for (int i = 0; i < actual.size(); ++i) {
        require(actual[i] == expected[static_cast<std::size_t>(i)],
                "element mismatch at index " + std::to_string(i));
    }
}

void test_default_state_and_exceptions() {
    MyDeque<int> values;
    require(values.empty(), "new deque must be empty");
    require(values.size() == 0, "new deque size must be zero");

    bool front_threw = false;
    bool back_threw = false;
    bool index_threw = false;
    try { (void)values.front(); } catch (const std::out_of_range&) { front_threw = true; }
    try { (void)values.back(); } catch (const std::out_of_range&) { back_threw = true; }
    try { (void)values[0]; } catch (const std::out_of_range&) { index_threw = true; }
    require(front_threw && back_threw && index_threw,
            "empty access must throw std::out_of_range");
}

void test_push_pop_and_chunk_boundaries() {
    MyDeque<int, 16> actual;
    std::deque<int> expected;
    for (int value = 0; value < 40; ++value) {
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

    for (int i = 0; i < 10; ++i) {
        actual.pop_front();
        expected.pop_front();
        actual.pop_back();
        expected.pop_back();
    }
    require_equal(actual, expected);
}

void test_wrapping_and_growth() {
    MyDeque<int, 12> actual;
    std::deque<int> expected;
    for (int value = 0; value < 18; ++value) {
        actual.push_back(value);
        expected.push_back(value);
    }
    for (int i = 0; i < 11; ++i) {
        actual.pop_front();
        expected.pop_front();
    }
    for (int value = 18; value < 55; ++value) {
        actual.push_back(value);
        expected.push_back(value);
    }
    for (int value = -1; value >= -20; --value) {
        actual.push_front(value);
        expected.push_front(value);
    }
    require_equal(actual, expected);
}

void test_clear_and_reuse() {
    MyDeque<std::string, 64> values;
    for (int i = 0; i < 20; ++i) values.push_back("old-" + std::to_string(i));
    values.clear();
    require(values.empty() && values.size() == 0, "clear did not empty deque");

    values.push_front("middle");
    values.push_front("first");
    values.push_back("last");
    require(values.size() == 3, "deque cannot be reused after clear");
    require(values[0] == "first" && values[1] == "middle" && values[2] == "last",
            "reused deque element order mismatch");
}

void test_copy_semantics() {
    MyDeque<std::string, 64> original;
    std::deque<std::string> expected;
    for (int i = 0; i < 15; ++i) {
        original.push_back("value-" + std::to_string(i));
        expected.push_back("value-" + std::to_string(i));
    }
    for (int i = 0; i < 5; ++i) {
        original.pop_front();
        expected.pop_front();
    }
    original.push_back("tail");
    expected.push_back("tail");

    MyDeque<std::string, 64> copied(original);
    require_equal(copied, expected);
    copied[0] = "changed";
    require(original[0] == expected[0], "copy constructor shared element storage");

    MyDeque<std::string, 64> assigned;
    assigned.push_back("discarded");
    assigned = original;
    require_equal(assigned, expected);
    assigned = assigned;
    require_equal(assigned, expected);
}

void test_move_semantics() {
    MyDeque<std::string, 64> source;
    source.push_front("first");
    source.push_back("second");

    MyDeque<std::string, 64> moved(std::move(source));
    require(source.empty(), "move-constructed source is not empty");
    require(moved.size() == 2 && moved[0] == "first" && moved[1] == "second",
            "move constructor mismatch");

    MyDeque<std::string, 64> assigned;
    assigned.push_back("discarded");
    assigned = std::move(moved);
    require(moved.empty(), "move-assigned source is not empty");
    require(assigned.size() == 2 && assigned.front() == "first" && assigned.back() == "second",
            "move assignment mismatch");
}

void test_against_std_deque() {
    MyDeque<int, 20> actual;
    std::deque<int> expected;
    std::mt19937 random(20260905);

    for (int step = 0; step < 5000; ++step) {
        const int operation = static_cast<int>(random() % 4);
        if (operation == 0 || expected.empty()) {
            const int value = static_cast<int>(random() % 100000);
            actual.push_front(value);
            expected.push_front(value);
        } else if (operation == 1) {
            const int value = static_cast<int>(random() % 100000);
            actual.push_back(value);
            expected.push_back(value);
        } else if (operation == 2) {
            actual.pop_front();
            expected.pop_front();
        } else {
            actual.pop_back();
            expected.pop_back();
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
    long long checksum = 0;
};

template <typename Work>
BenchmarkResult measure_repeated(const std::string& name, int repetitions, Work work) {
    BenchmarkResult result;
    result.name = name;
    result.minimum_microseconds = std::numeric_limits<long long>::max();
    std::vector<long long> elapsed_times;
    elapsed_times.reserve(static_cast<std::size_t>(repetitions));

    for (int repetition = 0; repetition < repetitions; ++repetition) {
        const auto begin = std::chrono::steady_clock::now();
        const long long checksum = work();
        const auto end = std::chrono::steady_clock::now();
        const auto elapsed =
            std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

        result.checksum = checksum;
        result.average_microseconds += elapsed;
        result.minimum_microseconds = std::min(result.minimum_microseconds, elapsed);
        result.maximum_microseconds = std::max(result.maximum_microseconds, elapsed);
        elapsed_times.push_back(elapsed);
    }

    result.average_microseconds /= repetitions;
    std::sort(elapsed_times.begin(), elapsed_times.end());
    const std::size_t middle = elapsed_times.size() / 2;
    result.median_microseconds = elapsed_times.size() % 2 == 1
        ? elapsed_times[middle]
        : (elapsed_times[middle - 1] + elapsed_times[middle]) / 2;
    return result;
}

template <int ChunkSize>
BenchmarkResult measure_my_deque(const std::vector<int>& input, int repetitions) {
    return measure_repeated(
        "MyDeque<int, " + std::to_string(ChunkSize) + ">", repetitions, [&input] {
            MyDeque<int, ChunkSize> values;
            for (std::size_t i = 0; i < input.size(); ++i) {
                if ((i & 1U) == 0) values.push_front(input[i]);
                else values.push_back(input[i]);
            }

            long long checksum = 0;
            bool take_front = true;
            while (!values.empty()) {
                if (take_front) {
                    checksum += values.front();
                    values.pop_front();
                } else {
                    checksum += values.back();
                    values.pop_back();
                }
                take_front = !take_front;
            }
            return checksum;
        });
}

BenchmarkResult measure_std_deque(const std::vector<int>& input, int repetitions) {
    return measure_repeated("std::deque<int>", repetitions, [&input] {
        std::deque<int> values;
        for (std::size_t i = 0; i < input.size(); ++i) {
            if ((i & 1U) == 0) values.push_front(input[i]);
            else values.push_back(input[i]);
        }

        long long checksum = 0;
        bool take_front = true;
        while (!values.empty()) {
            if (take_front) {
                checksum += values.front();
                values.pop_front();
            } else {
                checksum += values.back();
                values.pop_back();
            }
            take_front = !take_front;
        }
        return checksum;
    });
}

void print_benchmark_result(int case_number, const BenchmarkResult& result,
                            int chunk_bytes, int elements_per_chunk) {
    std::cout << "\n[Case " << case_number << "] " << result.name << '\n';
    if (chunk_bytes > 0) {
        std::cout << "  Requested chunk   : " << chunk_bytes << " bytes\n"
                  << "  Ints per chunk    : " << elements_per_chunk << '\n';
    } else {
        std::cout << "  Block policy      : implementation-defined by MSVC STL\n";
    }
    std::cout << "  Average time      : " << result.average_microseconds << " us\n"
              << "  Median time       : " << result.median_microseconds << " us\n"
              << "  Minimum time      : " << result.minimum_microseconds << " us\n"
              << "  Maximum time      : " << result.maximum_microseconds << " us\n"
              << "  Checksum          : " << result.checksum << '\n';
}

int run_benchmark(int element_count, int repetitions) {
    const auto input = test_support::make_int_input(element_count);
    const auto chunk16 = measure_my_deque<16>(input, repetitions);
    const auto chunk64 = measure_my_deque<64>(input, repetitions);
    const auto chunk256 = measure_my_deque<256>(input, repetitions);
    const auto chunk1024 = measure_my_deque<1024>(input, repetitions);
    const auto chunk4096 = measure_my_deque<4096>(input, repetitions);
    const auto standard = measure_std_deque(input, repetitions);

    for (const auto& result : {chunk16, chunk64, chunk256, chunk1024, chunk4096}) {
        require(result.checksum == standard.checksum, "benchmark checksum mismatch");
    }

    std::cout << "MyDeque chunk-size load benchmark\n"
              << "  Current build     : " << test_support::build_configuration() << " x64\n"
              << "  Recommended build : Release x64\n"
              << "  Element type      : int\n"
              << "  Workload          : alternating push_front/push_back, then alternating pop\n"
              << "  Elements per case : " << element_count << '\n'
              << "  Measurements      : " << repetitions << " per case\n"
              << "  Time unit         : microseconds (us)\n";

    print_benchmark_result(1, chunk16, 16, 4);
    print_benchmark_result(2, chunk64, 64, 16);
    print_benchmark_result(3, chunk256, 256, 64);
    print_benchmark_result(4, chunk1024, 1024, 256);
    print_benchmark_result(5, chunk4096, 4096, 1024);
    print_benchmark_result(6, standard, 0, 0);
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--benchmark") {
        const int element_count = argc > 2 ? std::stoi(argv[2]) : 10000;
        const int repetitions = argc > 3 ? std::stoi(argv[3]) : 10;
        if (element_count <= 0 || repetitions <= 0) {
            std::cerr << "Element count and repetitions must be positive integers.\n";
            return 1;
        }
        return run_benchmark(element_count, repetitions);
    }

    std::cout << "MyDeque validity tests ("
              << test_support::build_configuration() << ")\n\n";

    TestRunner runner(7);
    runner.run("default state and invalid-access exceptions", test_default_state_and_exceptions);
    runner.run("push/pop and chunk boundaries", test_push_pop_and_chunk_boundaries);
    runner.run("circular wrapping and chunk growth", test_wrapping_and_growth);
    runner.run("clear and storage reuse with std::string", test_clear_and_reuse);
    runner.run("copy construction, assignment and self-assignment", test_copy_semantics);
    runner.run("move construction and assignment", test_move_semantics);
    runner.run("5,000 random operations against std::deque", test_against_std_deque);
    return runner.report();
}
