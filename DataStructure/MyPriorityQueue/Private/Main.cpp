#include "MyPriorityQueue.h"
#include "TestUtils.h"

#include <functional>
#include <stdexcept>
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

} // namespace

int main() {
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
