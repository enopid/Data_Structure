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

struct BenchmarkOperation {
    int type;
    int value;
};

std::vector<BenchmarkOperation> make_mixed_operations(int operation_count) {
    std::vector<BenchmarkOperation> operations;
    operations.reserve(static_cast<std::size_t>(operation_count));
    std::mt19937 random(20260906);
    int current_size = 0;

    for (int step = 0; step < operation_count; ++step) {
        const bool push = current_size == 0 || random() % 10 < 6;
        const bool front = (random() & 1U) == 0;
        if (push) {
            operations.push_back({front ? 0 : 1, step});
            ++current_size;
        } else {
            operations.push_back({front ? 2 : 3, 0});
            --current_size;
        }
    }
    return operations;
}

template <typename Deque>
long long run_mixed_workload(const std::vector<BenchmarkOperation>& operations) {
    Deque values;
    long long checksum = 0;
    long long pop_order = 0;

    for (const auto& operation : operations) {
        switch (operation.type) {
        case 0:
            values.push_front(operation.value);
            break;
        case 1:
            values.push_back(operation.value);
            break;
        case 2:
            checksum += static_cast<long long>(values.front()) * ++pop_order;
            values.pop_front();
            break;
        default:
            checksum += static_cast<long long>(values.back()) * ++pop_order;
            values.pop_back();
            break;
        }
    }

    bool take_front = true;
    while (!values.empty()) {
        if (take_front) {
            checksum += static_cast<long long>(values.front()) * ++pop_order;
            values.pop_front();
        } else {
            checksum += static_cast<long long>(values.back()) * ++pop_order;
            values.pop_back();
        }
        take_front = !take_front;
    }
    return checksum;
}

template <int ChunkSize>
BenchmarkResult measure_my_deque(
    const std::vector<BenchmarkOperation>& operations, int repetitions) {
    return measure_repeated(
        "MyDeque<int, " + std::to_string(ChunkSize) + ">", repetitions, [&operations] {
            return run_mixed_workload<MyDeque<int, ChunkSize>>(operations);
        });
}

BenchmarkResult measure_std_deque(
    const std::vector<BenchmarkOperation>& operations, int repetitions) {
    return measure_repeated("std::deque<int>", repetitions, [&operations] {
        return run_mixed_workload<std::deque<int>>(operations);
    });
}

template <typename Container>
long long run_push_back_workload(int element_count) {
    Container values;
    for (int value = 0; value < element_count; ++value) {
        values.push_back(value);
    }
    return static_cast<long long>(values.front())
        + static_cast<long long>(values.back())
        + static_cast<long long>(values.size());
}

template <typename Container>
BenchmarkResult measure_push_back(
    const std::string& name, int element_count, int repetitions) {
    return measure_repeated(name, repetitions, [element_count] {
        return run_push_back_workload<Container>(element_count);
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

int run_benchmark(int operation_count, int repetitions) {
    const auto operations = make_mixed_operations(operation_count);
    const auto chunk16 = measure_my_deque<16>(operations, repetitions);
    const auto chunk64 = measure_my_deque<64>(operations, repetitions);
    const auto chunk256 = measure_my_deque<256>(operations, repetitions);
    const auto chunk1024 = measure_my_deque<1024>(operations, repetitions);
    const auto chunk4096 = measure_my_deque<4096>(operations, repetitions);
    const auto standard = measure_std_deque(operations, repetitions);

    for (const auto& result : {chunk16, chunk64, chunk256, chunk1024, chunk4096}) {
        require(result.checksum == standard.checksum, "benchmark checksum mismatch");
    }

    std::cout << "MyDeque chunk-size load benchmark\n"
              << "  Current build     : " << test_support::build_configuration() << " x64\n"
              << "  Recommended build : Release x64\n"
              << "  Element type      : int\n"
              << "  Workload          : mixed push_front/push_back/pop_front/pop_back\n"
              << "  Push/pop ratio    : approximately 60/40, then drain remaining elements\n"
              << "  Operations        : " << operation_count << '\n'
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

void print_push_back_result(int case_number, const BenchmarkResult& result) {
    std::cout << "\n[Case " << case_number << "] " << result.name << '\n'
              << "  Average time      : " << result.average_microseconds << " us\n"
              << "  Median time       : " << result.median_microseconds << " us\n"
              << "  Minimum time      : " << result.minimum_microseconds << " us\n"
              << "  Maximum time      : " << result.maximum_microseconds << " us\n"
              << "  Checksum          : " << result.checksum << '\n';
}

int run_push_back_benchmark(int element_count, int repetitions) {
    const auto chunk16 = measure_push_back<MyDeque<int, 16>>(
        "MyDeque<int, 16>", element_count, repetitions);
    const auto chunk64 = measure_push_back<MyDeque<int, 64>>(
        "MyDeque<int, 64>", element_count, repetitions);
    const auto chunk256 = measure_push_back<MyDeque<int, 256>>(
        "MyDeque<int, 256>", element_count, repetitions);
    const auto chunk1024 = measure_push_back<MyDeque<int, 1024>>(
        "MyDeque<int, 1024>", element_count, repetitions);
    const auto chunk4096 = measure_push_back<MyDeque<int, 4096>>(
        "MyDeque<int, 4096>", element_count, repetitions);
    const auto standard_deque = measure_push_back<std::deque<int>>(
        "std::deque<int>", element_count, repetitions);

    for (const auto& result : {chunk16, chunk64, chunk256, chunk1024, chunk4096,
                               standard_deque}) {
        require(result.checksum == standard_deque.checksum,
                "push_back benchmark checksum mismatch");
    }

    std::cout << "Sequential push_back benchmark\n"
              << "  Current build     : " << test_support::build_configuration() << " x64\n"
              << "  Recommended build : Release x64\n"
              << "  Element type      : int\n"
              << "  Workload          : sequential push_back only\n"
              << "  Elements          : " << element_count << '\n'
              << "  Measurements      : " << repetitions << " per case\n"
              << "  Time unit         : microseconds (us)\n";

    int case_number = 0;
    for (const auto& result : {chunk16, chunk64, chunk256, chunk1024, chunk4096,
                               standard_deque}) {
        print_push_back_result(++case_number, result);
    }
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc > 1 && (std::string(argv[1]) == "--benchmark"
        || std::string(argv[1]) == "--push-back-benchmark")) {
        const bool push_back_only = std::string(argv[1]) == "--push-back-benchmark";
        const int operation_count = argc > 2 ? std::stoi(argv[2]) : 100000;
        const int repetitions = argc > 3 ? std::stoi(argv[3]) : 10;
        if (operation_count <= 0 || repetitions <= 0) {
            std::cerr << "Operation count and repetitions must be positive integers.\n";
            return 1;
        }
        return push_back_only
            ? run_push_back_benchmark(operation_count, repetitions)
            : run_benchmark(operation_count, repetitions);
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
