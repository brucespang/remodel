%{
#include <stdio.h>
#include "include/remodel.h"
#include "include/array.h"

parser_edges_t* _parsed_edges;

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

expr  : files TARROW files              { _parsed_edges = parser_edges_new($3, $1, NULL); $$ = _parsed_edges; }
      | files TARROW files TCOLON TSTR  { _parsed_edges = parser_edges_new($3, $1, $5); $$ = _parsed_edges; }
      ;

%%
