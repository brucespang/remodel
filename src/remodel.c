#include <assert.h>
#include <ck_pr.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/remodel.h"
#include "include/file.h"
#include "src/scan.c"
#include "include/murmurhash.h"

remodel_options_t options = {
  .debug = false,
  .generate_graph = false
};

remodel_graph_t* remodel_graph_new() {
  remodel_graph_t* graph = calloc(1, sizeof(remodel_graph_t));
  graph->nodes = ht_new(4);
  graph->queue = queue_new(4);
  graph->num_visited = 0;
  return graph;
}

parser_edges_t* parser_edges_new(array_t* children, array_t* parents, const char* cmd) {
  parser_edges_t* edges = calloc(1, sizeof(parser_edges_t));
  edges->cmd = cmd;
  edges->children = children;
  edges->parents = parents;
  return edges;
}

remodel_node_t* remodel_node_new(const char* name) {
  remodel_node_t* node = calloc(1, sizeof(remodel_node_t));
  node->name = name;
  node->children = ht_new(4);
  node->modified = false;
  node->num_parents = 0;
  node->visited = 0;
  node->index = 0;
  node->low_index = 0;
  return node;
}

remodel_node_t* remodel_graph_get_node(remodel_graph_t* graph, const char* name) {
  return ht_get(graph->nodes, name, strlen(name));
}

bool remodel_graph_contains_node(remodel_graph_t* graph, const char* name) {
  return remodel_graph_get_node(graph, name) != NULL;
}

void remodel_graph_add_nodes(remodel_graph_t* graph, array_t* nodes) {
  for (uint32_t i = 0; i < nodes->len; i++) {
    char* name = array_get(nodes, i);
    if (ht_get(graph->nodes, name, strlen(name)) == NULL) {
      remodel_node_t* node = remodel_node_new(name);
      assert(ht_put(graph->nodes, name, strlen(name), node));
    }
  }
}

static void remodel_graph_add_edge(remodel_node_t* from, remodel_node_t* to, const char* cmd) {
  if (!ht_get(from->children, to, sizeof(remodel_node_t))) {
    remodel_edge_t* edge = calloc(1, sizeof(remodel_edge_t));
    edge->from = from;
    edge->to = to;
    edge->command = cmd;
    edge->to->num_parents++;

    ht_put(from->children, edge->to, sizeof(remodel_edge_t), edge);
  } else {
    fprintf(stderr, "[remodel] error: only one edge can exist from %s to %s.\n",
            from->name, to->name);
    exit(1);
  }
}

static const char* remodel_parent = "_remodel_parent";
static const char* remodel_child = "_remodel_child";

void remodel_graph_add_edges(remodel_graph_t* graph, parser_edges_t* edges) {
  // if we have a command that maps from m parents to n children, the intent is not
  // to execute the command m*n times, but execute it once when all parents have been executed
  // and atomically produce all children.
  // to do this, we replace multiple parents and multiple children with a node, so that
  // we will execute the command at most once.
  if (edges->cmd && (edges->children->len > 1 || edges->parents->len > 1)) {
    static uint32_t parent_node_id = 0;
    uint32_t parent_node_name_len = strlen(remodel_parent) + 10 + 1;
    char* parent_node_name = malloc(parent_node_name_len);
    snprintf(parent_node_name, parent_node_name_len, "%s%d", remodel_parent, parent_node_id);
    parent_node_id++;

    static uint32_t child_node_id = 0;
    uint32_t child_node_name_len = strlen(remodel_child) + 10 + 1;
    char* child_node_name = malloc(child_node_name_len);
    snprintf(child_node_name, parent_node_name_len, "%s%d", remodel_child, child_node_id);
    child_node_id++;

    array_t* parent_node = array_new();
    array_append(parent_node, parent_node_name);

    array_t* child_node = array_new();
    array_append(child_node, child_node_name);

    parser_edges_t* parent_edges = parser_edges_new(parent_node, edges->parents, NULL);
    parser_edges_t* child_edges = parser_edges_new(edges->children, child_node, NULL);
    parser_edges_t* cmd_edge = parser_edges_new(child_node, parent_node, edges->cmd);

    remodel_graph_add_edges(graph, parent_edges);
    remodel_graph_add_edges(graph, child_edges);
    remodel_graph_add_edges(graph, cmd_edge);
  } else {
    // add the children and parent nodes if they don't already exist
    remodel_graph_add_nodes(graph, edges->children);
    remodel_graph_add_nodes(graph, edges->parents);

    // add edges from the parents to the children
    for (uint32_t i = 0; i < edges->parents->len; i++) {
      char* parent_name = array_get(edges->parents, i);
      remodel_node_t* parent = ht_get(graph->nodes, parent_name, strlen(parent_name));
      assert(parent);

      for (uint32_t i = 0; i < edges->children->len; i++) {
        char* child_name = array_get(edges->children, i);
        remodel_node_t* child = ht_get(graph->nodes, child_name, strlen(child_name));
        assert(child);

        remodel_graph_add_edge(parent, child, edges->cmd);
      }
    }
  }
}

parser_edges_t* remodel_parse_line(const char* line) {
  YY_FLUSH_BUFFER;
  YY_BUFFER_STATE buf = yy_scan_string(line);
  yyparse();
  yy_delete_buffer(buf);
  return edges;
}

remodel_graph_t* remodel_load_file(char* path) {
  FILE* f = fopen(path, "r");
  if (f == NULL) {
    fprintf(stderr, "error: fopen: %s\n", strerror(errno));
    exit(1);
  }

  remodel_graph_t* graph = remodel_graph_new();

  char* line = NULL;
  size_t len;
  int res;
  while ((res = getline(&line, &len, f)) != -1) {
    if (strlen(line) > 1) {
      remodel_graph_add_edges(graph, remodel_parse_line(line));
    }

    free(line);
    line = NULL;
  }

  return graph;
}

array_t* remodel_roots(remodel_graph_t* graph) {
  array_t* roots = array_new();

  ht_entry_t* entry;
  ht_iterator_t iter = HT_ITERATOR_INITIALIZER;
  while (ht_next(graph->nodes, &iter, &entry)) {
    remodel_node_t* node = entry->value;
    if (node->num_parents == 0) {
      array_append(roots, node);
    }
  }

  return roots;
}

static bool is_internal_name(const char* name) {
  return strncmp(name, remodel_parent, strlen(remodel_parent)) == 0 ||
    strncmp(name, remodel_child, strlen(remodel_child)) == 0;
}

static void edge_execute(remodel_edge_t* edge) {
  if (options.debug) {
    fprintf(stderr, "[remodel] executing %s -> %s\n", edge->from->name, edge->to->name);
  }

  // if the parent of this edge been modified, we want to regenerate the child.
  // we only want to check nodes that we haven't inserted, but if the parents
  // of one of our parent nodes have been modified, we should update the child.
  edge->to->modified |= edge->from->modified;
  if (is_internal_name(edge->from->name) && options.debug) {
    fprintf(stderr, "[remodel] %s(%d) -> %s(%d)\n",
            edge->from->name, edge->from->modified,
            edge->to->name, edge->to->modified);
  } else if (file_changed(edge->from->name)) {
    if (options.debug) {
      fprintf(stderr, "[remodel] %s modified on disk\n", edge->from->name);
      fprintf(stderr, "[remodel] %s has modified parent (%s)\n",
              edge->to->name, edge->from->name);
    }
    edge->from->modified = true;
    // we want to update the child
    edge->to->modified = true;
  }

  // if the child of this edge has been modified, we want to regenerate it.
  if (!is_internal_name(edge->to->name) && file_changed(edge->to->name)) {
    if (options.debug) {
      fprintf(stderr, "[remodel] %s modified on disk\n", edge->to->name);
    }
    edge->to->modified = true;
  }

  // if either the child or parent has been modified, we want to regenerate the child.
  if (edge->to->modified) {
    if (options.debug) {
      fprintf(stderr, "[remodel] %s needs regeneration\n", edge->to->name);
    }

    if (edge->command) {
      fprintf(stderr, "%s\n", edge->command);
      int status;
      if ((status = system(edge->command)) != 0) {
        if (status < 0) {
          fprintf(stderr, "[remodel] error: system: %s\n", strerror(errno));
        } else {
          fprintf(stderr, "[remodel] command exited with non-zero status: %d\n", status);
          exit(1);
        }
      }
    }
  }
}

static void* topo_sort_worker(void* arg) {
  remodel_graph_t* graph = arg;

  remodel_edge_t* edge;
  while ((edge = queue_dequeue(graph->queue)) != NULL) {
    assert(!edge->visited);
    edge->visited = true;

    // we're guaranteed that there is exactly one edge for a child with a command to execute
    edge_execute(edge);

    remodel_node_t* child = edge->to;
    if (ck_pr_faa_32(&child->num_parents, -1) == 1) {
      if (ck_pr_fas_8(&child->visited, 1) == 0) {
        ck_pr_inc_64(&graph->num_visited);
        ht_entry_t* entry;
        ht_iterator_t iter = HT_ITERATOR_INITIALIZER;
        while (ht_next(child->children, &iter, &entry)) {
          remodel_edge_t* edge = entry->value;
          queue_enqueue(graph->queue, edge);
        }
      }
    }
  }

  return NULL;
}

void remodel_execute(remodel_graph_t* graph, uint32_t num_threads) {
  array_t* entry_nodes = remodel_roots(graph);

  graph->num_visited = entry_nodes->len;

  for (uint32_t i = 0; i < entry_nodes->len; i++) {
    remodel_node_t* root = array_get(entry_nodes, i);

    ht_entry_t* entry;
    ht_iterator_t iter = HT_ITERATOR_INITIALIZER;
    while (ht_next(root->children, &iter, &entry)) {
      remodel_edge_t* edge = entry->value;
      queue_enqueue(graph->queue, edge);
    }
  }

  if (queue_size(graph->queue) == 0) {
    return;
  }

  pthread_t threads[num_threads];
  for (uint32_t i = 0; i < num_threads; i++) {
    // TODO: handle errors better
    if (pthread_create(&threads[i], NULL, &topo_sort_worker, graph) < 0) {
      fprintf(stderr, "error: pthread_create: %s\n", strerror(errno));
      goto exit;
    }
  }

  while (graph->num_visited != ht_count(graph->nodes)) {
  }

  for (uint32_t i = 0; i < num_threads; i++) {
    if (pthread_cancel(threads[i]) < 0) {
      fprintf(stderr, "error: pthread_cancel: %s\n", strerror(errno));
    }
  }

  // TODO: it would be good to do a pthread_join here, but for some reason it hangs when
  // joining the second thread. I don't know why, but I suspect it's related to the
  // mutex/condition variable not being freed properly by the operating system, since I'm
  // seeing the threads wait for a mutex in pthread_exit.

exit:
  array_free(entry_nodes);
}
