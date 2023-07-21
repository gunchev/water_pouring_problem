#!/usr/bin/python3
# -*- coding: utf-8 -*-

"""Solve the three water vessels, tap and sink problem."""

from typing import List, Iterable, MutableSet
from functools import total_ordering
from math import gcd
import sys


@total_ordering
class VesselsState:
    """An immutable object representing the volume of water in three vessels."""
    # In reality, it is mutable but mutating it will break sets and dictionaries using it as a key.
    __slots__ = ['__a', '__b', '__c']  # Save some memory, we will have a lot of these, 56 bytes each

    def __init__(self, i: int, j: int, k: int):
        self.__a, self.__b, self.__c = i, j, k
        # Can skip on production for speed
        for val in (i, j, k):
            if not isinstance(val, int):
                raise TypeError(f'invalid water volume type {type(val).__name__}')
            if val < 0 or not isinstance(val, int):
                raise ValueError(f'invalid water volume of {val}')

    def __iter__(self):
        yield self.__a
        yield self.__b
        yield self.__c

    def __getitem__(self, val):
        return tuple(self)[val]

    def __eq__(self, other):
        return tuple(self) == tuple(other)

    def __lt__(self, other):
        return tuple(self) < tuple(other)

    def __hash__(self):
        return hash((self.__a, self.__b, self.__c))

    def __str__(self):
        return f'{type(self).__name__}<{self.__a}, {self.__b}, {self.__c}>'

    __repr__ = __str__


class Actor:
    """The person playing the game ;-)"""

    def __init__(self, limits: VesselsState):
        self._limits = tuple(limits)  # Easier to use this way

    def transfer(self, vessels: VesselsState, src: int, dst: int) -> VesselsState:
        """Transfer water from one vessel to another and return the new state"""
        result = list(vessels)
        to_move = min(result[src], self._limits[dst] - result[dst])
        result[dst] += to_move
        result[src] -= to_move
        return VesselsState(*result)

    def next_states(self, state: VesselsState) -> Iterable[VesselsState]:
        """Generate next possible states"""
        for src in range(3):
            new_state: List[int]
            # Fill (up to 3 if all empty)
            if state[src] == 0:
                new_state = list(state)
                new_state[src] = self._limits[src]
                yield VesselsState(*new_state)

            # Drain (up to 3 if all non-empty)
            if state[src] != 0:
                new_state = list(state)
                new_state[src] = 0
                yield VesselsState(*new_state)

            # Transfer (up to 6 if all non-empty and non-full)
            for dst in range(3):
                if src != dst and state[dst] < self._limits[dst] and state[src] > 0:
                    yield self.transfer(state, src, dst)


class PuzzleStep:  # pylint: disable=too-few-public-methods
    """Step in the puzzle solution history"""
    __slots__ = ['state', 'prev']  # Save some memory, we will have quite a lot of these, 48 bytes

    def __init__(self, state: VesselsState, prev: int):
        self.state = state
        self.prev = prev

    def __str__(self):
        return f'{type(self).__name__}<state={self.state}, prev={self.prev}>'

    __repr__ = __str__


class Puzzle:  # pylint: disable=too-few-public-methods
    """Measuring volume of water using three vessels puzzle."""
    INVALID_IDX = -1

    def __init__(self, limits: VesselsState):
        self._limits = limits
        self._actor = Actor(limits)
        self._visited: MutableSet[VesselsState] = set()
        self._history: List[PuzzleStep] = []

    def solve(self, target: int) -> bool:
        """Solve the puzzle for target volume of water"""
        if target == 0:
            print("All vessels are empty initially, all have 0 liters of water, 0 steps!")
            return True

        # Allow the method to be called multiple times, optimize the number of memory allocations
        self._visited.clear()
        self._history.clear()

        self._visited.add(VesselsState(0, 0, 0))  # We don't want to empty all of them
        self._visited.add(self._limits)  # We don't want to empty all of them

        step: int = 0  # count the steps
        old_ptr: int = 0  # All elements [0 .. len(history)) are new
        self._history.append(PuzzleStep(VesselsState(0, 0, 0), self.INVALID_IDX))  # Initial state

        while old_ptr != len(self._history):
            step += 1
            next_ptr = len(self._history)
            for ptr in range(old_ptr, next_ptr):
                old_state = self._history[ptr].state

                for new_state in self._actor.next_states(old_state):
                    if new_state in self._visited:
                        continue

                    self._visited.add(new_state)
                    self._history.append(PuzzleStep(new_state, ptr))

                    if target in new_state:
                        self._show_current_solution(target, step)
                        return True  # We can keep generating and printing solutions here...

            old_ptr = next_ptr

        return False

    def _show_current_solution(self, target: int, steps: int):
        """Show/print the current solution"""
        assert steps > 0
        assert len(self._history) != 0

        solution: List[int] = [self.INVALID_IDX] * (steps + 1)
        history_idx: int = len(self._history) - 1
        pos: int = steps
        while pos != self.INVALID_IDX:
            solution[pos] = history_idx  # Save current
            history_idx = self._history[history_idx].prev  # travel back
            if pos == 0:
                assert history_idx == -1
            else:
                assert history_idx >= 0
            pos -= 1

        print(f"Measuring {target} liters of water using {self._limits[0]}, {self._limits[1]}"
              f" and {self._limits[2]} vessels in {steps} steps.")
        print("+------+-----+-----+-----+")
        print(f"| Step | {self._limits[0]: >3} | {self._limits[1]: >3} | {self._limits[2]: >3} |")
        print("+------+-----+-----+-----+")

        for step, idx in enumerate(solution):
            data = tuple(self._history[idx].state)
            print(f"| {step: >3}. | {data[0]: >3} | {data[1]: >3} | {data[2]: >3} |")

        print("+------+-----+-----+-----+")


def main():
    """Main method"""
    if len(sys.argv) != 5:
        print(__doc__, file=sys.stderr)  # @UndefinedVariable
        print(f"\nUsage:\n\t{sys.argv[0]} LIMIT_1 LIMIT_2 LIMIT_3 TARGET", file=sys.stderr)
        return 64  # EX_USAGE

    try:
        nums = list(int(i) for i in sys.argv[1:])
    except ValueError as exc:
        print(exc, file=sys.stderr)
        return 65  # EX_DATAERR

    for i in nums:
        if i <= 0:
            print(f"Invalid input {i}!", file=sys.stderr)
            return 65  # EX_DATAERR

    # Quick check for solvability.
    print(f'GCD indicates the puzzle is {"" if nums[3] % gcd(*nums[0:3]) == 0 else "un"}solvable!')

    limits = VesselsState(*nums[0:3])
    target = int(nums[3])
    puzzle = Puzzle(limits)

    if puzzle.solve(target):
        return 0

    print("No solution!", file=sys.stderr)  # Can double-check with 0 == target % GCD(*nums[0:3])
    return 69  # EX_UNAVAILABLE


if __name__ == '__main__':
    sys.exit(main())
