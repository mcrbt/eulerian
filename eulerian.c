/**
 * eulerian -- compute an Eulerian trail through a graph iff one exists
 * Copyright (C) 2016, 2019  Daniel Haase
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
 
#include <stdio.h>
#include <stdlib.h>

/* set GRAPH to != 0
 * to print list of IDs and degrees of all nodes
 * and adjacency lists of all nodes
 */
#define GRAPH 0

/* set MEMCHECK to != 0
 * to print some memory allocation statistics
 */
#define MEMCHECK 0

/* data structure representing one node as data type "et_node_t" */
typedef struct et_node
{
	int et_id;
	int et_deg;
	int et_edeg;
	int et_vst;
	struct et_elem *et_adj_lst;
	struct et_elem *et_adj_cur;
} et_node_t;

/* data structure for elements of linked lists as data type "et_elem_t" */
typedef struct et_elem
{
	int et_elem_usd;
	et_node_t *et_elem_node;
	struct et_elem *et_elem_nxt;
} et_elem_t;

/* data structure for whole graph as data type "et_graph_t" */
typedef struct et_graph
{
	int et_node_num;
	int et_edge_num;
	et_elem_t *et_node_lst;
	et_elem_t *et_node_cur;
	et_elem_t *et_odd_nodes;
} et_graph_t;

/* data structure representing an Eulerian trial as data type "et_trail_t" */
typedef struct et_trail
{
	int et_trail_num;
	et_elem_t *et_trail_lst;
	et_elem_t *et_trail_cur;
} et_trail_t;

/* data structure to count malloc()/ free() calls (for debugging purpose) */
typedef struct mc_memcheck
{
	int mc_malloc_node;
	int mc_malloc_elem;
	int mc_malloc_trail;
	int mc_free_node;
	int mc_free_elem;
	int mc_free_trail;
	int mc_malloc_sum;
	int mc_free_sum;
	int mc_glbl_sum;
} mc_memcheck_t;

static et_graph_t graph;
static mc_memcheck_t mc;
static int initialized = 0, circuit = 0, trail_nodes = 0;

/* print list of all nodes of the graph (for debugging purpose) */
static void print_node_lst(void)
{
	et_elem_t *cur = graph.et_node_lst;

	if(!GRAPH) return;
	printf("Node list: ");

	while(cur)
	{
		printf("%d (%d). ", cur->et_elem_node->et_id, cur->et_elem_node->et_deg);
		cur = cur->et_elem_nxt;
	}

	printf("\r\n\r\n");
}

/* print adjacency list for all nodes (for debugging purpose) */
static void print_node_adj(void)
{
	et_elem_t *cur_adj, *cur = graph.et_node_lst;

	if(!GRAPH) return;
	printf("Adjacency lists:\r\n");

	while(cur)
	{
		printf("%d : ", cur->et_elem_node->et_id);
		cur_adj = cur->et_elem_node->et_adj_lst;

		while(cur_adj)
		{
			printf("%d -> ", cur_adj->et_elem_node->et_id);
			cur_adj = cur_adj->et_elem_nxt;
		}

		printf("\r\n");
		cur = cur->et_elem_nxt;
	}

	printf("\r\n");
}

/* print trail "trail" as sequence of node IDs */
static void et_print_trail(et_trail_t *trail)
{
	et_elem_t *cur;

	if(trail) cur = trail->et_trail_lst;
	else return;

	while(cur)
	{
		printf("%d ", cur->et_elem_node->et_id);
		cur = cur->et_elem_nxt;
	}

	/* here no carriage return as claimed */
	printf("\n");
}

/* initialize graph data structure */
static void et_init_graph(void)
{
	if(initialized) return;

	graph.et_node_num = 0;
	graph.et_edge_num = 0;
	graph.et_node_lst = NULL;
	graph.et_node_cur = NULL;
	graph.et_odd_nodes = NULL;
	initialized = 1;
}

/* free memory of all elements of a list "list" */
static void et_clean_list(et_elem_t *list)
{
	et_elem_t *tmp, *cur = list;

	while(cur)
	{
		tmp = cur;
		cur = cur->et_elem_nxt;
		free(tmp);
		if(MEMCHECK) mc.mc_free_elem++;
	}
}

/* free (all) allocated memory */
static void et_clean(et_trail_t *trail, et_trail_t *list)
{
	et_elem_t *cur, *tmp;

	if(trail)
	{
		et_clean_list(trail->et_trail_lst);
		free(trail);
		if(MEMCHECK) mc.mc_free_trail++;
	}

	if(list)
	{
		et_clean_list(list->et_trail_lst);
		free(list);
		if(MEMCHECK) mc.mc_free_trail++;
	}

	if(initialized) et_clean_list(graph.et_odd_nodes);
	cur = graph.et_node_lst;

	while(cur)
	{
		if(cur->et_elem_node) et_clean_list(cur->et_elem_node->et_adj_lst);
		tmp = cur;
		cur = cur->et_elem_nxt;
		if(tmp->et_elem_node) { free(tmp->et_elem_node); if(MEMCHECK) mc.mc_free_node++; }
		free(tmp);
		if(MEMCHECK) mc.mc_free_elem++;
	}
}

/* return node of type et_node_t with id "id" from graph's node list */
static et_node_t *et_get_node(int id)
{
	et_elem_t *elem = graph.et_node_lst;

	while(elem)
	{
		if(elem->et_elem_node->et_id == id) return elem->et_elem_node;
		elem = elem->et_elem_nxt;
	}

	return NULL;
}

/* add element "elem" to graph */
static void et_add_node(et_elem_t *elem)
{
	et_elem_t *tmp;

	if(!graph.et_node_lst)
	{
		graph.et_node_lst = elem;
		graph.et_node_cur = elem;
	}
	else
	{
		tmp = graph.et_node_cur;
		tmp->et_elem_nxt = elem;
		graph.et_node_cur = elem;
	}

	++(graph.et_node_num);
}

/* add "elem2" to end of adjacency list of "elem1" */
static void et_add_adj(et_elem_t *elem1, et_elem_t *elem2)
{
	et_elem_t *tmp;

	if(!elem1 || !elem2) return;

	if(!(elem1->et_elem_node->et_adj_cur))
	{
		elem1->et_elem_node->et_adj_lst = elem2;
		elem1->et_elem_node->et_adj_cur = elem2;
	}
	else
	{
		tmp = elem1->et_elem_node->et_adj_cur;
		tmp->et_elem_nxt = elem2;
		elem1->et_elem_node->et_adj_cur = elem2;
	}

	++(elem1->et_elem_node->et_deg);
}

/* insert nodes with IDs "n1" and "n2" into graph and update adjacency lists */
static void et_add_edge(int n1, int n2)
{
	et_elem_t *elem_grh1, *elem_grh2, *elem_adj1, *elem_adj2;
	et_node_t *node1, *node2;

	node1 = et_get_node(n1);

	if(!node1)
	{
		if((node1 = (et_node_t *)malloc(sizeof(et_node_t))) == NULL)
		{
			fprintf(stdout, "Out of memory.\r\n");
			et_clean(NULL, NULL);
			exit(EXIT_FAILURE);
		}

		if(MEMCHECK) mc.mc_malloc_node++;

		if((elem_grh1 = (et_elem_t *)malloc(sizeof(et_node_t))) == NULL)
		{
			fprintf(stderr, "Out of memory.\r\n");
			et_clean(NULL, NULL);
			exit(EXIT_FAILURE);
		}

		if(MEMCHECK) mc.mc_malloc_elem++;

		node1->et_id = n1;
		node1->et_deg = node1->et_edeg = node1->et_vst = 0;
		node1->et_adj_lst = NULL;
		node1->et_adj_cur = NULL;
		elem_grh1->et_elem_node = node1;
		elem_grh1->et_elem_nxt = NULL;
		elem_grh1->et_elem_usd = 0;
		et_add_node(elem_grh1);
	}

	if(n1 == n2) node2 = node1;
	else node2 = et_get_node(n2);

	if(!node2)
	{
		if((node2 = (et_node_t *)malloc(sizeof(et_node_t))) == NULL)
		{
			fprintf(stdout, "Out of memory.\r\n");
			et_clean(NULL, NULL);
			exit(EXIT_FAILURE);
		}

		if(MEMCHECK) mc.mc_malloc_node++;

		if((elem_grh2 = (et_elem_t *)malloc(sizeof(et_node_t))) == NULL)
		{
			fprintf(stderr, "Out of memory.\r\n");
			et_clean(NULL, NULL);
			exit(EXIT_FAILURE);
		}

		if(MEMCHECK) mc.mc_malloc_elem++;

		node2->et_id = n2;
		node2->et_deg = node2->et_edeg = node2->et_vst = 0;
		node2->et_adj_lst = NULL;
		node2->et_adj_cur = NULL;
		elem_grh2->et_elem_node = node2;
		elem_grh2->et_elem_nxt = NULL;
		elem_grh2->et_elem_usd = 0;
		et_add_node(elem_grh2);
	}

	if((elem_adj1 = (et_elem_t *)malloc(sizeof(et_elem_t))) == NULL)
	{
		fprintf(stderr, "Out of memory.\r\n");
		et_clean(NULL, NULL);
		exit(EXIT_FAILURE);
	}

	if(MEMCHECK) mc.mc_malloc_elem++;

	elem_adj1->et_elem_node = node1;
	elem_adj1->et_elem_nxt = NULL;
	elem_adj1->et_elem_usd = 0;

	if((elem_adj2 = (et_elem_t *)malloc(sizeof(et_elem_t))) == NULL)
	{
		fprintf(stderr, "Out of memory.\r\n");
		et_clean(NULL, NULL);
		exit(EXIT_FAILURE);
	}

	if(MEMCHECK) mc.mc_malloc_elem++;

	elem_adj2->et_elem_node = node2;
	elem_adj2->et_elem_nxt = NULL;
	elem_adj2->et_elem_usd = 0;

	if(n1 == n2)
	{
		++(node1->et_deg);
		et_add_adj(elem_adj1, elem_adj2);
		free(elem_adj1);
		if(MEMCHECK) mc.mc_free_elem++;
	}
	else
	{
		et_add_adj(elem_adj1, elem_adj2);
		et_add_adj(elem_adj2, elem_adj1);
	}
}

/* parse data from input file and initialize data structures */
static void et_build_graph(char *filename)
{
	FILE *inp;
	int ret, n1, n2, specified_node_num, edge_num = 0;

	et_init_graph();

	if((inp = fopen(filename, "r")) == NULL)
	{
		fprintf(stderr, "Failed to open file \"%s\".\r\n", filename);
		exit(EXIT_FAILURE);
	}

	if(fscanf(inp, "%d", &ret) != 1)
	{
		fprintf(stderr, "Invalid input file format.\r\n");
		exit(EXIT_FAILURE);
	}

	specified_node_num = ret;

	while(1)
	{
		if((ret = fscanf(inp, "%d %d", &n1, &n2)) == EOF) break;
		else if(ret != 2)
		{
			fprintf(stderr, "Invalid input file format. (line %d)\r\n", edge_num + 2);
			et_clean(NULL, NULL);
			exit(EXIT_FAILURE);
		}
		else
		{
			et_add_edge(n1, n2);
			++edge_num;
		}
	}

	fclose(inp);
	graph.et_edge_num = edge_num;

	if(GRAPH)
	{
		if(specified_node_num != graph.et_node_num)
		{
			fprintf(stderr, "Warning, bad node number.\r\n");
			fprintf(stderr, "%d nodes found although %d nodes specified in first line of input file.\r\n", graph.et_node_num, specified_node_num);
			printf("\r\n");
		}

		print_node_lst();
		print_node_adj();
	}

	if(specified_node_num > graph.et_node_num)
	{
		fprintf(stderr, "This instance is not solvable.\r\n");
		printf("-1\n");
		et_clean(NULL, NULL);
		exit(EXIT_SUCCESS);
	}
}

/* some kind of "depth first search" */
static void et_dfs(et_elem_t *elem, int *cnt)
{
	et_elem_t *cur, *odd;

	if(!elem) return;

	if(elem->et_elem_node->et_vst) return;
	else
	{
		elem->et_elem_node->et_vst = 1;
		elem->et_elem_node->et_edeg = elem->et_elem_node->et_deg;
		trail_nodes += elem->et_elem_node->et_deg;

		if(elem->et_elem_node->et_deg % 2)
		{
			if((odd = (et_elem_t *)malloc(sizeof(et_elem_t))) == NULL)
			{
				fprintf(stderr, "Out of memory.\r\n");
				et_clean(NULL, NULL);
				exit(EXIT_FAILURE);
			}

			if(MEMCHECK) mc.mc_malloc_elem++;

			odd->et_elem_node = elem->et_elem_node;
			odd->et_elem_nxt = NULL;
			odd->et_elem_usd = 0;
			if(graph.et_odd_nodes) graph.et_odd_nodes->et_elem_nxt = odd;
			else graph.et_odd_nodes = odd;
		}

		++(*cnt);
	}

	cur = elem->et_elem_node->et_adj_lst;

	while(cur)
	{
		if(!(cur->et_elem_node->et_vst)) et_dfs(cur, cnt);
		cur = cur->et_elem_nxt;
	}
}

/* verify connectivity of the graph using "depth first search" */
static int et_is_connected(void)
{
	int cnt = 0;

	et_dfs(graph.et_node_lst, &cnt);
	trail_nodes = (trail_nodes / 2) + 1;
	if(cnt == graph.et_node_num) return 1;
	else return 0;
}

/* if 0 is returned there's no need to search an Eulerian trail since there isn't one */
static int et_validate_graph(void)
{
	et_elem_t *cur_elem = graph.et_node_lst;
	int num_deg_odd = 0;

	if(!cur_elem) return 0;

	while(cur_elem)
	{
		if((cur_elem->et_elem_node->et_deg) % 2) ++num_deg_odd;
		cur_elem = cur_elem->et_elem_nxt;
	}

	if(!num_deg_odd) circuit = 1;
	if(num_deg_odd == 1 || num_deg_odd > 2) return 0;
	else return et_is_connected();
}

/* add an element "node" to trail "trail" */
static void et_trail_add_elem(et_trail_t *trail, et_node_t *node)
{
	et_elem_t *elem;

	if(!node) return;

	if((elem = (et_elem_t *)malloc(sizeof(et_elem_t))) == NULL)
	{
		fprintf(stderr, "Out of memory.\r\n");
		et_clean(NULL, NULL);
		exit(EXIT_FAILURE);
	}

	if(MEMCHECK) mc.mc_malloc_elem++;

	elem->et_elem_node = node;
	elem->et_elem_nxt = NULL;
	elem->et_elem_usd = 0;

	if(!(trail->et_trail_lst)) trail->et_trail_lst = trail->et_trail_cur = elem;
	else
	{
		trail->et_trail_cur->et_elem_nxt = elem;
		trail->et_trail_cur = elem;
	}

	++(trail->et_trail_num);
}

/* mark an "edge" (between two nodes) as used */
static void et_set_edge_used(et_node_t *node1, et_node_t *node2)
{
	et_elem_t *elem;

	if(!node1 || !node2) return;
	--(node1->et_edeg);
	--(node2->et_edeg);
	elem = node1->et_adj_lst;

	while(elem)
	{
		if(elem->et_elem_node->et_id == node2->et_id)
		{
			elem->et_elem_usd = 1;
			break;
		}

		elem = elem->et_elem_nxt;
	}

	elem = node2->et_adj_lst;

	while(elem)
	{
		if(elem->et_elem_node->et_id == node1->et_id)
		{
			elem->et_elem_usd = 1;
			break;
		}

		elem = elem->et_elem_nxt;
	}
}

/* add element "node" to list of start nodes of next sub circuits */
static void et_list_add_elem(et_trail_t *list, et_node_t *node)
{
	et_elem_t *cur;
	int fnd = 0;

	if(node->et_edeg > 1)
	{
		cur = list->et_trail_lst;

		while(cur)
		{
			if(cur->et_elem_node->et_id == node->et_id) { fnd = 1; break; }
			cur = cur->et_elem_nxt;
		}

		if(!fnd) et_trail_add_elem(list, node);
	}
	else return;
}

/*
 * compute sub circuit
 * if function is called the first time
 * 		and graph contains two nodes of odd degree
 * 		precompute trail from one of them ("start") to the other one ("end")
 * else compute sub circuit (start == end)
 */
static et_trail_t *et_sub_circuit(et_node_t *start, et_node_t *end, et_trail_t **list)
{
	et_trail_t *sub;
	et_elem_t *cur;
	et_node_t *prv;

	if(!start) return NULL;
	if(!end) end = start;

	if((sub = (et_trail_t *)malloc(sizeof(et_trail_t))) == NULL)
	{
		fprintf(stderr, "Out of memory.\r\n");
		et_clean(NULL, *list);
		exit(EXIT_FAILURE);
	}

	if(MEMCHECK) mc.mc_malloc_trail++;

	sub->et_trail_lst = sub->et_trail_cur = NULL;
	sub->et_trail_num = 0;
	et_trail_add_elem(sub, start);
	if(start->et_edeg > 2) et_list_add_elem(*list, start);

	cur = start->et_adj_lst;
	prv = start;

	while(cur)
	{
		if(cur->et_elem_node->et_edeg > 0 && !(cur->et_elem_usd))
		{
			et_trail_add_elem(sub, cur->et_elem_node);
			et_set_edge_used(prv, cur->et_elem_node);
			et_list_add_elem(*list, cur->et_elem_node);
			if(cur->et_elem_node->et_id == end->et_id) return sub;
			else
			{
				prv = cur->et_elem_node;
				cur = cur->et_elem_node->et_adj_lst;
			}
		}
		else cur = cur->et_elem_nxt;
	}

	et_clean_list(sub->et_trail_lst);
	free(sub);
	if(MEMCHECK) mc.mc_free_trail++;
	return NULL;
}

/* insert sub circuit "sub" into trail "trail" */
static void et_insert_sub_circuit(et_trail_t *trail, et_trail_t *sub)
{
	et_elem_t *cur, *tmp;

	if(!sub) return;
	if(!trail) { trail = sub; return; }
	cur = trail->et_trail_lst;

	while(cur)
	{
		if(cur->et_elem_node->et_id == sub->et_trail_lst->et_elem_node->et_id) break;
		cur = cur->et_elem_nxt;
	}

	if(!cur) return;
	tmp = cur->et_elem_nxt;
	cur->et_elem_nxt = sub->et_trail_lst->et_elem_nxt;
	sub->et_trail_cur->et_elem_nxt = tmp;
	trail->et_trail_num += ((sub->et_trail_num) - 1);
	free(sub->et_trail_lst);
	free(sub);
	if(MEMCHECK) { mc.mc_free_elem++; mc.mc_free_trail++; }
}

/* verify if computed sub circuit is already an Eulerian trail of the whole graph */
static int et_is_trail(et_trail_t *trail)
{
	return (trail->et_trail_num >= trail_nodes);
}

/* compute Eulerian trail that solves the current instance */
static et_trail_t *et_eulerian_trail(et_trail_t **list)
{
	et_trail_t *trail;
	et_elem_t *cur;

	if((*list = (et_trail_t *)malloc(sizeof(et_trail_t))) == NULL)
	{
		fprintf(stderr, "Out of memory.\r\n");
		et_clean(NULL, NULL);
		exit(EXIT_FAILURE);
	}

	if(MEMCHECK) mc.mc_malloc_trail++;

	(*list)->et_trail_lst = (*list)->et_trail_cur = NULL;
	(*list)->et_trail_num = 0;

	if(circuit) trail = et_sub_circuit(graph.et_node_lst->et_elem_node, NULL, list);
	else trail = et_sub_circuit(graph.et_odd_nodes->et_elem_node, graph.et_odd_nodes->et_elem_nxt->et_elem_node, list);

	if(et_is_trail(trail)) return trail;
	cur = (*list)->et_trail_lst;

	while(cur)
	{
		if(et_is_trail(trail)) break;
		et_insert_sub_circuit(trail, et_sub_circuit(cur->et_elem_node, NULL, list));
		cur = cur->et_elem_nxt;
	}

	return trail;
}

static void mc_print_memory_info(void)
{
	if(!MEMCHECK) return;

	mc.mc_malloc_sum = mc.mc_malloc_node + mc.mc_malloc_elem + mc.mc_malloc_trail;
	mc.mc_free_sum = mc.mc_free_node + mc.mc_free_elem + mc.mc_free_trail;
	mc.mc_glbl_sum = mc.mc_malloc_sum - mc.mc_free_sum;

	printf("\r\nMemory allocation information:\r\n\r\n");
	printf("\tNodes allocated:       %d\r\n", mc.mc_malloc_node);
	printf("\tNodes freed:           %d\r\n\r\n", mc.mc_free_node);
	printf("\tElements allocated:    %d\r\n", mc.mc_malloc_elem);
	printf("\tElements freed:        %d\r\n\r\n", mc.mc_free_elem);
	printf("\tTrails allocated:      %d\r\n", mc.mc_malloc_trail);
	printf("\tTrails freed:          %d\r\n\r\n", mc.mc_free_trail);
	printf("\tCumulated allocations: %d\r\n", mc.mc_malloc_sum);
	printf("\tCumulated frees:       %d\r\n\r\n", mc.mc_free_sum);
	printf("\tSystem balance:        %d\r\n\r\n", mc.mc_glbl_sum);
}

int main(int argc, char **argv)
{
	et_trail_t *trail, *list;

	trail = list = NULL;
	mc.mc_malloc_node = 0;
	mc.mc_malloc_elem = 0;
	mc.mc_malloc_trail = 0;
	mc.mc_free_node = 0;
	mc.mc_free_elem = 0;
	mc.mc_free_trail = 0;
	mc.mc_malloc_sum = 0;
	mc.mc_free_sum = 0;
	mc.mc_glbl_sum = 0;

	if(argc != 2)
	{
		printf("Usage: ./eulerian <filename>\r\n");
		return 0;
	}
	else et_build_graph(argv[1]);

	if(et_validate_graph())
	{
		trail = et_eulerian_trail(&list);
		et_print_trail(trail);
	}
	else { printf("-1\n"); fprintf(stderr, "This instance is not solvable.\r\n"); }

	et_clean(trail, list);
	mc_print_memory_info();
	return 0;
}
