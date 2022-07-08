#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#define OPTIONS_MAX 2
#define EMPTY_ALLOWED_SIZE 3
#define GROUP_TYPES_N 3
#define SYMBOL_EOL '\n'
#define SYMBOL_STAR '*'
#define SYMBOL_EMPTY '.'
#define SYMBOL_UNKNOWN '?'

typedef struct {
	int symbol;
	int stars_n;
	int candidates_max;
}
group_t;

typedef struct cell_s cell_t;

struct cell_s {
	group_t *region_group;
	group_t *row_group;
	group_t *column_group;
	int candidate;
	int options_n;
	int options[OPTIONS_MAX];
	int symbol;
	int empty_allowed[EMPTY_ALLOWED_SIZE];
	cell_t *last;
	cell_t *next;
};

void set_cell(cell_t *, group_t *, group_t *, group_t *, int);
void chain_cell(cell_t *, cell_t *, cell_t *);
void star_battle(int);
int sum_with_limit(int, int);
int get_empty_allowed(group_t *);
void lock_cell(cell_t *, int);
void test_dec_candidates(cell_t *);
void dec_candidates(cell_t *);
void set_empty_allowed(cell_t *);
int compare_ints(const void *, const void *);
int compare_empty_allowed(cell_t *, cell_t *);
void unlock_cell(cell_t *, int);
void test_inc_candidates(cell_t *);
void inc_candidates(cell_t *);
int is_candidate(cell_t *);

int regions_n, stars_n, side, nodes_n;
long solutions_max, solutions_n;
cell_t *cells, **locked_cells, *header;

int main(int argc, char *argv[]) {
	char *end;
	int groups_n, groups_idx, cells_n, region_groups_n, row, column;
	group_t *groups;
	cell_t *cell;
	if (argc > 2) {
		fprintf(stderr, "Usage: %s [<maximum number of solutions>]\n", argv[0]);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	if (argc == 1) {
		solutions_max = LONG_MAX;
	}
	else {
		solutions_max = strtol(argv[1], &end, 10);
		if (*end || solutions_max < 1) {
			fprintf(stderr, "Invalid maximum number of solutions\n");
			fflush(stderr);
			return EXIT_FAILURE;
		}
	}
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
	locked_cells = malloc(sizeof(cell_t *)*(size_t)cells_n);
	if (!locked_cells) {
		fprintf(stderr, "Could not allocate memory for locked_cells\n");
		fflush(stderr);
		free(cells);
		free(groups);
		return EXIT_FAILURE;
	}
	region_groups_n = 0;
	header = cells+cells_n;
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
				free(locked_cells);
				free(cells);
				free(groups);
				return EXIT_FAILURE;
			}
			for (groups_idx = 0; groups_idx < region_groups_n && groups[groups_idx].symbol != symbol; groups_idx++);
			if (groups_idx == region_groups_n) {
				if (region_groups_n == regions_n) {
					fprintf(stderr, "Too many regions in grid\n");
					fflush(stderr);
					free(locked_cells);
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
			free(locked_cells);
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
		free(locked_cells);
		free(cells);
		free(groups);
		return EXIT_FAILURE;
	}
	nodes_n = 0;
	solutions_n = 0;
	for (groups_idx = 0; groups_idx < groups_n; groups_idx++) {
		groups[groups_idx].candidates_max = 0;
	}
	for (cell = header->next; cell != header; cell = cell->next) {
		inc_candidates(cell);
	}
	star_battle(0);
	printf("Nodes %d\nSolutions %ld\n", nodes_n, solutions_n);
	fflush(stdout);
	free(locked_cells);
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

void star_battle(int locked_cells_sum) {
	int changes_n, options_min, locked_cells_n = 0;
	cell_t *cell;
	nodes_n = sum_with_limit(nodes_n, 1);
	if (solutions_n == solutions_max) {
		return;
	}
	do {
		changes_n = 0;
		options_min = OPTIONS_MAX;
		for (cell = header->next; cell != header && options_min > 0; cell = cell->next) {
			cell->options_n = 0;
			if (get_empty_allowed(cell->region_group) >= 0 && get_empty_allowed(cell->row_group) >= 0 && get_empty_allowed(cell->column_group) >= 0) {
				if (cell->region_group->stars_n < stars_n && cell->row_group->stars_n < stars_n && cell->column_group->stars_n < stars_n && cell->candidate) {
					cell->options[cell->options_n++] = SYMBOL_STAR;
				}
				if ((get_empty_allowed(cell->region_group) > 0 && get_empty_allowed(cell->row_group) > 0 && get_empty_allowed(cell->column_group) > 0) || !cell->candidate) {
					cell->options[cell->options_n++] = SYMBOL_EMPTY;
				}
				if (cell->options_n == 1) {
					changes_n++;
					lock_cell(cell, cell->options[0]);
					locked_cells[locked_cells_sum+locked_cells_n] = cell;
					locked_cells_n++;
				}
			}
			if (cell->options_n < options_min) {
				options_min = cell->options_n;
			}
		}
	}
	while (changes_n > 0 && options_min > 0 && header->next != header);
	if (options_min > 0) {
		if (header->next != header) {
			int options_idx;
			cell_t *cell_min;
			set_empty_allowed(header->next);
			cell_min = header->next;
			for (cell = cell_min->next; cell != header; cell = cell->next) {
				set_empty_allowed(cell);
				if (compare_empty_allowed(cell, cell_min) < 0) {
					cell_min = cell;
				}
			}
			for (options_idx = 0; options_idx < OPTIONS_MAX; options_idx++) {
				lock_cell(cell_min, cell_min->options[options_idx]);
				locked_cells[locked_cells_sum+locked_cells_n] = cell_min;
				star_battle(locked_cells_sum+locked_cells_n+1);
				unlock_cell(cell_min, cell_min->symbol);
			}
		}
		else {
			solutions_n++;
			if (solutions_n == 1) {
				int row;
				printf("Nodes %d\n", nodes_n);
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
	}
	while (locked_cells_n > 0) {
		locked_cells_n--;
		unlock_cell(locked_cells[locked_cells_sum+locked_cells_n], locked_cells[locked_cells_sum+locked_cells_n]->symbol);
	}
}

int sum_with_limit(int a, int b) {
	if (a <= INT_MAX-b) {
		return a+b;
	}
	return INT_MAX;
}

int get_empty_allowed(group_t *group) {
	return group->stars_n+group->candidates_max-stars_n;
}

void lock_cell(cell_t *cell, int symbol) {
	cell->last->next = cell->next;
	cell->next->last = cell->last;
	if (cell->candidate) {
		dec_candidates(cell);
	}
	if (symbol == SYMBOL_STAR) {
		test_dec_candidates(cell-1);
		test_dec_candidates(cell-side-1);
		test_dec_candidates(cell-side);
		test_dec_candidates(cell-side+1);
		test_dec_candidates(cell+1);
		test_dec_candidates(cell+side+1);
		test_dec_candidates(cell+side);
		test_dec_candidates(cell+side-1);
		cell->region_group->stars_n++;
		cell->row_group->stars_n++;
		cell->column_group->stars_n++;
	}
	cell->symbol = symbol;
}

void test_dec_candidates(cell_t *cell) {
	if (cell->symbol == SYMBOL_UNKNOWN && cell->candidate) {
		dec_candidates(cell);
	}
}

void dec_candidates(cell_t *cell) {
	cell->candidate = 0;
	cell->region_group->candidates_max--;
	cell->row_group->candidates_max--;
	cell->column_group->candidates_max--;
}

void set_empty_allowed(cell_t *cell) {
	cell->empty_allowed[0] = get_empty_allowed(cell->region_group);
	cell->empty_allowed[1] = get_empty_allowed(cell->row_group);
	cell->empty_allowed[2] = get_empty_allowed(cell->column_group);
	qsort(cell->empty_allowed, (size_t)EMPTY_ALLOWED_SIZE, sizeof(int), compare_ints);
}

int compare_ints(const void *a, const void *b) {
	const int *int_a = (const int *)a, *int_b = (const int *)b;
	return *int_a-*int_b;
}

int compare_empty_allowed(cell_t *cell_a, cell_t *cell_b) {
	if (cell_a->empty_allowed[0] != cell_b->empty_allowed[0]) {
		return cell_a->empty_allowed[0]-cell_b->empty_allowed[0];
	}
	if (cell_a->empty_allowed[1] != cell_b->empty_allowed[1]) {
		return cell_a->empty_allowed[1]-cell_b->empty_allowed[1];
	}
	return cell_a->empty_allowed[2]-cell_b->empty_allowed[2];
}

void unlock_cell(cell_t *cell, int symbol) {
	cell->symbol = SYMBOL_UNKNOWN;
	if (symbol == SYMBOL_STAR) {
		cell->column_group->stars_n--;
		cell->row_group->stars_n--;
		cell->region_group->stars_n--;
		test_inc_candidates(cell+side-1);
		test_inc_candidates(cell+side);
		test_inc_candidates(cell+side+1);
		test_inc_candidates(cell+1);
		test_inc_candidates(cell-side+1);
		test_inc_candidates(cell-side);
		test_inc_candidates(cell-side-1);
		test_inc_candidates(cell-1);
	}
	if (is_candidate(cell)) {
		inc_candidates(cell);
	}
	cell->next->last = cell;
	cell->last->next = cell;
}

void test_inc_candidates(cell_t *cell) {
	if (cell->symbol == SYMBOL_UNKNOWN && is_candidate(cell)) {
		inc_candidates(cell);
	}
}

void inc_candidates(cell_t *cell) {
	cell->region_group->candidates_max++;
	cell->row_group->candidates_max++;
	cell->column_group->candidates_max++;
	cell->candidate = 1;
}

int is_candidate(cell_t *cell) {
	return (cell-1)->symbol != SYMBOL_STAR && (cell-side-1)->symbol != SYMBOL_STAR && (cell-side)->symbol != SYMBOL_STAR && (cell-side+1)->symbol != SYMBOL_STAR && (cell+1)->symbol != SYMBOL_STAR && (cell+side+1)->symbol != SYMBOL_STAR && (cell+side)->symbol != SYMBOL_STAR && (cell+side-1)->symbol != SYMBOL_STAR;
}
