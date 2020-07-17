#if DEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <stdint.h>
#include "chaos/options.h"
#include "chaos/arena.h"
#include "chaos/spellenums.h"
#include "chaos/porting.h"

/* the current count of open and closed nodes */
static int s_open_cnt;
static int s_closed_cnt;
static int s_goal = 0;
static char *s_tmparena = 0;


struct node {
	unsigned char index;	/* arena index */
	unsigned char parent;	/* parent arena square (used to trace the final path) */
	unsigned long g;	/* real cost to get here */
	unsigned long h;	/* estimated cost to the end */
};

static struct node *s_open = 0;
static struct node *s_closed = 0;
static struct node *s_successors = 0;


/* estimate the distance from square n to the goal */
static int heuristic(int n)
{
	uint16_t distance;
	get_distance(s_goal, n, &distance);
	return distance;
}

static void open_node(const struct node *rhs)
{
	s_open[s_open_cnt] = *rhs;
	s_open_cnt++;
}

static void close_node(const struct node *rhs)
{
	s_closed[s_closed_cnt] = *rhs;
	s_closed_cnt++;
}

static int g_func(int index)
{
	/* fuzz factor */
	if (arena[0][index] != 0) {
		if (is_dead(index))
			return 0;
		/* it'll cost at least 5 to go through it */
		return 5;
	}
	return 0;
}

/* find all possible squares that can be moved to from parent */
static int generate_succesors(const struct node *parent)
{
	int count = 0;
	int i;
	int tmp = Options[OPT_OLD_BUGS];
	Options[OPT_OLD_BUGS] = 0;
	for (i = 0; i < 8; i++) {
		int lookat = apply_position_modifier(parent->index, i);
		if (lookat == 0) /* oob */
			continue;
		lookat--;
		if (lookat == parent->index)
			continue;
		int contents = arena[0][lookat];
		/* things that the creature cannot go through */
		if (contents == SPELL_WALL ||
		    contents == SPELL_MAGIC_FIRE ||
		    contents == SPELL_MAGIC_CASTLE ||
		    contents == SPELL_DARK_CITADEL ||
		    contents == SPELL_MAGIC_WOOD)
			continue;

		uint16_t distance;
		get_distance(parent->index, lookat, &distance);
		/* add to succesors, set cost */
		s_successors[count].index = lookat;
		s_successors[count].parent = parent->index;
		s_successors[count++].g = parent->g + distance + g_func(lookat);
	}
	Options[OPT_OLD_BUGS] = tmp;
	return count;
}

static int is_open(int idx)
{
	int i;
	for (i = 0; i < s_open_cnt; i++) {
		if (s_open[i].index == idx)
			return i;
	}
	return -1;
}

static int is_closed(int idx)
{
	int i;
	for (i = 0; i < s_closed_cnt; i++) {
		if (s_closed[i].index == idx)
			return i;
	}
	return -1;
}

static void remove_from(struct node * table, int *count, int id)
{
	struct node *dest = &table[id];
	int i;
	*count = (*count) - 1;
	for (i = id; i < *count; i++) {
		*dest = *(dest + 1);
		dest++;
	}
}

static void remove_node(int openid, int closedid)
{
	/* remove planet id from open and closed lists */
	if (openid != -1) {
		remove_from(s_open, &s_open_cnt, openid);
	}

	if (closedid != -1) {
		remove_from(s_closed, &s_closed_cnt, closedid);
	}
}

/* returns a copy of the best current solution that we have yet to explore */
static struct node get_lowest_f_open(void)
{
	unsigned int lowest_f = 0x7fffffff;
	int lowest_index = 0;
	int i;
	for (i = 0; i < s_open_cnt; i++) {
		if ((s_open[i].g + s_open[i].h) <= lowest_f) {
			lowest_f = (s_open[i].g + s_open[i].h);
			lowest_index = i;
		}
	}

	return s_open[lowest_index];
}

#if DEBUG
void dumparena(char *tmparena)
{
	int x, y;
	printf("+---------------+\n");
	for (y = 0; y < 10; y++) {
		printf("|");
		for (x = 0; x < 15; x++) {
			printf("%c", tmparena[x + y * 16]);
		}
		printf("|\n");
	}
	printf("+---------------+\n");
}

void dumpopenset(int from, int to)
{
	char tmparena[160];
	int i;
	for (i = 0; i < 160; i++) {
		tmparena[i] = ' ';
	}
	for (i = 0; i < s_open_cnt; i++) {
		tmparena[s_open[i].index] = 'o';
	}
	for (i = 0; i < s_closed_cnt; i++) {
		tmparena[s_closed[i].index] = 'c';
	}
	tmparena[from] = 'f';
	tmparena[to] = 'T';
	dumparena(tmparena);
}
#else

void dumparena(char *tmparena UNUSED)
{
}

void dumpopenset(int from UNUSED, int to UNUSED)
{
}
#endif

void init_nodes(void)
{
	if (s_open == 0) {
		s_open = malloc(sizeof(struct node) * 256);
		s_closed = malloc(sizeof(struct node) * 256);
		s_successors = malloc(sizeof(struct node) * 256);
	}
}

/* we want to go from square "from" to square "to" */
int path_find(int from, int to)
{
	init_nodes();
	s_goal = to;
	s_open_cnt = 0;
	s_closed_cnt = 0;
	if (from < 0 || from > ARENA_SIZE)
		return -1;
	if (to < 0 || to > ARENA_SIZE)
		return -1;

	/* place start point on the open list */
	s_successors[0].index = from;
	s_successors[0].parent = from;
	s_successors[0].g = 0;
	s_successors[0].h = heuristic(from);
	open_node(&s_successors[0]);

	while (s_open_cnt) {
		const struct node node_current = get_lowest_f_open();

		/* Add node_current to the CLOSED list */
		close_node(&node_current);
		remove_node(is_open(node_current.index), -1);
		/* if node_current is the same state as node_goal we have found the solution */
		if (node_current.index == to) {
			break;
		}
		/* Generate each state that can come after node_current */
		int i;
		int successors = generate_succesors(&node_current);
		for (i = 0; i < successors; i++) {
			/* Set the cost of node_successor to be the cost of node_current */
			/* plus the cost to get to node_successor from node_current */
			/* */
			/* this is already done in succesor generation */

			/* find node_successor on the OPEN list */
			int openid = is_open(s_successors[i].index);

			/*  if node_successor is on the OPEN list but the existing one is as */
			/*  good or better then discard this successor and continue */
			if (openid != -1 && (s_open[openid].g <= s_successors[i].g))
				continue;

			int closedid = is_closed(s_successors[i].index);
			/*  if node_successor is on the CLOSED list but the existing one is as */
			/*  good or better then discard this successor and continue */
			if (closedid != -1 && (s_closed[closedid].g <= s_successors[i].g))
				continue;

			/* else node_successor was not on the open or closed lists */
			/* or it was on the lists but this version is better */

			/* Remove occurences of node_successor from OPEN and CLOSED */
			remove_node(openid, closedid);

			/* Set the parent of node_successor to node_current */
			/* This is already done in generate_succesors */

			/* Set h to be the estimated distance to node_goal (Using the heuristic function) */
			s_successors[i].h = heuristic(s_successors[i].index);

			/* Add node_successor to the OPEN list */
			open_node(&s_successors[i]);
		}
		/* Sanity check - make sure we don't run out of nodes */
		if (s_open_cnt == 256 || s_closed_cnt == 256)
			return -1;
		dumpopenset(from, to);
	}

	int destination_index = is_closed(to);
	if (destination_index == -1) {
		/* no idea what to do */
		return -1;
	}
	if (s_closed[destination_index].g == 0) {
		return -1; /* destination reached */
	}
	if (s_tmparena == 0) {
		s_tmparena = malloc(ARENA_SIZE + 1);
	}
	unsigned int i;
	for (i = 0; i < ARENA_SIZE; i++) {
		s_tmparena[i] = ' ';
	}
	int result = -1;
	s_tmparena[from] = 'f';

	for (i = 0; i < 256; i++) {
		s_tmparena[s_closed[destination_index].index] = '~';
		int parent_index = is_closed(s_closed[destination_index].parent);
		if (parent_index == -1) {
			break;
		}
		if (s_closed[parent_index].g == 0) {
			/* parent is the start, so "destination_index" is the next best */
			result = s_closed[destination_index].index;
			s_tmparena[result] = '*';
			break;
		}
		destination_index = parent_index;
	}
	s_tmparena[to] = 'T';
	dumparena(s_tmparena);

	return result;
}
