#include "MyPriorityQueue.h"
#include "TestUtils.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <limits>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

using test_support::require;
using test_support::TestRunner;

void test_empty_queue() {
    MyPriorityQueue<int> queue;

    require(queue.empty(), "new queue must be empty");
    require(queue.size() == 0, "new queue size must be zero");

    bool threw = false;
    try {
        (void)queue.top();
    } catch (const std::out_of_range&) {
        threw = true;
    }
    require(threw, "top() must reject an empty queue");
}

void test_max_heap_order() {
    MyPriorityQueue<int> queue;
    const std::vector<int> input{4, 1, 9, 3, 9, -2, 7};
    const std::vector<int> expected{9, 9, 7, 4, 3, 1, -2};

    for (int value : input) queue.push(value);
    require(queue.size() == static_cast<int>(expected.size()), "size after push mismatch");

    for (int value : expected) {
        require(queue.top() == value, "max-heap pop order mismatch");
        queue.pop();
    }
    require(queue.empty(), "queue must be empty after every element is popped");
}

void test_custom_comparator() {
    MyPriorityQueue<int, std::greater<int>> queue;
    for (int value : {4, 1, 9, 3}) queue.push(value);

    for (int expected : {1, 3, 4, 9}) {
        require(queue.top() == expected, "custom comparator order mismatch");
        queue.pop();
    }
}

void test_handle_access() {
    MyPriorityQueue<int> queue;
    const auto first = queue.push(10);
    const auto second = queue.push(30);

    require(queue.valid(first), "first handle must be valid");
    require(queue.valid(second), "second handle must be valid");
    require(queue.get(first) == 10, "first handle points to the wrong value");
    require(queue.get(second) == 30, "second handle points to the wrong value");
}

void test_update() {
    MyPriorityQueue<int> queue;
    const auto first = queue.push(10);
    const auto second = queue.push(20);
    const auto third = queue.push(30);

    queue.update(first, 40);
    require(queue.top() == 40, "increased value did not move upward");
    require(queue.get(first) == 40, "updated handle has the wrong value");

    queue.update(first, 5);
    require(queue.top() == 30, "decreased value did not move downward");
    require(queue.get(first) == 5, "decreased value was not stored");
    require(queue.get(second) == 20 && queue.get(third) == 30,
            "update changed another handle's value");
}

void test_erase_and_pop_invalidate_handles() {
    MyPriorityQueue<int> queue;
    const auto low = queue.push(10);
    const auto middle = queue.push(20);
    const auto high = queue.push(30);

    queue.erase(middle);
    require(!queue.valid(middle), "erased handle must be invalid");
    require(queue.size() == 2, "erase did not reduce size");
    require(queue.top() == 30, "erase broke heap order");

    queue.pop();
    require(!queue.valid(high), "popped handle must be invalid");
    require(queue.valid(low) && queue.top() == 10, "remaining handle or value is invalid");

    bool threw = false;
    try {
        queue.erase(middle);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    require(threw, "erase must reject an invalid handle");
}

void test_clear_copy_and_move() {
    MyPriorityQueue<int> original;
    const auto old_handle = original.push(10);
    original.push(40);

    MyPriorityQueue<int> copied(original);
    require(copied.size() == 2 && copied.top() == 40, "copy construction failed");

    MyPriorityQueue<int> moved(std::move(copied));
    require(moved.size() == 2 && moved.top() == 40, "move construction failed");

    original.clear();
    require(original.empty() && original.size() == 0, "clear did not empty the queue");
    require(!original.valid(old_handle), "clear did not invalidate an old handle");

    const auto new_handle = original.push(25);
    require(original.top() == 25 && original.valid(new_handle), "queue cannot be reused after clear");
    require(!original.valid(old_handle), "reused slot revived a stale handle");
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

void print_benchmark_result(int case_number, const BenchmarkResult& result,
                            std::size_t element_count) {
    std::cout << "\n[Case " << case_number << "] " << result.name << '\n'
              << "  Elements          : " << element_count << '\n'
              << "  Average time      : " << result.average_microseconds << " us\n"
              << "  Median time       : " << result.median_microseconds << " us\n"
              << "  Minimum time      : " << result.minimum_microseconds << " us\n"
              << "  Maximum time      : " << result.maximum_microseconds << " us\n"
              << "  Checksum          : " << result.checksum << '\n';
}

int run_benchmark(int element_count, int repetitions) {
    std::mt19937 random(20260903);
    std::uniform_int_distribution<int> value_distribution(0, 1'000'000);
    std::vector<int> input;
    input.reserve(static_cast<std::size_t>(element_count));
    for (int i = 0; i < element_count; ++i) input.push_back(value_distribution(random));

    const auto custom = measure_repeated("MyPriorityQueue", repetitions, [&input] {
        MyPriorityQueue<int> queue;
        long long checksum = 0;
        for (int value : input) queue.push(value);
        while (!queue.empty()) {
            checksum += queue.top();
            queue.pop();
        }
        return checksum;
    });

    const auto standard = measure_repeated("std::priority_queue", repetitions, [&input] {
        std::priority_queue<int> queue;
        long long checksum = 0;
        for (int value : input) queue.push(value);
        while (!queue.empty()) {
            checksum += queue.top();
            queue.pop();
        }
        return checksum;
    });

    require(custom.checksum == standard.checksum, "benchmark result checksum mismatch");

    std::cout << "Priority queue load benchmark\n"
              << "  Current build     : " << test_support::build_configuration() << " x64\n"
              << "  Recommended build : Release x64\n"
              << "  Workload          : push all, then pop all\n"
              << "  Elements per case : " << input.size() << '\n'
              << "  Measurements      : " << repetitions << " per case\n"
              << "  Time unit         : microseconds (us)\n";
    print_benchmark_result(1, custom, input.size());
    print_benchmark_result(2, standard, input.size());
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--benchmark") {
        const int element_count = argc > 2 ? std::stoi(argv[2]) : 100000;
        const int repetitions = argc > 3 ? std::stoi(argv[3]) : 10;
        if (element_count <= 0 || repetitions <= 0) {
            std::cerr << "Element count and repetitions must be positive integers.\n";
            return 1;
        }
        return run_benchmark(element_count, repetitions);
    }

    std::cout << "MyPriorityQueue validity tests ("
              << test_support::build_configuration() << ")\n\n";

    TestRunner runner(7);
    runner.run("empty queue and top exception", test_empty_queue);
    runner.run("max-heap push/pop order", test_max_heap_order);
    runner.run("custom comparator", test_custom_comparator);
    runner.run("handle validity and access", test_handle_access);
    runner.run("priority update in both directions", test_update);
    runner.run("erase/pop handle invalidation", test_erase_and_pop_invalidate_handles);
    runner.run("clear, copy, and move", test_clear_copy_and_move);
    return runner.report();
}
