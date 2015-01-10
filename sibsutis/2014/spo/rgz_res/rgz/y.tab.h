/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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
     IF = 258,
     WHILE = 259,
     READ = 260,
     WRITE = 261,
     TVAR1 = 262,
     TVAR2 = 263,
     TVAR3 = 264,
     INTEGER = 265,
     DOUBLE = 266,
     STR = 267,
     ID = 268,
     IFX = 269,
     ELSE = 270,
     ASSIGN = 271,
     NEQ = 272
   };
#endif
/* Tokens.  */
#define IF 258
#define WHILE 259
#define READ 260
#define WRITE 261
#define TVAR1 262
#define TVAR2 263
#define TVAR3 264
#define INTEGER 265
#define DOUBLE 266
#define STR 267
#define ID 268
#define IFX 269
#define ELSE 270
#define ASSIGN 271
#define NEQ 272




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 22 "cw.y"

	struct node *nodet;
	int integer;
	double dbl;
	char *string;



/* Line 2068 of yacc.c  */
#line 93 "y.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


