
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     PROGRAM = 258,
     WHEN = 259,
     OTHERWISE = 260,
     REPEAT = 261,
     TIMES = 262,
     WHILE = 263,
     DEFINE = 264,
     GIVE = 265,
     SAY = 266,
     SET = 267,
     TO = 268,
     NUM = 269,
     TEXT = 270,
     DECIMAL = 271,
     CALL = 272,
     AND = 273,
     OR = 274,
     NOT = 275,
     IS = 276,
     GREATER = 277,
     LESS = 278,
     THAN = 279,
     EQUALS = 280,
     END_WHEN = 281,
     END_REPEAT = 282,
     END_WHILE = 283,
     END_DEFINE = 284,
     END_PROGRAM = 285,
     IDENTIFIER = 286,
     STRING_LITERAL = 287,
     INT_LITERAL = 288,
     FLOAT_LITERAL = 289,
     NEWLINE = 290,
     PLUS = 291,
     MINUS = 292,
     STAR = 293,
     SLASH = 294,
     LPAREN = 295,
     RPAREN = 296,
     COMMA = 297,
     UMINUS = 298
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 891 "src/parser.y"

    int ival;
    double fval;
    char *sval;
    void *ptr;



/* Line 1676 of yacc.c  */
#line 104 "src/parser.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


