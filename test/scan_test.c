#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/remodel.h"

void assert_child(parser_edges_t* edges, uint32_t idx, const char* name) {
  const char* obs = array_get(edges->children, idx);
  assert(strcmp(obs, name) == 0);
}

void assert_parent(parser_edges_t* edges, uint32_t idx, const char* name) {
  const char* obs = array_get(edges->parents, idx);
  assert(strcmp(obs, name) == 0);
}

int main() {
  remodel_init();

  parser_edges_t* edges = remodel_parse_line("DEFAULT <- main.c : \"test\"");
  assert(edges->children->len == 1);
  assert(edges->parents->len == 1);
  assert(strcmp(edges->cmd, "test") == 0);
  assert_child(edges, 0, "DEFAULT");
  assert_parent(edges, 0, "main.c");

  edges = remodel_parse_line("DEFAULT <- main.c");
  assert(edges->children->len == 1);
  assert(edges->parents->len == 1);
  assert(edges->cmd == NULL);

  edges = remodel_parse_line("DEFAULT, DEFAULT2 <- main.c : \"test\"");
  assert(edges->children->len == 2);
  assert(edges->parents->len == 1);
  assert(strcmp(edges->cmd, "test") == 0);
  assert_child(edges, 0, "DEFAULT2");
  assert_child(edges, 1, "DEFAULT");
  assert_parent(edges, 0, "main.c");

  edges = remodel_parse_line("DEFAULT <- main.c, main2.c : \"test\"");
  assert(edges->children->len == 1);
  assert(edges->parents->len == 2);
  assert(strcmp(edges->cmd, "test") == 0);
  assert_child(edges, 0, "DEFAULT");
  assert_parent(edges, 0, "main2.c");
  assert_parent(edges, 1, "main.c");

  edges = remodel_parse_line("DEFAULT, DEFAULT2 <- main.c, main2.c : \"test\"");
  assert(edges->children->len == 2);
  assert(edges->parents->len == 2);
  assert(strcmp(edges->cmd, "test") == 0);
  assert_child(edges, 0, "DEFAULT2");
  assert_child(edges, 1, "DEFAULT");
  assert_parent(edges, 0, "main2.c");
  assert_parent(edges, 1, "main.c");

  return 0;
}
