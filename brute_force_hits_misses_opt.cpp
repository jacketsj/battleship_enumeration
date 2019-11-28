#include <bits/stdc++.h>
using namespace std;

#define int long long

//CONSTANTS
//board dimensions
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

//#define grid_t vector<vector<int>>
#define grid_t int[][]

void draw_state(int length, bool horizontal, int loc, int mult, grid_t &grid)
{
}

void draw_state(int length, int no, grid_t &grid, int mult, int &below_zero, int &above_one)
{
	//we give a canonical ordering to the states
	//this can hopefully be modified for hits
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
		ret[i].resize(HEIGHT); return ret;
}

void add_grid(grid_t &a, const grid_t &b)
{
	for (int i = 0; i < WIDTH; ++i)
		for (int j = 0; j < HEIGHT; ++j)
			a[i][j] += b[i][j];
}

void multiply_grid(grid_t &a, const int &c)
{
	for (int i = 0; i < WIDTH; ++i)
		for (int j = 0; j < HEIGHT; ++j)
			a[i][j] *= c;
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
void get_counts(grid_t &counts, const grid_t &hits, const grid_t &misses, int &added_count)
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
		max_digits[p] = (WIDTH - sizes[p] + 1)*HEIGHT + WIDTH*(HEIGHT - sizes[p] + 1); //count of possible positions for each orientation
		total *= max_digits[p];
	}
	grid_t current_grid = create_grid();
	add_grid(current_grid, hits);
	add_grid(current_grid, misses);
	//minimum and maximum entries in grid
	int below_zero = 0;
	int above_one = 0;
	cout << "total=" << total << endl;

	auto check_valid = [&] ()
	{
		return (below_zero == 0 && above_one == 0);
	};

	added_count = 0; //the number of valid arrangements

	int iter = 0;
	int p, p0; //to be used in critical section
	bool fully_drawn; //to be used in critical section
	do
	{
		//keep track of how we're doing
		if (iter % 1000000 == 0)
		{
				cout << "iter=" << iter << ", current_position = "; print_position(positions,max_digits);
		}
		++iter;


		//NO ALLOC CODE SECTION

		//create the state, and check as we go
		p = 0;
		fully_drawn = true;
		for (; p < n; ++p)
		{
			draw_state(sizes[p], positions[p], current_grid, 1, below_zero, above_one);
			//check if it is a valid state (check collisions, check hits, check misses)
			if (!check_valid()) //we failed without all the ships on the board
			{
				p0 = p; //amount of things drawn
				//undraw current state
				for (; p >= 0; --p) //note that we only undraw the ships we drew
				{
					draw_state(sizes[p], positions[p], current_grid, -1, below_zero, above_one);
				}
				//skip all cases with the 'smaller order' ships in any position
				for (p = p0+1; p < n; ++p)
				{
					//cout << "skipping boat p=" << p << " configs from position=" << positions[p] << endl;
					positions[p] = max_digits[p]-1;
				}
				fully_drawn = false; //don't re-undraw
				break; //stop drawing new ships
			}
		}
		//if it is a valid state, add the matrix to counts
		if (fully_drawn)
		{
			//all ships are now fully drawn, so we have a valid state
			add_grid(counts, current_grid);
			//note that this is also includes hit and miss offsets, so we have to subtract those after
			++added_count; //will subtract -added_count*current_grid at the end
			//undraw current state
			for (p = 0; p < n; ++p)
			{
				draw_state(sizes[p], positions[p], current_grid, -1, below_zero, above_one);
			}
		}
		//increment the ship position states
		for (p = n-1; p >= 0; --p)
		{
			++positions[p];
			if (positions[p] >= max_digits[p])
				positions[p] = 0;
			else
				break;
		}
	}
	while (unfinished(positions));
	//subtract added_count*current_grid since we had an extra difference of it
	multiply_grid(current_grid,-added_count);
	add_grid(counts, current_grid);
	//how many iterations did that take overall?
	cout << "total iter=" << iter << endl;
}

void print_grid(const grid_t &a)
{
	for (int i = 0; i < WIDTH; ++i)
		for (int j = 0; j < HEIGHT; ++j)
			cout << a[i][j] << "\t\n"[j==HEIGHT-1];
}

void print_grid_chance(const grid_t &a, const int &added_count)
{
	double count = added_count;
	for (int i = 0; i < WIDTH; ++i)
		for (int j = 0; j < HEIGHT; ++j)
			cout << double(a[i][j])/count << "\t\n"[j==HEIGHT-1];
}

#undef int
int main()
{
#define int long long
	grid_t counts = create_grid();
	//assume no hits or misses for now
	grid_t hits = create_grid();
	grid_t misses = create_grid();

	int added_count;
	get_counts(counts, hits, misses, added_count);
	print_grid(counts);
	cout << "Total configurations: " << added_count << endl;
	cout << fixed << setprecision(2); //show 2 decimals for chance
	print_grid_chance(counts,added_count);
}
