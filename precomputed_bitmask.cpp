#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

// CONSTANTS
// board dimensions
#define WIDTH 8
#define HEIGHT 8
/*
#define WIDTH 10
#define HEIGHT 10
//boats
const vector<int> sizes = {2,3,3,4,5};
*/
const vector<int> sizes = {2,3,3};
const int n = sizes.size();

typedef vector<ll> grid_t;

grid_t create_grid()
{
	return grid_t(WIDTH*HEIGHT);
}

// produce the number of unique horizontal or vertical positions that a
// top-left coordinate can take on in the corresponding orientation
int horizontal_state_class_count(int length)
{
	return (WIDTH - length + 1);
}
int vertical_state_class_count(int length)
{
	return (HEIGHT - length + 1);
}
// produce number of horizontal states for a single ship on a clean board
int horizontal_state_count(int length)
{
	return horizontal_state_count(length)*HEIGHT;
}
int vertical_state_count(int length)
{
	return vertical_state_count(length)*WIDTH;
}
// produce the number of states for a ship on a clean board
int state_count(int length)
{
	return horizontal_state_count(length) + vertical_state_count(length);
}

// (x,y,dx,dy)
typedef tuple<int,int,int,int> dpos_t;

// given a state index and a ship length, produce (x,y,dx,dy)
dpos_t get_state_dpos(int state_index, int length)
{
	// define the canonical ordering to be left to right,
	// top to bottom, horizontal to vertical
	// all are defined by their top-left coordinate

	// number of states in a given row for the horizontal orientation
	int horizontal_width = horizontal_state_class_count(length);

	// number of states with horizontal orientation
	int num_in_horizontal = horizontal_width*HEIGHT;
	if (state_index < num_in_horizontal)
	{
		// horizontal orientation
		return make_tuple(state_index % horizontal_width, state_index / horizontal_width, 1,0);
	}
	//vertical orientation
	state_index %= num_in_horizontal;
	return make_tuple(state_index % WIDTH, state_index / WIDTH, 0,1);
}

// given a state index and a ship length, draw it onto a grid with a multiplier,
// and return false if a conflict is found, but only after finishing the drawing
bool draw_state(int state_index, int length, ll mult, grid_t &grid)
{
	int x, y, dx, dy;
	tie(x,y,dx,dy) = get_state_dpos(state_index, length);
	bool ret = true;
	for (int l = 0; l < length; ++l)
	{
		grid[x+y*WIDTH] += mult;

		if (grid[x+y*WIDTH] > 1) // this grid cell is covered by two ships, so the drawing is invalid
			ret = false;

		x += dx;
		y += dy;
	}
	return ret;
}

// returns a list of state indeces that do not conflict with misses
vector<int> find_valid_individual(int length, grid_t &misses)
{
	vector<int> res; // final result
	// check validity of each state index
	for (int state_index = 0; state_index < state_count(length); ++state_index)
	{
		// draw and find validity
		if (draw_state(state_index, length, 1, misses))
			res.push_back(state_index);
		// undraw
		draw_state(state_index, length, -1, misses);
	}
	return res;
}

int main()
{
}
