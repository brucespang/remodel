#include <assert.h>
#include <stdlib.h>
#include "include/remodel.h"

int main() {
  remodel_graph_t* graph = remodel_load_file("test/test.remodel");

  assert(ht_count(graph->nodes) == 6);
  assert(remodel_graph_contains_node(graph, "c1"));
  assert(remodel_graph_contains_node(graph, "c2"));
  assert(remodel_graph_contains_node(graph, "foo.c"));
  assert(remodel_graph_contains_node(graph, "bar.c"));
  assert(remodel_graph_contains_node(graph, "remodel_parent0"));
  assert(remodel_graph_contains_node(graph, "remodel_child0"));

  assert(remodel_graph_get_node(graph, "c1")->num_parents == 1);
  assert(remodel_graph_get_node(graph, "c2")->num_parents == 1);
  assert(remodel_graph_get_node(graph, "foo.c")->num_parents == 1);
  assert(remodel_graph_get_node(graph, "bar.c")->num_parents == 0);

  array_t* roots = remodel_roots(graph);
  assert(roots->len == 1);
  remodel_node_t* root = array_get(roots, 0);
  assert(strcmp("bar.c", root->name) == 0);

  return 0;
}
