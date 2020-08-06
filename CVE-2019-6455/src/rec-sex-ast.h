/* -*- mode: C -*-
 *
 *       File:         rec-sex-ast.h
 *       Date:         Tue Jan 12 17:07:59 2010
 *
 *       GNU recutils - SEX Abstract Syntax Trees
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

#ifndef REC_SEX_AST_H
#define REC_SEX_AST_H

#include <config.h>

#include <stdbool.h>

enum rec_sex_ast_node_type_e
{
  REC_SEX_NOVAL,

  /* Operations.  */
  REC_SEX_OP_NEG,
  REC_SEX_OP_ADD,
  REC_SEX_OP_SUB,
  REC_SEX_OP_MUL,
  REC_SEX_OP_DIV,
  REC_SEX_OP_MOD,
  REC_SEX_OP_EQL,
  REC_SEX_OP_NEQ,
  REC_SEX_OP_MAT,
  REC_SEX_OP_LT,
  REC_SEX_OP_GT,
  REC_SEX_OP_LTE,
  REC_SEX_OP_GTE,
  REC_SEX_OP_AND,
  REC_SEX_OP_OR,
  REC_SEX_OP_NOT,
  REC_SEX_OP_SHA,
  REC_SEX_OP_SAMETIME,
  REC_SEX_OP_IMPLIES,
  REC_SEX_OP_BEFORE,
  REC_SEX_OP_AFTER,
  REC_SEX_OP_COND,
  REC_SEX_OP_CONCAT,

  /* Values.  */
  REC_SEX_INT,
  REC_SEX_REAL,
  REC_SEX_STR,
  REC_SEX_NAME
};

typedef struct rec_sex_ast_node_s *rec_sex_ast_node_t;
typedef struct rec_sex_ast_s *rec_sex_ast_t;

/*
 * Public functions.
 */


/* Creation and destruction of ASTs.  */
rec_sex_ast_t rec_sex_ast_new ();
void rec_sex_ast_destroy (rec_sex_ast_t ast);

/* Running the AST.  */
/* Not here but in rec-sex.c int rec_sex_ast_run (rec_sex_ast_t ast, rec_record_t record); */

rec_sex_ast_node_t rec_sex_ast_top (rec_sex_ast_t ast);
void rec_sex_ast_set_top (rec_sex_ast_t ast, rec_sex_ast_node_t node);

/* Nodes management.  */
rec_sex_ast_node_t rec_sex_ast_node_new (void);
void rec_sex_ast_node_destroy (rec_sex_ast_node_t node);

enum rec_sex_ast_node_type_e rec_sex_ast_node_type (rec_sex_ast_node_t node);
void rec_sex_ast_node_set_type (rec_sex_ast_node_t node,
                                enum rec_sex_ast_node_type_e type);

int rec_sex_ast_node_int (rec_sex_ast_node_t node);
void rec_sex_ast_node_set_int (rec_sex_ast_node_t node, int num);
double rec_sex_ast_node_real (rec_sex_ast_node_t node);
void rec_sex_ast_node_set_real (rec_sex_ast_node_t node, double num);
char *rec_sex_ast_node_str (rec_sex_ast_node_t node);
void rec_sex_ast_node_set_str (rec_sex_ast_node_t node, char *str);
const char *rec_sex_ast_node_name (rec_sex_ast_node_t node);
const char *rec_sex_ast_node_subname (rec_sex_ast_node_t node);
void rec_sex_ast_node_set_name (rec_sex_ast_node_t node, const char *name, const char *subname);

int rec_sex_ast_node_num_children (rec_sex_ast_node_t node);

rec_sex_ast_node_t rec_sex_ast_node_child (rec_sex_ast_node_t node,
                                           int n);

void rec_sex_ast_node_link (rec_sex_ast_node_t parent,
                            rec_sex_ast_node_t child);

void rec_sex_ast_node_reset (rec_sex_ast_node_t node);
int rec_sex_ast_node_index (rec_sex_ast_node_t node);
void rec_sex_ast_node_set_index (rec_sex_ast_node_t node,
                                 int index);

void rec_sex_ast_print (rec_sex_ast_t ast);

void rec_sex_ast_node_fix (rec_sex_ast_node_t node, char *val);
void rec_sex_ast_node_unfix (rec_sex_ast_node_t node);
bool rec_sex_ast_node_fixed (rec_sex_ast_node_t node);
char *rec_sex_ast_node_fixed_val (rec_sex_ast_node_t node);

/* This function returns 'true' if there is a node on AST of type
   REC_SEX_NAME where NAME.name == NAME and NAME.idx <= IDX.  */
bool rec_sex_ast_name_p (rec_sex_ast_t ast, const char *name, size_t idx);

/* This function returns 'true' if there is a node on AST of type
   REC_SEX_NAME where NAME.name == NAME and the parent of the node is
   of type REC_SEX_OP_SHA, i.e. it recognizes #NAME in the source.  */
bool rec_sex_ast_hash_name_p (rec_sex_ast_t ast, const char *name);

#endif /* rec-sex-ast.h */


/* End of rec-sex-ast.h */
