%{
#include <string.h>
#define YYSTYPE char *
#include "ccc.tab.h"
extern unsigned int lineno;
%}
%%
\/\/[^\n\r\f]*		return COMMENT;
\"[^"]*\"		yylval = strdup(yytext); return LITERAL;
[a-z][a-z0-9]*		yylval = strdup(yytext); return IDENTIFIER;
"::="			return DEFINEDAS;
"|"			return ALTERNATIVE;
"["			return LBRACKET;
"]"			return RBRACKET;
"+"			return ONEORMORE;
"?"			return IDENTORNULL;
"."			return ENDCLAUSE;
[\n\r\f]		lineno ++;
[ \t]+			/* ignore whitespace */
.			return INVALID;
%%
