/* -*- mode: C -*-
 *
 *       File:         rec-sex.y
 *       Date:         Sat Jan  9 16:36:55 2010
 *
 *       GNU recutils - Selection Expressions parser
 *
 */

/* Copyright (C) 2010-2019 Jose E. Marchesi */

/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

%pure-parser
%name-prefix "sex"
%parse-param {rec_sex_parser_t sex_parser}
%lex-param { void *scanner }

%{
  #include <config.h>

  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <regex.h>

  #include <rec-sex-ast.h>
  #include <rec-sex-parser.h>
  #include "rec-sex-tab.h"
  /*  #include "rec-sex-lex.h" */

  void sexerror (rec_sex_parser_t context, const char *err)
  {
    /* Do nothing.  */
  }

  #define scanner (rec_sex_parser_scanner (sex_parser))

  #define CREATE_NODE_OP1(TYPE,RES,OP)                  \
    do                                                  \
      {                                                 \
        /* Create the node.  */                         \
        (RES) = rec_sex_ast_node_new ();                \
        rec_sex_ast_node_set_type ((RES), (TYPE));      \
                                                        \
        /* Set children. */                             \
        rec_sex_ast_node_link ((RES), (OP));            \
      }                                                 \
    while (0)

#define CREATE_NODE_OP2(TYPE, RES, OP1, OP2)            \
      do                                                \
        {                                               \
          /* Create the node.  */                       \
          (RES) = rec_sex_ast_node_new ();              \
          rec_sex_ast_node_set_type ((RES), (TYPE));    \
                                                        \
          /* Set children. */                           \
          rec_sex_ast_node_link ((RES), (OP1));         \
          rec_sex_ast_node_link ((RES), (OP2));         \
        }                                               \
     while (0)

#define CREATE_NODE_OP3(TYPE, RES, OP1, OP2, OP3)       \
      do                                                \
        {                                               \
          /* Create the node.  */                       \
          (RES) = rec_sex_ast_node_new ();              \
          rec_sex_ast_node_set_type ((RES), (TYPE));    \
                                                        \
          /* Set children. */                           \
          rec_sex_ast_node_link ((RES), (OP1));         \
          rec_sex_ast_node_link ((RES), (OP2));         \
          rec_sex_ast_node_link ((RES), (OP3));         \
        }                                               \
     while (0)
  
%}

%union {
  rec_sex_ast_node_t node;
  rec_sex_ast_t ast;
};

/* Bison declarations.  */

%token <node> REC_SEX_TOK_INT
%token <node> REC_SEX_TOK_REAL
%token <node> REC_SEX_TOK_STR
%token <node> REC_SEX_TOK_NAM
%token <node> REC_SEX_TOK_COLON
%left <node> REC_SEX_TOK_QM
%right <node> REC_SEX_TOK_IMPLIES
%left <node> REC_SEX_TOK_AND REC_SEX_TOK_OR
%left <node> REC_SEX_TOK_EQL REC_SEX_TOK_NEQ REC_SEX_TOK_LT REC_SEX_TOK_GT REC_SEX_TOK_LTE REC_SEX_TOK_GTE
%left <node> REC_SEX_TOK_SAMETIME REC_SEX_TOK_AFTER REC_SEX_TOK_BEFORE
%left <node> REC_SEX_TOK_SUB REC_SEX_TOK_ADD
%left <node> REC_SEX_TOK_MUL REC_SEX_TOK_DIV REC_SEX_TOK_MOD REC_SEX_TOK_MAT
%left <node> REC_SEX_TOK_AMP
%left <node> REC_SEX_TOK_NEG  REC_SEX_TOK_MIN /* negation--unary minus */
%right <node> REC_SEX_TOK_NOT
%token <node> REC_SEX_TOK_BP REC_SEX_TOK_EP
%token <node> REC_SEX_TOK_ERR
%token <node> REC_SEX_TOK_SHARP

%type <ast> input
%type <node> exp

%% /* The grammar follows.  */

input: 
     exp
     {
       rec_sex_ast_t ast;

       ast = rec_sex_ast_new ();
       rec_sex_ast_set_top (ast, $1);
       rec_sex_parser_set_ast (sex_parser, ast);
     }
     ;

exp : REC_SEX_TOK_INT          { $$ = $1; }
    | REC_SEX_TOK_REAL         { $$ = $1; }
    | REC_SEX_TOK_STR          { $$ = $1; }
    | REC_SEX_TOK_NAM          { $$ = $1; }
    | exp REC_SEX_TOK_QM exp REC_SEX_TOK_COLON exp
                               { CREATE_NODE_OP3 (REC_SEX_OP_COND, $$, $1, $3, $5); }
    | exp REC_SEX_TOK_EQL exp  { CREATE_NODE_OP2 (REC_SEX_OP_EQL, $$, $1, $3); }
    | exp REC_SEX_TOK_NEQ exp  { CREATE_NODE_OP2 (REC_SEX_OP_NEQ, $$, $1, $3); }
    | exp REC_SEX_TOK_MAT exp  
    {
      if ((rec_sex_ast_node_type ($1) == REC_SEX_INT)
          || (rec_sex_ast_node_type ($3) == REC_SEX_INT))
        {
           rec_sex_ast_node_destroy ($1);
           rec_sex_ast_node_destroy ($3);
           YYABORT;
        }

      CREATE_NODE_OP2 (REC_SEX_OP_MAT, $$, $1, $3);
    }
    | exp REC_SEX_TOK_ADD exp  { CREATE_NODE_OP2 (REC_SEX_OP_ADD, $$, $1, $3); }
    | exp REC_SEX_TOK_SUB exp  { CREATE_NODE_OP2 (REC_SEX_OP_SUB, $$, $1, $3); }
    | exp REC_SEX_TOK_MUL exp  { CREATE_NODE_OP2 (REC_SEX_OP_MUL, $$, $1, $3); }
    | exp REC_SEX_TOK_DIV exp  { CREATE_NODE_OP2 (REC_SEX_OP_DIV, $$, $1, $3); }
    | exp REC_SEX_TOK_MOD exp  { CREATE_NODE_OP2 (REC_SEX_OP_MOD, $$, $1, $3); }
    | exp REC_SEX_TOK_GT exp   { CREATE_NODE_OP2 (REC_SEX_OP_GT, $$, $1, $3); }
    | exp REC_SEX_TOK_LT exp   { CREATE_NODE_OP2 (REC_SEX_OP_LT, $$, $1, $3); }
    | exp REC_SEX_TOK_GTE exp  { CREATE_NODE_OP2 (REC_SEX_OP_GTE, $$, $1, $3); }
    | exp REC_SEX_TOK_LTE exp  { CREATE_NODE_OP2 (REC_SEX_OP_LTE, $$, $1, $3); }
    | exp REC_SEX_TOK_AFTER exp { CREATE_NODE_OP2 (REC_SEX_OP_AFTER, $$, $1, $3); }
    | exp REC_SEX_TOK_BEFORE exp { CREATE_NODE_OP2 (REC_SEX_OP_BEFORE, $$, $1, $3); }
    | exp REC_SEX_TOK_SAMETIME exp { CREATE_NODE_OP2 (REC_SEX_OP_SAMETIME, $$, $1, $3); }
    | exp REC_SEX_TOK_IMPLIES exp  { CREATE_NODE_OP2 (REC_SEX_OP_IMPLIES, $$, $1, $3); }
    | REC_SEX_TOK_NOT exp      { CREATE_NODE_OP1 (REC_SEX_OP_NOT, $$, $2); }
    | exp REC_SEX_TOK_AND exp  { CREATE_NODE_OP2 (REC_SEX_OP_AND, $$, $1, $3); }
    | exp REC_SEX_TOK_OR exp   { CREATE_NODE_OP2 (REC_SEX_OP_OR, $$, $1, $3); }
    | exp REC_SEX_TOK_AMP exp  { CREATE_NODE_OP2 (REC_SEX_OP_CONCAT, $$, $1, $3); }
    | REC_SEX_TOK_SHARP REC_SEX_TOK_NAM    { CREATE_NODE_OP1 (REC_SEX_OP_SHA, $$, $2); }
    | REC_SEX_TOK_BP exp REC_SEX_TOK_EP { $$ = $2; }

%%

/* End of rec-sex.y */
