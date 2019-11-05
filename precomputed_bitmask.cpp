#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

// CONSTANTS
// board dimensions
#define WIDTH 10
#define HEIGHT 10
/*
#define WIDTH 10
#define HEIGHT 10
//boats
const vector<int> lengths = {2,3,3,4,5};
*/
const vector<int> lengths = {2,3,3,4,5};
const int n = lengths.size();

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
	return horizontal_state_class_count(length)*HEIGHT;
}
int vertical_state_count(int length)
{
	return vertical_state_class_count(length)*WIDTH;
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
vector<int> find_valid_individual_states(int length, grid_t &misses)
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

// returns a list of position indeces that do not conflict with misses for each ship
vector<vector<int>> find_valid_states(grid_t &misses)
{
	vector<vector<int>> res(n);
	for (int i = 0; i < n; ++i)
		res[i] = find_valid_individual_states(lengths[i], misses);
	return res;
}

typedef bitset<WIDTH*(HEIGHT-1)+HEIGHT*(WIDTH-1)> pos_set;

bool compatible_pair(int length_a, int state_a, int length_b, int state_b, grid_t &test_grid)
{
	bool valid = draw_state(state_a,length_a,1,test_grid);
	valid = draw_state(state_b,length_b,1,test_grid) && valid;
	draw_state(state_a,length_a,-1,test_grid);
	draw_state(state_b,length_b,-1,test_grid);
	return valid;
}

pos_set all_compatible_pairs(int length_a, int state_a, int length_b, vector<int> &states_b, grid_t &test_grid)
{
	pos_set res;
	int i = 0;
	for (int i = 0; i < states_b.size(); ++i)
	{
		int state_b = states_b[i];
		if (compatible_pair(length_a, state_a, length_b, state_b, test_grid))
			res[i] = true;
	}
	return res;
}

vector<vector<vector<pos_set>>> find_all_pos_sets(vector<vector<int>> &valid_states)
{
	// create a single test grid to avoid reallocating memory needlessly
	grid_t test_grid = create_grid();
	// indexed by ship1, position of ship1, ship2
	vector<vector<vector<pos_set>>> res(n);
	for (int i = 0; i < n; ++i)
	{
		res[i].resize(valid_states[i].size());
		for (int state_i = 0; state_i < valid_states[i].size(); ++state_i) // for each state of ship i
			res[i][state_i].resize(n);
	}

	for (int i = 0; i < n; ++i) // for each ship
		for (int state_i = 0; state_i < valid_states[i].size(); ++state_i) // for each state of ship i
			for (int j = i+1; j < n; ++j) // for each ship that will be seen later than i
				res[i][state_i][j] = all_compatible_pairs(lengths[i], valid_states[i][state_i], lengths[j], valid_states[j], test_grid);
	return res;
}

ll place_ship(const int ship_index, const vector<vector<vector<pos_set>>> &validity_masks,
		vector<pos_set> &currently_valid, vector<vector<int>> &state_frequency, const vector<int> &num_valid_states, ll &total_successful)
{
	// base case: all ships placed successfully
	if (ship_index == n)
	{
		if (++total_successful % 100000000 == 0)
			cout << "Cumulative successful: " << total_successful << endl;
		return 1;
	}

	// if there are no valid placements of this ship, return (TODO: Case may be unnecessary)
	// TODO: This actually gives a segfault??
	//if (currently_valid[ship_index].none())
	//	return 0;

	// Save the information about currently_valid that we will need when we return from recursive calls
	vector<pos_set> edits(n-ship_index-1);
	for (int j = ship_index + 1; j < n; ++j)
		edits[j-ship_index-1] = currently_valid[j];

	// count of the number of valid placements of all remaining ships
	ll count = 0;

	// iterate over all the ship states that are still valid
	// TODO: remove the wasteful loops (currently, this code will run about 10^11 times)
	//       use __builtin_ffsll (returns 1 + the index of the first set bit, or 0 if no set bits)
	//       this is a (forward) bitscan. MSVC also has a function for it (_BitScanForward64)
	for (int state_index = 0; state_index < num_valid_states[ship_index]; ++state_index)
	{
		if (currently_valid[ship_index][state_index])
		{
			// update legal states for remaining ships
			for (int j = ship_index + 1; j < n; ++j)
				currently_valid[j] &= validity_masks[ship_index][state_index][j];
			// recurse on remaining ships
			ll sub_result = place_ship(ship_index + 1, validity_masks, currently_valid, state_frequency, num_valid_states, total_successful);
			// record counts
			count += sub_result;
			state_frequency[ship_index][state_index] += sub_result;
			// set currently_valid values back
			for (int j = ship_index + 1; j < n; ++j)
				currently_valid[j] = edits[j-ship_index-1];
		}
	}
	return count;
}

// print the final grid
void print_grid(const grid_t &a)
{
	for (int y = 0; y < HEIGHT; ++y)
		for (int x = 0; x < WIDTH; ++x)
			cout << a[x+y*WIDTH] << "\t\n"[x == WIDTH-1];
}

// print the final grid of probabilities
void print_grid_chance(const grid_t &a, const ll &added_count)
{
	cout << fixed << setprecision(3); //show 3 decimals for chance
	double count = added_count;
	for (int y = 0; y < HEIGHT; ++y)
		for (int x = 0; x < WIDTH; ++x)
			cout << double(a[x+y*WIDTH])/count << "\t\n"[x == WIDTH-1];
}

// list of hits not currently supported, need to derive new algorithm
// count the number of occurrences of each spot on the grid
// do this by counting the frequency of each position for each ship
void count_occurrences(grid_t &misses)
{
	vector<vector<int>> valid_states = find_valid_states(misses);
	// count the number of valid states, to be used when placing ships
	vector<int> num_valid_states(n);
	for (int i = 0; i < n; ++i)
		num_valid_states[i] = valid_states[i].size();

	vector<vector<vector<pos_set>>> validity_masks = find_all_pos_sets(valid_states);

	vector<pos_set> currently_valid(n); // for each ship, keep track of which states are still valid
	for (int i = 0; i < n; ++i)
		for (int state_i = 0; state_i < num_valid_states[i]; ++state_i)
			currently_valid[i][state_i] = true;

	vector<vector<int>> state_frequency(n); // for each ship, keep the frequency of each of its states
	for (int i = 0; i < n; ++i)
		state_frequency[i].resize(num_valid_states[i]);

	ll total_successful = 0;
	// call recursive ship placement routine to iterate through all valid placements
	ll total_states = place_ship(0, validity_masks, currently_valid, state_frequency, num_valid_states, total_successful);

	grid_t frequencies = create_grid();
	for (int i = 0; i < n; ++i)
		for (int state_index = 0; state_index < num_valid_states[i]; ++state_index)
			draw_state(state_index, lengths[i], state_frequency[i][state_index], frequencies);

	cout << "Total states: " << total_states << endl;
	cout << "Total successful (this should be the same number): " << total_successful << endl;
	print_grid(frequencies);
	print_grid_chance(frequencies,total_successful);
}

int main()
{
	ios::sync_with_stdio(0);
	cin.tie(NULL);

	// no misses for now, empty grid
	grid_t misses = create_grid();
	count_occurrences(misses);
}
