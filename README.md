# Battleship Enumeration
This is a repository containing various solvers for the game battleship.
Given "miss", "hit", and "sink" points (sink points may also specify a ship length),
we assume a uniform distribution over the valid configurations of the ships,
and compute the exact probability that each square on the board overlaps with a ship.

Everything should be compiled with `g++ -O3 -std=c++11` for expected performance.

Below is a brief comparison of the different solvers

## Brute Force
This currently has some issues with integer overflow, but is far too slow to be useful regardless.
- Simplest approach
- Iterate through the ships, and try each placement. Record the successful placements.
- Does not support "sink" messages.

## Brute Force stop
This makes a very simple change over the brute force method.
If, while trying a partial placement of the ships, an overlap is found, it skips the sub-calls.
- Faster than simple Brute Force
- Can solve 8x8 boards relatively efficiently!

## Sharp Sat
This takes the approach of trying to reduce the problem to a SAT counting problem,
and generates a `cnf` file.
Shapsat solvers running on the resulting cnf appear to run slower than even the brute force solver.

## Precomputed Bitmask
This converts the placements of each ship into a set of integers, represented by a bitmask.
It then runs the brute force algorithm on this representation, to lots of success.
This particular version does not support "hit" or "sink" messages.
- Very fast! Even this basic version runs in around 5 minutes on a laptop.
- Non-trivial. This algorithm uses several steps of pre-computation and several steps in the branching routine.
- Uses the `C++` `bitset<N>` implementation of arbitrary-sized bitmasks.

## Precomputed Bitmask Custom
This is the same as the non-custom version, but it reimplements `bitset`,
allowing for a bitscan operation.
In practice, this is slower than the non-custom version, but is important for soon-to-come solvers.

## Precomputed Bitmask Unroll
This version uses templates to unroll the branching algorithm at compile-time.
- Currently, this is the fastest solver, running at around 1m6s on the laptop this repo was written on.
- This optimization cannot be done at all if the ship lengths and count are determined dynamically.

## Precomputed Bitmask Custom Unroll
This combines the last two modifications, and runs in about 1m16s.
This will be the base for online appoaches.
