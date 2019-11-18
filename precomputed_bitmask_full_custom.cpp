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
const vector<int> lengths = {5,4,3,3,2};
const int n = lengths.size();


typedef unsigned long long ull;
//typedef uint64_t ull;

namespace fast_bitset
{
	template <ll T>
	struct bitset
	{
	#define sz ll(sizeof(ull)*8)
	#define N (T+sz-1)/sz //ceiling of T/sz
		ull vals[N];
		bitset()
		{
			for (int i = 0; i < N; ++i)
				vals[i] = 0;
		}
		void operator &=(const bitset &other)
		{
			for (int i = 0; i < N; ++i)
				vals[i] &= other.vals[i];
		}
		// any bit is set
		bool any() const
		{
			for (int i = 0; i < N; ++i)
				if (vals[i] != 0)
					return true;
			return false;
		}
		bool get(ll i) const
		{
			int k = i/sz;
			i %= sz;
			return (vals[k] & (ll(1)<<i)) != 0;
		}
		void set(ll i)
		{
			int k = i/sz;
			i %= sz;
			vals[k] |= (ll(1)<<i);
		}
		void reset(ll i)
		{
			int k = i/sz;
			i %= sz;
			vals[k] &= (~(ll(1)<<i));
		}
		void assign(int i, bool b)
		{
			if (b)
				set(i);
			else
				reset(i);
		}
		int bitscan_destructive_any()
		{
			for (int i = 0; i < N; ++i)
			{
				if (vals[i] != 0)
				{
					ull t = vals[i] & -vals[i];
					int j = __builtin_ctzll(vals[i]);
					vals[i] ^= t;
					return sz*i+j;
				}
			}
			return -1;
		}
		// do a destructive bitscan
		// returns -1 if nothing found
		int bitscan_destructive()
		{
			for (int i = 0; i < N; ++i)
			{
				if (vals[i] != 0)
				{
					// compute the index
					//int ret = sz*i+(__builtin_ffsll(vals[i])-1);
					// __builtin_ctzll counts trailing 0s
					ull t = vals[i] & -vals[i]; //lowestOneBit
					int j = __builtin_ctzll(vals[i]);
					int ret = sz*i+j;
					// destroy it
					//vals[i] &= ~(1<<j);
					//the fenwick tree code doesn't work (?)
					vals[i] ^= t;
					return ret;
				}
			}
			cout << "this should never happen" << endl;
			return -1;
		}
		int ctz(ll v) const
		{
			for (int i = 0; i < sz; ++i)
				if ((v & (ll(1)<<i)) !=0 )
					return i;
		}
		// non-destructive bitscan
		int bitscan()
		{
			for (int i = 0; i < N; ++i)
				if (vals[i] != 0)
					//return sz*i+ctz(vals[i]);
					return sz*i+__builtin_ctzll(vals[i]);
		}
	#undef sz
	#undef N
	};
}

typedef fast_bitset::bitset<WIDTH*(HEIGHT-1)+HEIGHT*(WIDTH-1)> pos_set;

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

// non-editing (and general) iteration through state_squares
template <typename F>
void look_at_state_squares(int state_index, int length, const grid_t &grid,
		F callback)
{
	int x, y, dx, dy;
	tie(x,y,dx,dy) = get_state_dpos(state_index, length);
	for (int l = 0; l < length; ++l)
	{
		callback(grid[x+y*WIDTH]);
		x += dx;
		y += dy;
	}
}

// returns a list of state indeces that do not conflict with misses
vector<int> find_valid_individual_states(int length, const grid_t &misses, const grid_t &hits, const grid_t &sinks,
		vector<int> &miss_count,
		vector<int> &hit_count,
		vector<int> &sink_count,
		int &total_misses,
		int &total_hits,
		int &total_sinks)
{
	vector<int> res; // final result
	// check validity of each state index
	int m = state_count(length);
	for (int state_index = 0; state_index < m; ++state_index)
	{
		// record counts
		look_at_state_squares(state_index, length, misses,
				[&](ll square)
				{
					if (square != 0)
						++miss_count[state_index];
				});
		look_at_state_squares(state_index, length, hits,
				[&](ll square)
				{
					if (square != 0)
						++hit_count[state_index];
				});
		look_at_state_squares(state_index, length, sinks,
				[&](ll square)
				{
					if (square != 0)
						++sink_count[state_index];
				});
		// find validity of state
		bool valid = true;
		// does it overlap with a miss?
		if (miss_count[state_index] > 0)
			valid = false;
		// does it overlap with a sink of the incorrect length?
		look_at_state_squares(state_index, length, sinks,
				[&](ll sink)
				{
					if (sink > 0)
						valid = valid && (sink == length);
				});
		// does it overlap with at most one sink?
		if (sink_count[state_index] > 1)
			valid = false;
		// does it overlap with a sink of the correct length, but not enough hits?
		if (sink_count[state_index] > 0 && hit_count[state_index] < length - 1)
			valid = false;
		// does it overlap with length hits (no sink, but is sunk)?
		if (hit_count[state_index] == length)
			valid = false;
		// is it still a valid state?
		res.push_back(valid);
	}
	return res;
}

// returns a list of position indeces that do not conflict with misses for each ship
vector<vector<int>> find_valid_states(const grid_t &misses,
		const grid_t &hits, const grid_t &sinks,
		vector<vector<int>> &miss_counts,
		vector<vector<int>> &hit_counts,
		vector<vector<int>> &sink_counts,
		int &total_misses,
		int &total_hits,
		int &total_sinks)
{
	vector<vector<int>> res(n);
	for (int i = 0; i < n; ++i)
		res[i] = find_valid_individual_states(lengths[i], misses, hits, sinks,
				miss_counts[i], hit_counts[i], sink_counts[i], total_misses, total_hits, total_sinks);
	return res;
}

bool compatible_pair(int length_a, int state_a, int length_b, int state_b, grid_t &test_grid)
{
	bool valid = draw_state(state_a,length_a,1,test_grid);
	valid = draw_state(state_b,length_b,1,test_grid) && valid;
	draw_state(state_a,length_a,-1,test_grid);
	draw_state(state_b,length_b,-1,test_grid);
	return valid;
}

pos_set all_compatible_pairs(int length_a, int state_a, int length_b, int state_count_b, grid_t &test_grid)
{
	pos_set res;
	for (int state_b = 0; state_b < state_count_b; ++state_b)
	{
		if (compatible_pair(length_a, state_a, length_b, state_b, test_grid))
			res.set(state_b);
	}
	return res;
}

vector<vector<vector<pos_set>>> find_all_pos_sets()
{
	// create a single test grid to avoid reallocating memory needlessly
	grid_t test_grid = create_grid();
	// indexed by ship1, position of ship1, ship2
	vector<vector<vector<pos_set>>> res(n);
	// for each ship i
	for (int i = 0; i < n; ++i)
	{
		int m = state_count(lengths[i]);
		res[i].resize(m);
		// for each state of ship i
		for (int state_i = 0; state_i < m; ++state_i) // for each state of ship i
			res[i][state_i].resize(n);
	}

	for (int i = 0; i < n; ++i) // for each ship
	{
		int m = state_count(lengths[i]);
		for (int state_i = 0; state_i < m; ++state_i) // for each state of ship i
			for (int j = i+1; j < n; ++j) // for each ship that will be seen later than i
			{
				int state_count_j = state_count(lengths[j]);
				res[i][state_i][j] = all_compatible_pairs(lengths[i], state_i,
						lengths[j], state_count_j, test_grid);
			}
	}
	return res;
}

ll place_ship(const int ship_index,
		const vector<vector<vector<pos_set>>> &validity_masks,
		vector<pos_set> &currently_valid,
		vector<vector<int>> &state_frequency,
		const vector<vector<int>> &hit_counts,
		const vector<vector<int>> &sink_counts,
		int remaining_hits,
		int remaining_sinks)
{
	// base case: all ships placed successfully
	if (ship_index == n)
	{
		// since this section of code is run so much, we should minimize branching (since that is very slow)
		// we could also do this as a memory lookup using remaining_sinks and remaining_hits as indeces
		// is the configuration valid? want to return 1 if so, 0 else
		// valid iff remaining_sinks = 0 and remaining_hits = 0
		int temp = remaining_sinks + remaining_hits; // 0 iff answer should be 1, >0 else
		//since the above is bounded by 2*(WIDTH*HEIGHT), we can ceiling the division
		int div = 2*WIDTH*HEIGHT;
		temp = (temp + div - 1) / div; // now 0 if valid, 1 else
		return 1-temp; // swap 1 and 0 responses
	}

	// Save the information about currently_valid that
	// we will need when we return from recursive calls
	vector<pos_set> edits(n-ship_index-1);
	for (int j = ship_index + 1; j < n; ++j)
		edits[j-ship_index-1] = currently_valid[j];

	// count of the number of valid placements of all remaining ships
	ll count = 0;

	// iterate over all the ship states that are still valid
	pos_set current = currently_valid[ship_index];
	//current.copy(currently_valid[ship_index]);
	//while (current.any())
	int state_index;
	while ((state_index = current.bitscan_destructive_any()) != -1)
	{
		//int state_index = current.bitscan_destructive();
		// update legal states for remaining ships
		for (int j = ship_index + 1; j < n; ++j)
			currently_valid[j] &= validity_masks[ship_index][state_index][j];
		// recurse on remaining ships
		ll sub_result = place_ship(ship_index + 1, validity_masks, currently_valid,
				state_frequency, hit_counts, sink_counts, remaining_hits - hit_counts[ship_index][state_index],
				remaining_sinks - sink_counts[ship_index][state_index]);
		// record counts
		count += sub_result;
		state_frequency[ship_index][state_index] += sub_result;
		// set currently_valid values back
		for (int j = ship_index + 1; j < n; ++j)
			currently_valid[j] = edits[j-ship_index-1];
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

// count the number of occurrences of each spot on the grid
// do this by counting the frequency of each position for each ship
void count_occurrences(grid_t &misses, grid_t &hits, grid_t &sinks)
{
	// find all the validity masks (conflict-free pairs), even for invalid states
	vector<vector<vector<pos_set>>> validity_masks = find_all_pos_sets();

	// for each ship and state, how many misses/hits/sinks overlap with it?
	vector<vector<int>> miss_counts(n), hit_counts(n), sink_counts(n);
	for (int i = 0; i < n; ++i)
	{
		int m = state_count(lengths[i]);
		miss_counts[i].resize(m);
		hit_counts[i].resize(m);
		sink_counts[i].resize(m);
	}
	int total_misses = 0, total_hits = 0, total_sinks = 0;

	// find all the valid state indeces
	vector<vector<int>> valid_states = find_valid_states(misses,hits,sinks,
			miss_counts,hit_counts,sink_counts,total_misses,total_hits,total_sinks);

	vector<pos_set> currently_valid(n); // for each ship, keep track of which states are still valid
	for (int i = 0; i < n; ++i)
	{
		int m = state_count(lengths[i]);
		for (int state_i = 0; state_i < m; ++state_i)
			currently_valid[i].set(state_i);
	}

	vector<vector<int>> state_frequency(n); // for each ship, keep the frequency of each of its states
	for (int i = 0; i < n; ++i)
		state_frequency[i].resize(state_count(lengths[i]));

	// call recursive ship placement routine to iterate through all valid configurations
	ll total_states = place_ship(0, validity_masks, currently_valid, state_frequency,
			hit_counts, sink_counts, total_hits, total_sinks);

	grid_t frequencies = create_grid();
	for (int i = 0; i < n; ++i)
	{
		int m = state_count(lengths[i]);
		for (int state_index = 0; state_index < m; ++state_index)
		{
			//cout << "final frequency of state_index=" << state_index << " was res=" << state_frequency[i][state_index] << endl;
			draw_state(state_index, lengths[i], state_frequency[i][state_index], frequencies);
		}
	}

	cout << "Total states: " << total_states << endl;
	print_grid(frequencies);
	print_grid_chance(frequencies,total_states);
}

int main()
{
	ios::sync_with_stdio(0);
	cin.tie(NULL);

	// misses is a grid of 0s and 1s, where the point is 1 iff it is a miss
	grid_t misses = create_grid();
	// hits is a grid of 0s and 1s, where the point is 1 iff it is a hit
	grid_t hits = create_grid();
	// hits is a grid of 0s -1s, and positive integers, where the point is 0 iff it is not a sink,
	// -1 if it is a sink for unspecified length, and k>0 if it is a sink of length k
	grid_t sinks = create_grid();
	count_occurrences(misses,hits,sinks);
}
