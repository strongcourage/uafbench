/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

#ifndef YY_SEX_REC_SEX_TAB_H_INCLUDED
# define YY_SEX_REC_SEX_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int sexdebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    REC_SEX_TOK_INT = 258,
    REC_SEX_TOK_REAL = 259,
    REC_SEX_TOK_STR = 260,
    REC_SEX_TOK_NAM = 261,
    REC_SEX_TOK_COLON = 262,
    REC_SEX_TOK_QM = 263,
    REC_SEX_TOK_IMPLIES = 264,
    REC_SEX_TOK_AND = 265,
    REC_SEX_TOK_OR = 266,
    REC_SEX_TOK_EQL = 267,
    REC_SEX_TOK_NEQ = 268,
    REC_SEX_TOK_LT = 269,
    REC_SEX_TOK_GT = 270,
    REC_SEX_TOK_LTE = 271,
    REC_SEX_TOK_GTE = 272,
    REC_SEX_TOK_SAMETIME = 273,
    REC_SEX_TOK_AFTER = 274,
    REC_SEX_TOK_BEFORE = 275,
    REC_SEX_TOK_SUB = 276,
    REC_SEX_TOK_ADD = 277,
    REC_SEX_TOK_MUL = 278,
    REC_SEX_TOK_DIV = 279,
    REC_SEX_TOK_MOD = 280,
    REC_SEX_TOK_MAT = 281,
    REC_SEX_TOK_AMP = 282,
    REC_SEX_TOK_NEG = 283,
    REC_SEX_TOK_MIN = 284,
    REC_SEX_TOK_NOT = 285,
    REC_SEX_TOK_BP = 286,
    REC_SEX_TOK_EP = 287,
    REC_SEX_TOK_ERR = 288,
    REC_SEX_TOK_SHARP = 289
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 92 "../../src/rec-sex-tab.y" /* yacc.c:1909  */

  rec_sex_ast_node_t node;
  rec_sex_ast_t ast;

#line 94 "rec-sex-tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int sexparse (rec_sex_parser_t sex_parser);

#endif /* !YY_SEX_REC_SEX_TAB_H_INCLUDED  */
