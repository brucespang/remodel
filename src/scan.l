%{
#include <stdio.h>
#include "include/remodel.h"
#include "include/array.h"
#include "y.tab.h"
%}

%option noyywrap

%%

[ \t\n]             ;
"<-"                { return TARROW; }
","                 { return TCOMMA; }
":"                 { return TCOLON; }
\"[^\"]*\"	        { yylval.str = strdup(yytext+1); yylval.str[strlen(yylval.str)-1] = '\0'; return TSTR; }
[A-Za-z0-9\.-_/]+   { yylval.str = strdup(yytext); return TFILENAME; }

%%
