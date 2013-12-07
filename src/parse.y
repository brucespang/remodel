%{
#include <stdio.h>
#include "include/remodel.h"
#include "include/array.h"

parser_edges_t* edges;

extern int yylex();

static void yyerror(const char *s) {
  printf("ERROR: %s\n", s);
  exit(1);
}

%}

%union {
  char* str;
  int token;
  array_t* files;
  parser_edges_t* edges;
}

%error-verbose

%token <str> TFILENAME TSTR
%token <token> TARROW TCOMMA TCOLON TWHITESPACE

%type <edges> expr
%type <files> files

%start expr

%%

files : TFILENAME                       { $$ = array_new(); array_append($$, $1); }
      | TFILENAME TCOMMA files          { $$ = $3; array_append($$, $1); }
      ;

expr  : files TARROW files              { edges = parser_edges_new($1, $3, NULL); $$ = edges; }
      | files TARROW files TCOLON TSTR  { edges = parser_edges_new($1, $3, $5); $$ = edges; }
      ;

%%
