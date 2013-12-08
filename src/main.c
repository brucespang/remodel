#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/stat.h>

#include "include/remodel.h"
#include "include/stack.h"
#include "include/array.h"

extern remodel_options_t options;

static inline uint32_t min(uint32_t x, uint32_t y) {
  if (x < y) {
    return x;
  } else {
    return y;
  }
}

#define DEFAULT_NUM_THREADS 8

static const char* version = "0.0.1";
static const char* usage = "Usage %s [-hv] [/path/to/remodel/file]\n";

// Tarjan's Algorithm from wikipedia
uint32_t strongly_connected(struct stack* stack, remodel_node_t* node, uint32_t* index) {
  node->index = *index;
  node->low_index = *index;
  (*index)++;
  stack_push(stack, node);

  ht_entry_t* entry;
  ht_iterator_t iter = HT_ITERATOR_INITIALIZER;
  while (ht_next(node->children, &iter, &entry)) {
    remodel_edge_t* edge = entry->value;
    assert(edge->from == node);

    if (edge->to->index == 0) {
      strongly_connected(stack, edge->to, index);
      node->low_index = min(node->low_index, edge->to->low_index);
    } else if(stack_contains(stack, edge->to)) {
      node->low_index = min(node->low_index, edge->to->index);
    }
  }

  if (node->low_index == node->index) {
    array_t* component = array_new();
    remodel_node_t* w;
    do {
      assert(!stack_is_empty(stack));
      w = stack_pop(stack);
      array_append(component, w);
    } while (w != node);

    if (component->len > 1) {
      fprintf(stderr, "error: dependency graph contains cycle:\n");
      fprintf(stderr, "  [");
      for (uint32_t i = 0; i < component->len; i++) {
        remodel_node_t* node = array_get(component, i);
        fprintf(stderr, "%s", node->name);
        if (i != component->len - 1) {
          fprintf(stderr, ", ");
        }
      }
      fprintf(stderr, "]\n");

      array_free(component);
      return true;
    } else {
      array_free(component);
      return false;
    }
  } else {
    return false;
  }
}

void validate_detect_cycles(remodel_graph_t* graph) {
  struct stack* stack = stack_new(ht_count(graph->nodes));
  uint32_t index = 1;
  bool has_cycle = false;

  ht_entry_t* entry;
  ht_iterator_t iter = HT_ITERATOR_INITIALIZER;
  while (ht_next(graph->nodes, &iter, &entry)) {
    remodel_node_t* node = entry->value;
    if (node->index == 0) {
      has_cycle |= strongly_connected(stack, node, &index);
    }
  }

  if (has_cycle) {
    fprintf(stderr, "dependency graph has a cycle, exiting...\n");
    exit(1);
  }

  stack_free(stack);
}

void validate_graph(remodel_graph_t* graph) {
  // a valid graph has no cycles. if it has no cycles, it's also guaranteed to
  // have an entry point (a node with no incoming edges).
  validate_detect_cycles(graph);
}

int main(int argc, char** argv) {
  char* remodel_file;

  uint32_t num_threads = DEFAULT_NUM_THREADS;

  char c;
  while ((c = getopt(argc, argv, "h:v:p:gd")) != -1) {
    switch (c) {
    case 'h':
      fprintf(stderr, usage, argv[0]);
      exit(0);
      break;
    case 'v':
      fprintf(stderr, "Remodel: %s\n", version);
      break;
    case 'p':
      num_threads = atoi(optarg);
      break;
    case 'd':
      fprintf(stderr, "[REMODEL] debug mode enabled\n");
      options.debug = true;
      break;
    case 'g':
      options.generate_graph = true;
      break;
    default:
      fprintf(stderr, usage, argv[0]);
      exit(1);
      break;
    }
  }

  if (optind < argc) {
    remodel_file = argv[optind];
    optind++;
  } else {
    remodel_file = "build.remodel";
  }

  remodel_graph_t* graph = remodel_load_file(remodel_file);
  validate_graph(graph);
  if (options.generate_graph) {
    printf("digraph g {\n");
    ht_entry_t* entry;
    ht_iterator_t iter = HT_ITERATOR_INITIALIZER;
    while (ht_next(graph->nodes, &iter, &entry)) {
      remodel_node_t* parent = entry->value;

      ht_entry_t* child_entry;
      ht_iterator_t child_iter = HT_ITERATOR_INITIALIZER;
      while (ht_next(parent->children, &child_iter, &child_entry)) {
        remodel_edge_t* edge = child_entry->value;
        printf("\t\"%s\" -> \"%s\"", parent->name, edge->to->name);
        if (edge->command) {
          printf(" [label=\"%s\"]", edge->command);
        }
        printf(";\n");
      }
    }
    printf("}\n");
  } else {
    remodel_execute(graph, num_threads);
  }

  return 0;
}
