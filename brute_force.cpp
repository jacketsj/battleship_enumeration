#include <bits/stdc++.h>
using namespace std;

#define int long long

//CONSTANTS
//board dimensions
#define WIDTH 10
#define HEIGHT 10
//boats
const vector<int> sizes = {2,3,3,4,5};
const int n = sizes.size();

#define grid_t vector<vector<int>>

//if we could do this with incremental computation, checks would be near-constant time (log(W*H))
void draw_state(int length, int no, grid_t &grid, int mult, int &below_zero, int &above_one)
{
	//we give a canonical ordering to the states
	//this can hopefully be modified for hits
	//TODO: STRING MATCHING ALGORITHMS! where we just count the matches-ish? or maybe use/extend suffis trees
	int length1 = (WIDTH - length)*HEIGHT;

	//TODO: this check should be done in get_counts
	//int length2 = WIDTH*(HEIGHT - length);
	//if (no >= length1+length2)
	//	pass_flag = true;

	bool horizontal_orientation = no < length1;
	no %= length1;
	int adjusted_width = WIDTH;
	if (horizontal_orientation)
		adjusted_width = WIDTH - length;
	int x = no % adjusted_width;
	int y = no / adjusted_width;
	int dx = 0, dy = 1;
	if (horizontal_orientation)
	{
		dx = 1;
		dy = 0;
	}
	for (int l = 0; l < length; ++l)
	{
		if (grid[x][y] < 0)
			--below_zero;
		if (grid[x][y] > 1)
			--above_one;
		grid[x][y] += mult;
		if (grid[x][y] < 0)
			++below_zero;
		if (grid[x][y] > 1)
			++above_one;
		x += dx; y += dy;
	}
}


grid_t create_grid()
{
	grid_t ret(WIDTH);
	for (int i = 0; i < WIDTH; ++i)
		ret[i].resize(HEIGHT);
	return ret;
}

void add_grid(grid_t &a, const grid_t &b)
{
	for (int i = 0; i < WIDTH; ++i)
		for (int j = 0; j < HEIGHT; ++j)
			a[i][j] += b[i][j];
}

//counts is the return value
//hits has entries=-1 for each hit
//misses has entries=1 for each miss
void get_counts(grid_t &counts, const grid_t &hits, const grid_t &misses)
{
	//if we can get a canonical order: for each permutation of sizes

	//do integer incrementation algorithm, where digit positions are ships, and the digits are ship positions
	//the number of digits for a ship of size k to iterate through are:
	//(WIDTH - k)*HEIGHT + WIDTH*(HEIGHT - k) = 2*WIDTH*HEIGHT - k*(HEIGHT + WIDTH)
	//i.e., for each orientation, iterate through positions
		//check for collisions, and if so skip
		//add each to counts
	vector<int> positions(n);
	vector<int> max_digits(n);
	int total = 1;
	//figure out the number of possible positions for each piece
	for (int p = 0; p < n; ++p)
	{
		max_digits[p] = (WIDTH - sizes[p])*HEIGHT + WIDTH*(HEIGHT - sizes[p]);
		total *= max_digits[p];
	}
	grid_t current_grid = create_grid();
	add_grid(current_grid, hits);
	add_grid(current_grid, misses);
	//minimum and maximum entries in grid
	int below_zero = 0;
	int above_one = 0;
	cout << "total=" << total << endl;
	for (int i = 0; i < total; ++i)
	{
		//increment the state
		for (int p = 0; p < n; ++p)
		{
			++positions[p];
			if (positions[p] >= max_digits[p])
				positions[p] = 0;
			else
				break;
		}
		//check if it is a valid state (check collisions, check hits, check misses)
		//create the state, and check as we go
		for (int p = 0; p < n; ++p)
		{
			draw_state(sizes[p], positions[p], current_grid, 1, below_zero, above_one);
		}
		//if it is a valid state, add the matrix to counts
		if (below_zero == 0 && above_one == 0)
		{
			add_grid(counts, current_grid);
		}
		//undraw current state
		for (int p = 0; p < n; ++p)
		{
			draw_state(sizes[p], positions[p], current_grid, -1, below_zero, above_one);
		}
	}
}

void print_grid(const grid_t &a)
{
	for (int i = 0; i < WIDTH; ++i)
	{
		for (int j = 0; j < HEIGHT; ++j)
			cout << a[i][j] << ' ';
		cout << '\n';
	}
}

#undef int
int main()
{
#define int long long
	grid_t counts = create_grid();
	//assume no hits or misses for now
	grid_t hits = create_grid();
	grid_t misses = create_grid();

	get_counts(counts, hits, misses);
	print_grid(counts);
}
