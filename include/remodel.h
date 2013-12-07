#ifndef REMODEL_H_
#define REMODEL_H_

#include <stdint.h>
#include <stdbool.h>

#include "include/ht.h"
#include "include/array.h"
#include "include/queue.h"

#define REMODEL_DIR ".remodel"

typedef struct {
  ht_t* nodes;
  queue_t* queue;
  uint64_t num_visited;
} remodel_graph_t;

typedef struct {
  const char* cmd;
  array_t* children;
  array_t* parents;
} parser_edges_t;

typedef struct {
  const char* name;

  // graph data
  ht_t* children;

  // state for parallel topological sort
  bool modified;
  uint32_t num_parents;
  // we would like this to be a bool, but libck doesn't support CAS on bools
  uint8_t visited;

  // state for tarjan's algorithm
  uint32_t index;
  uint32_t low_index;
} remodel_node_t;

typedef struct {
  remodel_node_t* from;
  remodel_node_t* to;
  const char* command;
  bool visited;
} remodel_edge_t;

typedef struct {
  bool debug;
  bool generate_graph;
} remodel_options_t;

static remodel_options_t options = {
  .debug = false,
  .generate_graph = false
};

extern parser_edges_t* edges;

remodel_graph_t* remodel_load_file(char* path);
array_t* remodel_roots(remodel_graph_t* graph);
void remodel_execute(remodel_graph_t* graph, uint32_t num_threads);
remodel_graph_t* remodel_graph_new();
void remodel_graph_add_edges(remodel_graph_t* graph, parser_edges_t* node);
parser_edges_t* parser_edges_new(array_t* children, array_t* parents, const char* command);
parser_edges_t* remodel_parse_line(const char* line);
bool remodel_graph_contains_node(remodel_graph_t* graph, const char* name);
remodel_node_t* remodel_graph_get_node(remodel_graph_t* graph, const char* name);

#endif  // REMODEL_H_
