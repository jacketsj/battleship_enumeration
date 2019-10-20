#include <bits/stdc++.h>
using namespace std;

#define int long long

//CONSTANTS
//board dimensions
//#define WIDTH 3
//#define HEIGHT 3
#define WIDTH 10
#define HEIGHT 10
//boats
const vector<int> sizes = {2,3,3,4,5};
//const vector<int> sizes = {2,2};
const int n = sizes.size();

#define grid_t vector<vector<int>>

//variables = ships * positions. Label: x[s][p] - sizes[s], positions[p]
//for each position p: for each pair of ships s1, s2: -x[s1][p] -x[s2][p]
//for each ship s: for each position p1,p2 in different rows/columns: -x[s][p1] -x[s][p2]
//                 for each position p: -x[s][
//
//variables = ships * positions(for that ship) * orientations(2)
//for each ship pair s1,s2: for each conflicting position pair p1,p2: -x[s1][p1] -x[s2][p2]
//for each ship s: for distinct positions p1, p2: -x[s][p1] -x[s][p2]
//for each ship s: x[s][p1] x[s][p2] ...

void draw_state(int length, int no, grid_t &grid, int mult, int &below_zero, int &above_one)
{
	//we give a canonical ordering to the states
	//this can hopefully be modified for hits
	//TODO: STRING MATCHING ALGORITHMS! where we just count the matches-ish? or maybe use/extend suffix trees
	int length1 = (WIDTH - length + 1)*HEIGHT;

	bool horizontal_orientation = no < length1;
	no %= length1;
	int adjusted_width = WIDTH;
	if (horizontal_orientation)
		adjusted_width = WIDTH - length + 1;
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

bool print_position(const vector<int> &positions, const vector<int> &max_digits)
{
	int n = positions.size();
	for (int i = 0; i < n; ++i)
		cout << positions[i] << '/' << max_digits[i] << (i == n-1 ? ' ' : ',');
	cout << '\n';
}

bool unfinished(const vector<int> &positions)
{
	for (int a : positions)
		if (a > 0)
			return true;
	return false;
}

//counts is the return value
//hits has entries=-1 for each hit
//misses has entries=1 for each miss
void sat_formulation(const grid_t &hits, const grid_t &misses)
{
	//the number of digits for a ship of size k to iterate through are:
	//variables = ships * positions(for that ship, including orientations)
	//for each ship pair s1,s2: for each conflicting position pair p1,p2: -x[s1][p1] -x[s2][p2]
	//for each ship s: for distinct positions p1, p2: -x[s][p1] -x[s][p2]
	//for each ship s: x[s][p1] x[s][p2] ...
	//TODO need to add more for hits and misses

	//number of positions for a ship of length k:
	//  (WIDTH - k)*HEIGHT + WIDTH*(HEIGHT - k) = 2*WIDTH*HEIGHT - k*(HEIGHT + WIDTH)
	vector<int> max_digits(n);
	vector<vector<int>> vars(n);
	//figure out the number of possible positions for each piece
	for (int s = 0; s < n; ++s)
	{
		max_digits[s] = (WIDTH - sizes[s] + 1)*HEIGHT + WIDTH*(HEIGHT - sizes[s] + 1); //count of possible positions for each orientation
		vars[s].resize(max_digits[s]);
	}
	//compute total number of variables
	int var_size = 0;
	for (int s = 0; s < n; ++s)
		for (int p = 0; p < vars[s].size(); ++p)
			vars[s][p] = ++var_size;

	grid_t current_grid = create_grid();
	add_grid(current_grid, hits);
	add_grid(current_grid, misses);
	//minimum and maximum entries in grid
	int below_zero = 0;
	int above_one = 0;

	auto check_valid = [&] ()
	{
		return (below_zero == 0 && above_one == 0);
	};

	vector<vector<int>> clauses;
	auto add_clause = [&] (vector<int> &literals) //note that a 0 is appended
	{
		literals.push_back(0);
		clauses.push_back(literals);
	};
	//for each ship pair s1,s2: for each conflicting position pair p1,p2: -x[s1][p1] -x[s2][p2]
	for (int s1 = 0; s1 < n; ++s1)
	{
		for (int p1 = 0; p1 < max_digits[s1]; ++p1)
		{
			draw_state(sizes[s1], p1, current_grid, 1, below_zero, above_one);
			for (int s2 = s1+1; s2 < n; ++s2)
			{
				for (int p2 = 0; p2 < max_digits[s2]; ++p2)
				{
					draw_state(sizes[s2], p2, current_grid, 1, below_zero, above_one);
					if (!check_valid()) //these two ships conflict
					{
						vector<int> literals = {-vars[s1][p1],-vars[s2][p2]};
						add_clause(literals);
					}
					draw_state(sizes[s2], p2, current_grid, -1, below_zero, above_one);
				}
			}
			draw_state(sizes[s1], p1, current_grid, -1, below_zero, above_one);
		}
	}
	//for each ship s: for distinct positions p1, p2: -x[s][p1] -x[s][p2]
	for (int s = 0; s < n; ++s)
	{
		for (int p1 = 0; p1 < max_digits[s]; ++p1)
		{
			for (int p2 = p1+1; p2 < max_digits[s]; ++p2)
			{
				vector<int> literals = {-vars[s][p1], -vars[s][p2]};
				add_clause(literals);
			}
		}
	}
	//for each ship s: x[s][p1] x[s][p2] ...
	for (int s = 0; s < n; ++s)
	{
		vector<int> literals(max_digits[s]);
		for (int p = 0; p < max_digits[s]; ++p)
		{
			literals[p] = vars[s][p];
		}
		add_clause(literals);
	}

	//now output
	cout << "p cnf " << var_size << clauses.size() << '\n';
	for (vector<int> &literals : clauses)
	{
		for (int l : literals)
			cout << l << ' ';
		cout << '\n';
	}
}

#undef int
int main()
{
#define int long long
	//assume no hits or misses for now
	grid_t hits = create_grid();
	grid_t misses = create_grid();

	sat_formulation(hits, misses);
}
