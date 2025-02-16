#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <fmt/core.h>
#include <limits>
#include <sysexits.h>
#include <unordered_set>
#include <vector>

#include "utils.h"

using water = uint16_t; // Water level measurement

template <typename T>
constexpr T type_max() noexcept {
    return std::numeric_limits<T>::max();
}

/// Three water vessel's current contents in liters of water, the volumes must be kept in one more State variable.
// Inherits comparison operators from std::array<T, S>()
class VesselsState: public std::array<water, 3> {
public:
    constexpr VesselsState() noexcept: std::array<water, 3>({0, 0, 0}) {}
    constexpr VesselsState(water a, water b, water c) noexcept: std::array<water, 3>({a, b, c}) {}

    /// Hash for unordered containers, c++ 23 it can even be static (__cpp_static_call_operator, P1169R3)
    constexpr size_t operator()(const VesselsState &state) const noexcept {
        return (size_t(state[0]) * type_max<water>() + state[1]) * type_max<water>() + state[2];
    };

    /// Return new state after transferring water
    [[nodiscard]]
    VesselsState transfer(unsigned src, unsigned dst, const VesselsState &volumes) const noexcept {
        VesselsState result = *this; // copy
        const water dst_free = volumes.at(dst) - this->at(dst);
        if (this->at(src) <= dst_free) {
            result.at(dst) += this->at(src);
            result.at(src) = 0;
        } else {
            result.at(dst) += dst_free;
            result.at(src) -= dst_free;
        }
        return result;
    }

    /// Calculate all possible next states
    [[nodiscard]]
    std::vector<VesselsState> next_states(const VesselsState &volumes) const {
        std::vector<VesselsState> result;
        result.reserve(12); // up to 12, use only 1 memory allocation

        for (unsigned from = 0; from < 3; from++) {
            // Fill (up to 3)
            if (this->at(from) == 0) {
                VesselsState new_state = *this;
                new_state.at(from) = volumes.at(from);
                result.push_back(new_state);
            }

            // Drain (up to 3)
            if (this->at(from) != 0) {
                VesselsState new_state = *this;
                new_state.at(from) = 0;
                result.push_back(new_state);
            }

            // Transfer (up to 6)
            for (unsigned to = 0; to < 3; to++) {
                if (from != to && this->at(to) < volumes.at(to) && this->at(from) > 0) {
                    result.push_back(transfer(from, to, volumes));
                }
            }
        }

        result.shrink_to_fit(); // And trim the unused part on the right (if any)
        return result;
    }

    // Do we contain the specified volume of water in any vessel?
    [[nodiscard]]
    constexpr bool contains(water volume) const noexcept {
//        return std::find(this->begin(), this->end(), volume) != this->end(); // C++20
        return this->at(0) == volume || this->at(1) == volume || this->at(2) == volume;
    }
};

// "Unit test" for C++20 and above
#if __cplusplus >= 202002L
static_assert(VesselsState{1, 2, 3} == VesselsState{1, 2, 3});
static_assert(VesselsState{1, 2, 3} != VesselsState{2, 2, 3});
static_assert(VesselsState{2, 2, 3} != VesselsState{1, 2, 3});
static_assert(VesselsState{1, 2, 8} != VesselsState{1, 2, 3});
static_assert(VesselsState{1, 2, 3} != VesselsState{1, 2, 8});
static_assert(VesselsState{1, 2, 3} < VesselsState{1, 2, 4});
static_assert(VesselsState{2, 2, 3} > VesselsState{1, 2, 4});
#endif

/// Solve the water pouring puzzle with tap, sink and empty initial state.
class WaterPouringPuzzleSolver {
    using History = std::vector<std::pair<VesselsState, int>>;
    using Visited = std::unordered_set<VesselsState, VesselsState>;
#if __cplusplus < 201703L
    enum { INVALID_IDX = -1 };
#else
    constexpr inline static const int INVALID_IDX = -1;
#endif

protected:
    VesselsState m_volumes;
    History m_history{}; // State discovery history
    Visited m_visited{}; // States visited

public:
    explicit WaterPouringPuzzleSolver(const VesselsState &volumes): m_volumes(volumes) {}

    /// Returns in how many steps it can be solved (and prints it), -1 is no solution
    int solve_water(const water target) {
        if (target == 0) {
            puts("All vessels are empty initially, all have 0 liters of water, 0 steps!");
            return 0;
        }

        init(); // Allow the method to be called multiple times, optimize the number of memory allocations

        m_visited.emplace(0, 0, 0);  // We don't want to empty all of them
        m_visited.insert(m_volumes); // We also don't want to fill all of them

        int step = 0;                                               // count steps
        size_t old_ptr = 0;                                         // All elements [0 .. history.size()) are new
        m_history.emplace_back(VesselsState{0, 0, 0}, INVALID_IDX); // Initial state

        while (old_ptr != m_history.size()) {
            ++step;

            const size_t next_ptr = m_history.size();
            for (size_t ptr = old_ptr; ptr < next_ptr; ++ptr) {
                const VesselsState &old_state = m_history.at(ptr).first;

                for (const VesselsState new_state : old_state.next_states(m_volumes)) {
                    if (m_visited.find(new_state) != m_visited.end()) {
                        continue;
                    }
                    m_visited.insert(new_state);
                    m_history.emplace_back(new_state, ptr);

                    if (new_state.contains(target)) {
                        show(target, step);
                        return step;
                    }
                }
            }

            old_ptr = next_ptr;
        }

        return -1; // No new state transitions possible, no solution
    }

protected:
    void init() {
        // Allow the method to be called multiple times
        m_history.clear();
        m_visited.clear();
        // Save some memory allocations
        m_history.reserve(256);
        m_visited.reserve(256);
    }

    /// Print the solution
    void show(const water target, int steps) {
        if (steps <= 0) {
            return;
        }
        assert(!m_history.empty());

        fmt::print("Solved measure {} liters of water using {}, {} and {} vessels in {} steps\n", target,
                   m_volumes.at(0), m_volumes.at(1), m_volumes.at(2), steps);
        fmt::print("┌──────┬─────┬─────┬─────┐\n");
        fmt::print("│ Step │ {: >3} │ {: >3} │ {: >3} │\n", m_volumes.at(0), m_volumes.at(1), m_volumes.at(2));
        fmt::print("├──────┼─────┼─────┼─────┤\n");

        // If only the first solution is needed we can modify the history to reverse the index pointers and walk
        // forward.

        std::vector<int> solution;
        solution.resize(static_cast<size_t>(steps) + 1);

        int history_idx = static_cast<int>(m_history.size() - 1);
        for (int pos = steps; pos != -1; --pos) {
            solution.at(static_cast<size_t>(pos)) = history_idx;                 // Save current
            history_idx = m_history.at(static_cast<size_t>(history_idx)).second; // travel back

            if (pos == 0) {
                assert(history_idx == -1);
            } else {
                assert(history_idx >= 0);
            }
        }

        for (int i = 0; i != steps + 1; ++i) {
            const VesselsState &state = m_history.at(static_cast<size_t>(solution.at(static_cast<size_t>(i)))).first;
            fmt::print("│ {: >3}. │ {: >3} │ {: >3} │ {: >3} │\n", i, state.at(0), state.at(1), state.at(2));
        }
        fmt::print("└──────┴─────┴─────┴─────┘\n");
    }
};

int main(int argc, char *argv[]) {
    if (argc != 5) {
        puts("Solve the three water vessels, tap and sink problem.\n\n"
             "Usage:\n\twater LIMIT_1 LIMIT_2 LIMIT_3 TARGET\n\n"
             "Example:\n\twater 3 5 8 4");

        return EX_USAGE;
    }

    VesselsState volumes{};
    water target{};

    { // Use some stack memory temporary
        std::array<water, 5> numbers{};
        for (int i = 1; i < 5; ++i) {
            char *end = nullptr;
            const long result = strtol(argv[i], &end, 10);
            numbers[i] = static_cast<water>(result);
            if (end == argv[i] || *end != '\0' || numbers[i] != result) {
                fmt::print("Invalid number (argument {}): '{}'!\n", i, argv[i]);
                return EX_DATAERR;
            }
        }
        std::sort(&numbers[1], &numbers[3]); // Not really needed
        volumes = VesselsState(numbers[1], numbers[2], numbers[3]);
        target = numbers[4];
    }

    // Quick check
    const auto volume_gcd = gcd(volumes.at(0), volumes.at(1), volumes.at(2));
    fmt::print("GCD indicates the puzzle is {}solvable!\n", (target % volume_gcd != 0 ? "un" : ""));

    // Try to solve it
    WaterPouringPuzzleSolver solver{volumes};
    if (solver.solve_water(target) < 0) {
        puts("No solution found!");
        return EX_UNAVAILABLE;
    }

    return EX_OK;
}
