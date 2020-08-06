/* -*- mode: C -*-
 *
 *       File:         rec-sex.c
 *       Date:         Sat Jan  9 20:28:43 2010
 *
 *       GNU recutils - Record Selection Expressions.
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

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <parse-datetime.h>

#include <rec.h>
#include <rec-utils.h>
#include <rec-sex-ast.h>
#include <rec-sex-parser.h>
#include <rec-sex-tab.h>

/*
 * Data structures
 */

struct rec_sex_s
{
  rec_sex_ast_t ast;
  rec_sex_parser_t parser;
};

#define REC_SEX_VAL_INT  0
#define REC_SEX_VAL_REAL 1
#define REC_SEX_VAL_STR  2

struct rec_sex_val_s
{
  int type;

  int int_val;
  double real_val;
  char *str_val;
};

/* Static functions declarations.  */
static struct rec_sex_val_s rec_sex_eval_node (rec_sex_t sex,
                                               rec_record_t record,
                                               rec_sex_ast_node_t node,
                                               bool *status);
static bool rec_sex_op_real_p (struct rec_sex_val_s op1,
                               struct rec_sex_val_s op2);

/*
 * Public functions.
 */

rec_sex_t
rec_sex_new (bool case_insensitive)
{
  rec_sex_t new;

  new = malloc (sizeof (struct rec_sex_s));
  if (new)
    {
      /* Initialize a new parser.  */
      new->parser = rec_sex_parser_new ();
      rec_sex_parser_set_case_insensitive (new->parser,
                                           case_insensitive);

      /* Initialize a new AST.  */
      new->ast = NULL;
    }

  return new;
}

void
rec_sex_destroy (rec_sex_t sex)
{
  if (sex)
    {
      if (sex->parser)
        {
          rec_sex_parser_destroy (sex->parser);
        }
      
      if (sex->ast)
        {
          rec_sex_ast_destroy (sex->ast);
        }
      
      free (sex);  /* yeah! :D */
    }
}

bool
rec_sex_compile (rec_sex_t sex,
                 const char *expr)
{
  bool res;

  res = rec_sex_parser_run (sex->parser, expr);
  if (res)
    {
      sex->ast = rec_sex_parser_ast (sex->parser);
    }
  return res;
}

#define EXEC_AST(RECORD)                                                \
  do                                                                    \
    {                                                                   \
      val = rec_sex_eval_node (sex,                                     \
                               (RECORD),                                \
                               rec_sex_ast_top (sex->ast),              \
                               status);                                 \
                                                                        \
      switch (val.type)                                                 \
        {                                                               \
        case REC_SEX_VAL_INT:                                           \
          {                                                             \
            res = (val.int_val != 0);                                   \
            break;                                                      \
          }                                                             \
        case REC_SEX_VAL_REAL:                                          \
        case REC_SEX_VAL_STR:                                           \
          {                                                             \
            res = false;                                                \
            break;                                                      \
          }                                                             \
        }                                                               \
    }                                                                   \
  while (0)

char *
rec_sex_eval_str (rec_sex_t sex,
                  rec_record_t record)
{
  char *res;
  struct rec_sex_val_s val;
  bool status;

  rec_sex_ast_node_unfix (rec_sex_ast_top (sex->ast));
  val = rec_sex_eval_node (sex,
                           record,
                           rec_sex_ast_top (sex->ast),
                           &status);

  if (!status)
    {
      /* Error evaluating the expression.  */
      return NULL;
    }

  res = NULL;
  switch (val.type)
    {
    case REC_SEX_VAL_INT:
      {
        asprintf (&res, "%d", val.int_val);
        break;
      }
    case REC_SEX_VAL_REAL:
      {
        asprintf (&res, "%f", val.real_val);
        break;
      }
    case REC_SEX_VAL_STR:
      {
        res = strdup (val.str_val);
        break;
      }
    }

  return res;
}

bool
rec_sex_eval (rec_sex_t sex,
              rec_record_t record,
              bool *status)
{
  bool res;
  rec_field_t field;
  rec_field_t wfield;
  rec_record_t wrec;
  rec_mset_iterator_t iter;
  int j, nf;
  struct rec_sex_val_s val;
  
  res = false;
  wrec = NULL;

  rec_sex_ast_node_unfix (rec_sex_ast_top (sex->ast));
  EXEC_AST (record);
  if (res)
    {
      goto exit;
    }

  rec_record_reset_marks (record);

  iter = rec_mset_iterator (rec_record_mset (record));
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void**) &field, NULL))
    {
      nf = rec_record_get_num_fields_by_name (record, rec_field_name (field));
      if ((nf > 1)
          && (rec_record_field_mark (record, field) == 0)
          && (rec_sex_ast_name_p (sex->ast, rec_field_name (field), nf))
          && (!rec_sex_ast_hash_name_p (sex->ast, rec_field_name (field))))
        {
          for (j = 0; j < nf; j++)
            {
              wfield = rec_record_get_field_by_name (record,
                                                     rec_field_name (field),
                                                     j);
              if (wrec)
                {
                  rec_record_destroy (wrec);
                }

              rec_record_mark_field (record, wfield, 1);

              wrec = rec_record_dup (record);
              rec_record_remove_field_by_name (wrec,
                                               rec_field_name (field),
                                               -1); /* Delete all.  */
              rec_mset_append (rec_record_mset (wrec), MSET_FIELD, (void *) rec_field_dup (wfield), MSET_ANY);

              EXEC_AST(wrec);

              if (res)
                {
                  rec_record_destroy (wrec);
                  goto exit;
                }
            }
        }
    }

  rec_mset_iterator_free (&iter);

 exit:          

  if (!*status)
    {
      res = false;
    }

  return res;
}

void
rec_sex_print_ast (rec_sex_t sex)
{
  rec_sex_parser_print_ast (sex->parser);
}

/*
 * Private functions.
 */

#define GET_CHILD_VAL(DEST,NUM)                                         \
  do                                                                    \
    {                                                                   \
      (DEST) = rec_sex_eval_node (sex,                                  \
                                  record,                               \
                                  rec_sex_ast_node_child (node, (NUM)), \
                                  status);                              \
      if (!*status)                                                     \
        {                                                               \
          return res;                                                   \
        }                                                               \
    }                                                                   \
    while (0)


#define ATOI_VAL(DEST, VAL)                             \
  do                                                    \
    {                                                   \
      switch ((VAL).type)                               \
        {                                               \
        case REC_SEX_VAL_INT:                           \
          {                                             \
            (DEST) = (VAL).int_val;                     \
            break;                                      \
          }                                             \
        case REC_SEX_VAL_STR:                           \
          {                                             \
          if (strcmp ((VAL).str_val, "") == 0)          \
              {                                         \
                (DEST) = 0;                             \
              }                                         \
            else                                        \
              {                                         \
                if (!rec_atoi ((VAL).str_val, &(DEST))) \
                {                                       \
                  *status = false;                      \
                  return res;                           \
                }                                       \
              }                                         \
          break;                                        \
        }                                               \
    }                                                   \
  }                                                     \
  while (0)

#define ATOD_VAL(DEST, VAL)                             \
  do                                                    \
    {                                                   \
      switch ((VAL).type)                               \
        {                                               \
        case REC_SEX_VAL_REAL:                          \
          {                                             \
            (DEST) = (VAL).real_val;                    \
            break;                                      \
          }                                             \
        case REC_SEX_VAL_INT:                           \
          {                                             \
            (DEST) = (VAL).int_val;                     \
            break;                                      \
          }                                             \
        case REC_SEX_VAL_STR:                           \
          {                                             \
          if (strcmp ((VAL).str_val, "") == 0)          \
              {                                         \
                (DEST) = 0.0;                           \
              }                                         \
            else                                        \
              {                                         \
                if (!rec_atod ((VAL).str_val, &(DEST))) \
                {                                       \
                  *status = false;                      \
                  return res;                           \
                }                                       \
              }                                         \
          break;                                        \
        }                                               \
    }                                                   \
  }                                                     \
  while (0)

#define ATOTS_VAL(DEST, VAL)                            \
  do                                                    \
    {                                                   \
      switch ((VAL).type)                               \
        {                                               \
        case REC_SEX_VAL_REAL:                          \
          {                                             \
           *status = false;                             \
           return res;                                  \
           break;                                       \
          }                                             \
        case REC_SEX_VAL_INT:                           \
          {                                             \
            *status = false;                            \
            return res;                                 \
            break;                                      \
          }                                             \
        case REC_SEX_VAL_STR:                           \
          {                                             \
            if (!parse_datetime (&(DEST), (VAL).str_val, NULL))\
            {                                           \
              *status = false;                          \
              return res;                               \
            }                                           \
                                                        \
            break;                                      \
          }                                             \
        }                                               \
    }                                                   \
  while (0)

struct rec_sex_val_s
rec_sex_eval_node (rec_sex_t sex,
                   rec_record_t record,
                   rec_sex_ast_node_t node,
                   bool *status)
{
  struct rec_sex_val_s res = {0, 0, 0, NULL};
  struct rec_sex_val_s child_val1 = {0, 0, 0, NULL};
  struct rec_sex_val_s child_val2 = {0, 0, 0, NULL};
  struct rec_sex_val_s child_val3 = {0, 0, 0, NULL};

  *status = true;

  switch (rec_sex_ast_node_type (node))
    {
    case REC_SEX_NOVAL:
      {
        fprintf (stderr, "Application bug: REC_SEX_NOVAL node found.\nPlease report this!\n");
        exit (EXIT_FAILURE);
        break;
      }
      /* Operations.  */
    case REC_SEX_OP_NEG:
    case REC_SEX_OP_ADD:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if (rec_sex_op_real_p (child_val1, child_val2))
          {
            /* Real operation.  */
            ATOD_VAL (op1_real, child_val1);
            ATOD_VAL (op2_real, child_val2);

            res.type = REC_SEX_VAL_REAL;
            res.real_val = op1_real + op2_real;
          }
        else
          {
            /* Integer operation.  */
            ATOI_VAL (op1, child_val1);
            ATOI_VAL (op2, child_val2);

            res.type = REC_SEX_VAL_INT;
            res.int_val = op1 + op2;
          }

        break;
      }
    case REC_SEX_OP_SUB:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if (rec_sex_op_real_p (child_val1, child_val2))
          {
            /* Real operation.  */
            ATOD_VAL (op1_real, child_val1);
            ATOD_VAL (op2_real, child_val2);

            res.type = REC_SEX_VAL_REAL;
            res.real_val = op1 - op2;
          }
        else
          {
            /* Integer operation.  */

            ATOI_VAL (op1, child_val1);
            ATOI_VAL (op2, child_val2);

            res.type = REC_SEX_VAL_INT;
            res.int_val = op1 - op2;
          }

        break;
      }
    case REC_SEX_OP_MUL:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if (rec_sex_op_real_p (child_val1, child_val2))
          {
            /* Real operation.  */
            ATOD_VAL (op1_real, child_val1);
            ATOD_VAL (op2_real, child_val2);

            res.type = REC_SEX_VAL_REAL;
            res.real_val = op1_real * op2_real;
          }
        else
          {
            /* Integer operation.  */
            ATOI_VAL (op1, child_val1);
            ATOI_VAL (op2, child_val2);
            
            res.type = REC_SEX_VAL_INT;
            res.int_val = op1 * op2;
          }

        break;
      }
    case REC_SEX_OP_DIV:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if (rec_sex_op_real_p (child_val1, child_val2))
          {
            /* Real operation.  */
            ATOD_VAL (op1_real, child_val1);
            ATOD_VAL (op2_real, child_val2);

            res.type = REC_SEX_VAL_REAL;
            res.real_val = op1_real / op2_real;
          }
        else
          {
            /* Integer operation.  */
            ATOI_VAL (op1, child_val1);
            ATOI_VAL (op2, child_val2);
            
            res.type = REC_SEX_VAL_INT;
            
            if (op2 != 0)
              {
                res.int_val = op1 / op2;
              }
            else
              {
                /* Error: division by zero */
                *status = false;
                return res;
              }
          }

        break;
      }
    case REC_SEX_OP_MOD:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        /* Integer operation.  */
        ATOI_VAL (op1, child_val1);
        ATOI_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;

        if (op2 != 0)
          {
            res.int_val = op1 % op2;
          }
        else
          {
            /* Error: division by zero */
            *status = false;
            return res;
          }

        break;
      }
    case REC_SEX_OP_EQL:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if ((child_val1.type == REC_SEX_VAL_STR)
            && (child_val2.type == REC_SEX_VAL_STR))
          {
            /* String comparison.  */
            res.type = REC_SEX_VAL_INT;

            if (rec_sex_parser_case_insensitive (sex->parser))
              {
                res.int_val = (strcasecmp (child_val1.str_val,
                                           child_val2.str_val) == 0);
              }
            else
              {
                res.int_val = (strcmp (child_val1.str_val,
                                       child_val2.str_val) == 0);
              }
          }
        else
          {
            if (rec_sex_op_real_p (child_val1, child_val2))
              {
                /* Real comparison.  */
                ATOD_VAL (op1_real, child_val1);
                ATOD_VAL (op2_real, child_val2);

                res.type = REC_SEX_VAL_INT;
                res.int_val = op1_real == op2_real;
              }
            else
              {
                /* Integer comparison.  */
                ATOI_VAL (op1, child_val1);
                ATOI_VAL (op2, child_val2);
                
                res.type = REC_SEX_VAL_INT;
                res.int_val = op1 == op2;
              }
          }

        break;
      }
    case REC_SEX_OP_NEQ:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if ((child_val1.type == REC_SEX_VAL_STR)
            && (child_val2.type == REC_SEX_VAL_STR))
          {
            /* String comparison.  */
            res.type = REC_SEX_VAL_INT;

            if (rec_sex_parser_case_insensitive (sex->parser))
              {
                res.int_val = (strcasecmp (child_val1.str_val,
                                           child_val2.str_val) != 0);
              }
            else
              {
                res.int_val = (strcmp (child_val1.str_val,
                                       child_val2.str_val) != 0);
              }
          }
        else
          {
            if (rec_sex_op_real_p (child_val1, child_val2))
              {
                /* Real comparison.  */
                ATOD_VAL (op1_real, child_val1);
                ATOD_VAL (op2_real, child_val2);

                res.type = REC_SEX_VAL_INT;
                res.int_val = op1_real != op2_real;
              }
            else
              {
                /* Integer comparison.  */
                ATOI_VAL (op1, child_val1);
                ATOI_VAL (op2, child_val2);
            
                res.type = REC_SEX_VAL_INT;
                res.int_val = op1 != op2;
              }
          }

        break;
      }
    case REC_SEX_OP_MAT:
      {
        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if ((child_val1.type == REC_SEX_VAL_STR)
            && (child_val2.type == REC_SEX_VAL_STR))
          {
            /* String match.  */
            res.type = REC_SEX_VAL_INT;

            if (rec_sex_parser_case_insensitive (sex->parser))
              {
                res.int_val =
                  rec_match_insensitive (child_val1.str_val, child_val2.str_val);
              }
            else
              {
                res.int_val =
                  rec_match (child_val1.str_val, child_val2.str_val);
              }
          }
        else
          {
            /* Error.  */
            *status = false;
            return res;
          }

        break;
      }
    case REC_SEX_OP_BEFORE:
      {
        struct timespec op1;
        struct timespec op2;
        struct timespec diff;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOTS_VAL (op1, child_val1);
        ATOTS_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = rec_timespec_subtract (&diff, &op1, &op2);
        break;
      }
    case REC_SEX_OP_AFTER:
      {
        struct timespec op1;
        struct timespec op2;
        struct timespec diff;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOTS_VAL (op1, child_val1);
        ATOTS_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = (!rec_timespec_subtract (&diff, &op1, &op2)
                       && ((diff.tv_sec != 0) || (diff.tv_nsec != 0)));
        break;
      }
    case REC_SEX_OP_SAMETIME:
      {
        struct timespec op1;
        struct timespec op2;
        struct timespec diff;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOTS_VAL (op1, child_val1);
        ATOTS_VAL (op2, child_val2);

        rec_timespec_subtract (&diff, &op1, &op2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = ((diff.tv_sec == 0) && (diff.tv_nsec == 0));
        break;
      }
    case REC_SEX_OP_IMPLIES:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOI_VAL (op1, child_val1);
        ATOI_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = !op1 || (op1 && op2);

        break;
      }
    case REC_SEX_OP_LT:
    case REC_SEX_OP_LTE:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if (rec_sex_op_real_p (child_val1, child_val2))
          {
            /* Real comparison.  */
            ATOD_VAL (op1_real, child_val1);
            ATOD_VAL (op2_real, child_val2);

            res.type = REC_SEX_VAL_INT;

            if (rec_sex_ast_node_type (node) == REC_SEX_OP_LT)
              {
                res.int_val = op1_real < op2_real;
              }
            else
              {
                res.int_val = op1_real <= op2_real;
              }
          }
        else
          {
            /* Integer comparison.  */
            ATOI_VAL (op1, child_val1);
            ATOI_VAL (op2, child_val2);
            
            res.type = REC_SEX_VAL_INT;

            if (rec_sex_ast_node_type (node) == REC_SEX_OP_LT)
              {
                res.int_val = op1 < op2;
              }
            else
              {
                res.int_val = op1 <= op2;
              }
          }

        break;
      }
    case REC_SEX_OP_GT:
    case REC_SEX_OP_GTE:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if (rec_sex_op_real_p (child_val1, child_val2))
          {
            /* Real comparison.  */
            ATOD_VAL (op1_real, child_val1);
            ATOD_VAL (op2_real, child_val2);

            res.type = REC_SEX_VAL_INT;

            if (rec_sex_ast_node_type (node) == REC_SEX_OP_GT)
              {
                res.int_val = op1_real > op2_real;
              }
            else
              {
                res.int_val = op1_real >= op2_real;
              }
          }
        else
          {
            /* Integer comparison.  */
            ATOI_VAL (op1, child_val1);
            ATOI_VAL (op2, child_val2);
            
            res.type = REC_SEX_VAL_INT;

            if (rec_sex_ast_node_type (node) == REC_SEX_OP_GT)
              {
                res.int_val = op1 > op2;
              }
            else
              {
                res.int_val = op1 >= op2;
              }
          }

        break;
      }
    case REC_SEX_OP_AND:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOI_VAL (op1, child_val1);
        ATOI_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = op1 && op2;

        break;
      }
    case REC_SEX_OP_OR:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOI_VAL (op1, child_val1);
        ATOI_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = op1 || op2;

        break;
      }
    case REC_SEX_OP_CONCAT:
      {
        size_t str1_size;
        size_t str2_size;
        
        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if ((child_val1.type == REC_SEX_VAL_STR)
            && (child_val2.type == REC_SEX_VAL_STR))
          {
            str1_size = strlen (child_val1.str_val);
            str2_size = strlen (child_val2.str_val);

            res.type = REC_SEX_VAL_STR;
            res.str_val = malloc (str1_size + str2_size + 1);
            memcpy (res.str_val, child_val1.str_val, str1_size);
            memcpy (res.str_val + str1_size, child_val2.str_val, str2_size);
            res.str_val[str1_size + str2_size] = '\0';
          }
        else
          {
            *status = false;
            return res;
          }

        break;
      }
    case REC_SEX_OP_NOT:
      {
        int op;

        GET_CHILD_VAL (child_val1, 0);
        ATOI_VAL (op, child_val1);

        res.type = REC_SEX_VAL_INT;
        res.int_val = !op;

        break;
      }
    case REC_SEX_OP_SHA:
      {
        int n;
        const char *field_name    = NULL;
        const char *field_subname = NULL;
        rec_sex_ast_node_t child;

        /* The child should be a Name.  */
        child = rec_sex_ast_node_child (node, 0);
        if (rec_sex_ast_node_type (rec_sex_ast_node_child(node, 0))
            != REC_SEX_NAME)
          {
            *status = false;
            return res;
          }

        field_name = rec_sex_ast_node_name (child);
        field_subname = rec_sex_ast_node_subname (child);

        if (field_subname)
          {
            /* Compound a field name from the name/subname pair in the
               AST node.  */
            
            char *effective_name
              = rec_concat_strings (field_name, "_", field_subname);

            n = rec_record_get_num_fields_by_name (record,
                                                   effective_name);
            free (effective_name);
          }
        else
          {
            n = rec_record_get_num_fields_by_name (record, field_name);
          }

        res.type = REC_SEX_VAL_INT;
        res.int_val = n;
        break;
      }
    case REC_SEX_OP_COND:
      {
        int op1;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);
        GET_CHILD_VAL (child_val3, 2);

        /* Get the boolean value of the first operand.  */
        ATOI_VAL (op1, child_val1);

        /* Return the first or the second operand, depending on the
           value of op1.  */
        if (op1)
          {
            res = child_val2;
          }
        else
          {
            res = child_val3;
          }

        break;
      }
      /* Values.  */
    case REC_SEX_INT:
      {
        res.type = REC_SEX_VAL_INT;
        res.int_val = rec_sex_ast_node_int (node);
        break;
      }
    case REC_SEX_REAL:
      {
        res.type = REC_SEX_VAL_REAL;
        res.real_val = rec_sex_ast_node_real (node);
        break;
      }
    case REC_SEX_STR:
      {
        res.type = REC_SEX_VAL_STR;
        res.str_val = rec_sex_ast_node_str (node);
        break;
      }
    case REC_SEX_NAME:
      {
        rec_field_t field;
        const char *field_name;
        const char *field_subname;
        int index;
        bool tofix;

        if (rec_sex_ast_node_fixed (node))
          {
            res.type = REC_SEX_VAL_STR;
            res.str_val = rec_sex_ast_node_fixed_val (node);
          }
        else
          {
            field_name = rec_sex_ast_node_name (node);
            field_subname = rec_sex_ast_node_subname (node);
            index = rec_sex_ast_node_index (node);
            tofix = (index != -1);
            if (index == -1)
              {
                index = 0;
              }

            /* If there is a subname then the effective field name is
               the concatenation of the name and the subname separated
               by a '_' character.  Otherwise it is just the name.  */

            {
              if (field_subname)
                {
                  char *effective_field_name = malloc (sizeof (char) *
                                                       (strlen (field_name) + strlen (field_subname) + 2));
                  memcpy (effective_field_name, field_name, strlen(field_name));
                  effective_field_name[strlen(field_name)] = '_';
                  memcpy (effective_field_name + strlen(field_name) + 1, field_subname, strlen(field_subname) + 1);

                  field = rec_record_get_field_by_name (record, effective_field_name, index);
                }
              else
                {
                  field = rec_record_get_field_by_name (record, field_name, index);
                }
            }

            res.type = REC_SEX_VAL_STR;
            if (field)
              {
                res.str_val = strdup (rec_field_value (field));
              }
            else
              {
                /* No field => ""  */
                res.str_val = "";
              }

            if (tofix)
              {
                /* Make this node fixed.  */
                rec_sex_ast_node_fix (node, res.str_val);
              }
          }

        break;
      }
    }

  return res;
}

static bool
rec_sex_op_real_p (struct rec_sex_val_s op1,
                   struct rec_sex_val_s op2)
{
  bool ret;
  int integer;
  double real;

  ret = true;

  if ((op1.type == REC_SEX_VAL_INT)
      || ((op1.type == REC_SEX_VAL_STR)
          && rec_atoi (op1.str_val, &integer)))
    {
      /* Operand 1 is an integer.  */
      switch (op2.type)
        {
        case REC_SEX_VAL_INT:
          {
            ret = false;
            break;
          }
        case REC_SEX_VAL_REAL:
          {
            ret = true;
            break;
          }
        case REC_SEX_VAL_STR:
          {
            ret = (rec_atod (op2.str_val, &real)
                   && (!rec_atoi (op2.str_val, &integer)));
            break;
          }
        default:
          {
            ret = false;
            break;
          }
        }
    }

  if ((op1.type == REC_SEX_VAL_REAL)
      || ((op1.type == REC_SEX_VAL_STR)
          && rec_atod (op1.str_val, &real)
          && (!rec_atoi (op1.str_val, &integer))))
    {
      /* Operand 1 is a real.  */
      switch (op2.type)
        {
        case REC_SEX_VAL_INT:
          {
            ret = true;
            break;
          }
        case REC_SEX_VAL_REAL:
          {
            ret = true;
            break;
          }
        case REC_SEX_VAL_STR:
          {
            ret = rec_atod (op2.str_val, &real);
            break;
          }
        default:
          {
            ret = false;
            break;
          }
        }
    }

  return ret;
}
 
/* End of rec-sex.c */
