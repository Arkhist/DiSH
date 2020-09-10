%{

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "parser.h"

%}

blanks [ \t]+
chars [^ \t\n;=\<\>\$]+

%option nounput noinput

%%

{blanks}    { 

}

\n    {
    return NEWLINE;
}

\;  {
    return SEPARATOR;
}

&& {
    return AND;
}

&/[^&]  {
    return AMP;
}

"\>\>"    {
    return DGREAT;
}

\<  {
    return LESS;
}

\>  {
    return GREAT;
}

\|  {
    return PIPE;
}

\=  {
    return EQUALS;
}


[0-9]+/[\<\>]   {
    yylval.iVal = atoi(yytext);
    return IO_NUMBER;
}

{chars} {
    yylval.strVal = strdup(yytext);
    return WORD;
}

<<EOF>> {
    return END;
}
%%

void yyerror (char const *s) {
    fprintf (stderr, "%s\n", s);
}