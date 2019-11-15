# Battleship Enumeration

This is a repository containing various solvers for the game battleship.
Given "miss", "hit", and "sink" points (sink points may also specify a ship length),
we assume a uniform distribution over the valid configurations of the ships,
and compute the exact probability that each square on the board overlaps with a ship.

Below is a brief comparison of the different solvers:

## Brute Force
This currently has some issues with integer overflow, but is far too slow to be useful regardless.
- Simplest approach
- Iterate through the ships, and try each placement. Record the successful placements.
- Does not support "sink" messages.

## Brute Force stop
This makes a very simple change over the brute force method:
If, while trying a partial placement of the ships, an overlap is found, it skips the sub-calls.
- Faster than simple Brute Force
- Can solve 8x8 boards relatively efficiently!

## Sharp Sat
This takes the approach of trying to reduce the problem to a SAT counting problem,
and gere
