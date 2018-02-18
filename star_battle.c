#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define OPTIONS_MAX 2
#define GROUP_TYPES_N 3
#define SYMBOL_EOL '\n'
#define SYMBOL_STAR '*'
#define SYMBOL_EMPTY '.'
#define SYMBOL_UNKNOWN '?'

typedef struct {
	int symbol;
	int stars_n;
	int candidates_n;
}
group_t;

typedef struct cell_s cell_t;

struct cell_s {
	group_t *region_group;
	group_t *row_group;
	group_t *column_group;
	int options_n;
	int options[OPTIONS_MAX];
	int symbol;
	cell_t *last;
	cell_t *next;
};

void set_cell(cell_t *, group_t *, group_t *, group_t *, int);
void chain_cell(cell_t *, cell_t *, cell_t *);
void star_battle(void);
int empty_cell_allowed(group_t *);
void test_dec_candidates(cell_t *);
void dec_candidates(cell_t *);
void test_inc_candidates(cell_t *);
void inc_candidates(cell_t *);
int is_candidate(cell_t *);

int regions_n, stars_n, groups_n, side, nodes_n, solutions_n;
group_t *groups;
cell_t *cells, *header;

int main(void) {
	int groups_idx, cells_n, region_groups_n, row, column;
	cell_t *cell;

	/* Read parameters */
	if (scanf("%d%d", &regions_n, &stars_n) != 2 || regions_n < 1 || stars_n < 1) {
		fprintf(stderr, "Invalid parameters\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	if (getchar() != SYMBOL_EOL) {
		fprintf(stderr, "Invalid separator\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}

	/* Allocate memory for groups and cells and initialize groups */
	groups_n = regions_n*GROUP_TYPES_N;
	groups = malloc(sizeof(group_t)*(size_t)groups_n);
	if (!groups) {
		fprintf(stderr, "Could not allocate memory for groups\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	for (groups_idx = 0; groups_idx < groups_n; groups_idx++) {
		groups[groups_idx].stars_n = 0;
	}
	side = regions_n+2;
	cells_n = side*side;
	cells = malloc(sizeof(cell_t)*(size_t)(cells_n+1));
	if (!cells) {
		fprintf(stderr, "Could not allocate memory for cells\n");
		fflush(stderr);
		free(groups);
		return EXIT_FAILURE;
	}

	/* Read grid data and initialize border/grid cells */
	region_groups_n = 0;
	header = cells+cells_n;
	header->options_n = OPTIONS_MAX+1;
	header->last = header;
	header->next = header;
	cell = cells;
	for (column = 0; column < side; column++) {
		set_cell(cell++, NULL, NULL, NULL, SYMBOL_EMPTY);
	}
	for (row = 0; row < regions_n; row++) {
		set_cell(cell++, NULL, NULL, NULL, SYMBOL_EMPTY);
		for (column = 0; column < regions_n; column++) {
			int symbol = getchar();
			if (!isalnum(symbol)) {
				fprintf(stderr, "Invalid symbol\n");
				fflush(stderr);
				free(cells);
				free(groups);
				return EXIT_FAILURE;
			}
			for (groups_idx = 0; groups_idx < region_groups_n && groups[groups_idx].symbol != symbol; groups_idx++);
			if (groups_idx == region_groups_n) {
				if (region_groups_n == regions_n) {
					fprintf(stderr, "Too many regions in grid\n");
					fflush(stderr);
					free(cells);
					free(groups);
					return EXIT_FAILURE;
				}
				groups[region_groups_n++].symbol = symbol;
			}
			set_cell(cell, groups+groups_idx, groups+regions_n+row, groups+regions_n*2+column, SYMBOL_UNKNOWN);
			chain_cell(cell++, header->last, header);
		}
		set_cell(cell++, NULL, NULL, NULL, SYMBOL_EMPTY);
		if (getchar() != SYMBOL_EOL) {
			fprintf(stderr, "Invalid separator\n");
			fflush(stderr);
			free(cells);
			free(groups);
			return EXIT_FAILURE;
		}
	}
	for (column = 0; column < side; column++) {
		set_cell(cell++, NULL, NULL, NULL, SYMBOL_EMPTY);
	}
	if (region_groups_n < regions_n) {
		fprintf(stderr, "Not enough regions in grid\n");
		fflush(stderr);
		free(cells);
		free(groups);
		return EXIT_FAILURE;
	}

	/* Solve the grid */
	nodes_n = 0;
	solutions_n = 0;
	for (groups_idx = 0; groups_idx < groups_n; groups_idx++) {
		groups[groups_idx].candidates_n = 0;
	}
	for (cell = header->next; cell != header; cell = cell->next) {
		test_inc_candidates(cell);
	}
	star_battle();
	printf("Nodes %d\nSolutions %d\n", nodes_n, solutions_n);
	fflush(stdout);

	/* Free data and exit */
	free(cells);
	free(groups);
	return EXIT_SUCCESS;
}

void set_cell(cell_t *cell, group_t *region_group, group_t *row_group, group_t *column_group, int symbol) {
	cell->region_group = region_group;
	cell->row_group = row_group;
	cell->column_group = column_group;
	cell->symbol = symbol;
}

void chain_cell(cell_t *cell, cell_t *last, cell_t *next) {
	cell->last = last;
	last->next = cell;
	cell->next = next;
	next->last = cell;
}

void star_battle(void) {
	int groups_idx;
	cell_t *cell;
	nodes_n++;

	/* Check if number of stars can still be reached for all groups */
	for (groups_idx = 0; groups_idx < groups_n && groups[groups_idx].stars_n+groups[groups_idx].candidates_n >= stars_n; groups_idx++);
	if (groups_idx < groups_n) {
		return;
	}

	/* If all cells have been chosen */
	if (header->next == header) {

		/* Solution found */
		solutions_n++;
		if (solutions_n == 1) {
			int row;
			for (row = 1; row <= regions_n; row++) {
				int column;
				for (column = 1; column <= regions_n; column++) {
					putchar(cells[row*side+column].symbol);
				}
				puts("");
			}
			fflush(stdout);
		}
	}
	else {
		int options_idx;
		cell_t *cell_min = header;

		/* Choose the cell that has the least options */
		for (cell = header->next; cell != header && cell_min->options_n > 1; cell = cell->next) {
			int cell_is_candidate = is_candidate(cell);
			cell->options_n = 0;
			if (cell->region_group->stars_n < stars_n && cell->row_group->stars_n < stars_n && cell->column_group->stars_n < stars_n && cell_is_candidate) {
				cell->options[cell->options_n++] = SYMBOL_STAR;
			}
			if (!cell_is_candidate || (empty_cell_allowed(cell->region_group) && empty_cell_allowed(cell->row_group) && empty_cell_allowed(cell->column_group))) {
				cell->options[cell->options_n++] = SYMBOL_EMPTY;
			}
			if (cell->options_n < cell_min->options_n) {
				cell_min = cell;
			}
		}
		if (cell_min->options_n == 0) {
			return;
		}
		cell_min->last->next = cell_min->next;
		cell_min->next->last = cell_min->last;
		for (options_idx = 0; options_idx < cell_min->options_n; options_idx++) {
			test_dec_candidates(cell_min);
			if (cell_min->options[options_idx] == SYMBOL_STAR) {
				test_dec_candidates(cell_min-1);
				test_dec_candidates(cell_min-side-1);
				test_dec_candidates(cell_min-side);
				test_dec_candidates(cell_min-side+1);
				test_dec_candidates(cell_min+1);
				test_dec_candidates(cell_min+side+1);
				test_dec_candidates(cell_min+side);
				test_dec_candidates(cell_min+side-1);
				cell_min->region_group->stars_n++;
				cell_min->row_group->stars_n++;
				cell_min->column_group->stars_n++;
			}
			cell_min->symbol = cell_min->options[options_idx];
			star_battle();
			cell_min->symbol = SYMBOL_UNKNOWN;
			if (cell_min->options[options_idx] == SYMBOL_STAR) {
				cell_min->column_group->stars_n--;
				cell_min->row_group->stars_n--;
				cell_min->region_group->stars_n--;
				test_inc_candidates(cell_min+side-1);
				test_inc_candidates(cell_min+side);
				test_inc_candidates(cell_min+side+1);
				test_inc_candidates(cell_min+1);
				test_inc_candidates(cell_min-side+1);
				test_inc_candidates(cell_min-side);
				test_inc_candidates(cell_min-side-1);
				test_inc_candidates(cell_min-1);
			}
			test_inc_candidates(cell_min);
		}
		cell_min->next->last = cell_min;
		cell_min->last->next = cell_min;
	}
}

int empty_cell_allowed(group_t *group) {
	return group->stars_n+group->candidates_n > stars_n;
}

void test_dec_candidates(cell_t *cell) {
	if (is_candidate(cell)) {
		dec_candidates(cell);
	}
}

void dec_candidates(cell_t *cell) {
	cell->region_group->candidates_n--;
	cell->row_group->candidates_n--;
	cell->column_group->candidates_n--;
}

void test_inc_candidates(cell_t *cell) {
	if (is_candidate(cell)) {
		inc_candidates(cell);
	}
}

void inc_candidates(cell_t *cell) {
	cell->region_group->candidates_n++;
	cell->row_group->candidates_n++;
	cell->column_group->candidates_n++;
}

int is_candidate(cell_t *cell) {
	return cell->symbol == SYMBOL_UNKNOWN && (cell-1)->symbol != SYMBOL_STAR && (cell-side-1)->symbol != SYMBOL_STAR && (cell-side)->symbol != SYMBOL_STAR && (cell-side+1)->symbol != SYMBOL_STAR && (cell+1)->symbol != SYMBOL_STAR && (cell+side+1)->symbol != SYMBOL_STAR && (cell+side)->symbol != SYMBOL_STAR && (cell+side-1)->symbol != SYMBOL_STAR;
}
