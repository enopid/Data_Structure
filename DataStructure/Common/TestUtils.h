#pragma once

#include <functional>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace test_support {

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
        std::cout << '\n' << passed_ << " passed, " << failed_ << " failed\n";
        return failed_ == 0 ? 0 : 1;
    }

private:
    int total_ = 0;
    int executed_ = 0;
    int passed_ = 0;
    int failed_ = 0;
};

inline void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

inline constexpr const char* build_configuration() {
#ifdef NDEBUG
    return "Release";
#else
    return "Debug";
#endif
}

inline std::vector<int> make_int_input(int count) {
    std::vector<int> input;
    input.reserve(static_cast<std::size_t>(count));
    for (int value = 0; value < count; ++value) input.push_back(value);
    return input;
}

inline std::vector<std::string> make_string_input(int count, unsigned int seed) {
    constexpr char characters[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    constexpr int string_length = 24;
    std::mt19937 random(seed);
    std::uniform_int_distribution<std::size_t> pick(0, sizeof(characters) - 2);
    std::vector<std::string> input;
    input.reserve(static_cast<std::size_t>(count));

    for (int index = 0; index < count; ++index) {
        std::string value;
        value.reserve(string_length);
        for (int character = 0; character < string_length; ++character) {
            value.push_back(characters[pick(random)]);
        }
        input.push_back(std::move(value));
    }
    return input;
}

} // namespace test_support
