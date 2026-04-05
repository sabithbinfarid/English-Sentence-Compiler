%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

int line_num = 1;

static void lex_error(const char *tok) {
    fprintf(stderr, "[Lexer Error] Line %d: Unknown token '%s'\n", line_num, tok);
    exit(1);
}
%}

%option noyywrap
%option nodefault

DIGIT      [0-9]
ID_START   [A-Za-z_]
ID_CHAR    [A-Za-z0-9_]
INT        {DIGIT}+
FLOAT      {DIGIT}+\.{DIGIT}+
STRING     \"([^\"\\]|\\.)*\"

%%

[ \t\r\f\v]+                    ;

"#"[^\n]*                        ;
"//"[^\n]*                       ;

[\r\n;]+                         {
                                    int i;
                                    for (i = 0; yytext[i] != '\0'; ++i) {
                                        if (yytext[i] == '\n') {
                                            line_num++;
                                        }
                                    }
                                    return NEWLINE;
                                  }

"end"[ \t]+"when"               { return END_WHEN; }
"end"[ \t]+"repeat"             { return END_REPEAT; }
"end"[ \t]+"while"              { return END_WHILE; }
"end"[ \t]+"define"             { return END_DEFINE; }
"end"[ \t]+"program"            { return END_PROGRAM; }

"program"                         { return PROGRAM; }
"when"                            { return WHEN; }
"otherwise"                       { return OTHERWISE; }
"repeat"                          { return REPEAT; }
"times"                           { return TIMES; }
"while"                           { return WHILE; }
"define"                          { return DEFINE; }
"give"                            { return GIVE; }
"say"                             { return SAY; }
"set"                             { return SET; }
"to"                              { return TO; }
"num"                             { return NUM; }
"text"                            { return TEXT; }
"decimal"                         { return DECIMAL; }
"call"                            { return CALL; }
"and"                             { return AND; }
"or"                              { return OR; }
"not"                             { return NOT; }
"is"                              { return IS; }
"greater"                         { return GREATER; }
"less"                            { return LESS; }
"than"                            { return THAN; }
"equals"                          { return EQUALS; }

"+"                               { return PLUS; }
"-"                               { return MINUS; }
"*"                               { return STAR; }
"/"                               { return SLASH; }
"("                               { return LPAREN; }
")"                               { return RPAREN; }
","                               { return COMMA; }

{FLOAT}                            { yylval.fval = atof(yytext); return FLOAT_LITERAL; }
{INT}                              { yylval.ival = atoi(yytext); return INT_LITERAL; }
{STRING}                           { yylval.sval = strdup(yytext); return STRING_LITERAL; }
{ID_START}{ID_CHAR}*               { yylval.sval = strdup(yytext); return IDENTIFIER; }

.                                  { lex_error(yytext); }

%%
