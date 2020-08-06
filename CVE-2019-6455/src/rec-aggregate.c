/* -*- mode: C -*-
 *
 *       File:         rec-aggregate.c
 *       Date:         Mon Apr 23 11:05:57 2012
 *
 *       GNU recutils - Support for aggregate functions
 *
 */

/* Copyright (C) 2012-2019 Jose E. Marchesi */

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
#include <float.h>
#include <math.h>
#include <minmax.h>

#include <rec-utils.h>
#include <rec.h>

/*
 * Data structures.
 */

#define MAX_FUNCTIONS 40

struct rec_aggregate_reg_elem_s
{
  char *name;
  rec_aggregate_t function;
};

struct rec_aggregate_reg_s
{
  struct rec_aggregate_reg_elem_s functions[MAX_FUNCTIONS];
  size_t num_functions;
};

/* Static functions defined in this file.  */

static char *rec_aggregate_std_count (rec_rset_t rset,
                                      rec_record_t record,
                                      const char *field_name);

static char *rec_aggregate_std_avg (rec_rset_t rset,
                                    rec_record_t record,
                                    const char *field_name);
static double rec_aggregate_std_avg_record (rec_record_t record,
                                            const char *field_name);

static char *rec_aggregate_std_sum (rec_rset_t rset,
                                    rec_record_t record,
                                    const char *field_name);
static double rec_aggregate_std_sum_record (rec_record_t record,
                                            const char *field_name);

static char *rec_aggregate_std_min (rec_rset_t rset,
                                    rec_record_t record,
                                    const char *field_name);
static double rec_aggregate_std_min_record (rec_record_t record,
                                            const char *field_name);

static char *rec_aggregate_std_max (rec_rset_t rset,
                                    rec_record_t record,
                                    const char *field_name);
static double rec_aggregate_std_max_record (rec_record_t record,
                                            const char *field_name);

/*
 * Static structure containing the descriptors of the standard
 * aggregates.
 *
 * The aggregate names must be in lower-case, even if the matching is
 * case-insensitive.
 */

struct rec_aggregate_descriptor_s
{
  const char *name;
  rec_aggregate_t func;
};

#define NUM_STD_AGGREGATES 5

static struct rec_aggregate_descriptor_s std_aggregates[] =
  {{"count", &rec_aggregate_std_count},
   {"avg",   &rec_aggregate_std_avg},
   {"sum",   &rec_aggregate_std_sum},
   {"min",   &rec_aggregate_std_min},
   {"max",   &rec_aggregate_std_max}};

/*
 * Public functions.
 */

rec_aggregate_reg_t
rec_aggregate_reg_new (void)
{
  rec_aggregate_reg_t new;

  new = malloc (sizeof (struct rec_aggregate_reg_s));
  if (new)
    new->num_functions = 0;

  return new;
}

void
rec_aggregate_reg_destroy (rec_aggregate_reg_t func_reg)
{
  if (func_reg)
    {
      size_t i = 0;

      for (i = 0; i < func_reg->num_functions; i++)
        free (func_reg->functions[i].name);
      free (func_reg);
    }
}

bool
rec_aggregate_reg_add (rec_aggregate_reg_t func_reg,
                       const char *name,
                       rec_aggregate_t function)
{
  bool function_replaced = false;
  size_t i = 0;

  for (i = 0; i < func_reg->num_functions; i++)
    if (strcmp (name, func_reg->functions[i].name) == 0)
      {
        /* Replace the existing function.  */
        func_reg->functions[i].function = function;
        function_replaced = true;
        break;
      }

  if (!function_replaced)
    {
      /* Insert the function into a new entry in the registry.  */

      if (func_reg->num_functions == MAX_FUNCTIONS)
        /* FIXME: this is crappy as hell.  */
        return false;

      func_reg->functions[func_reg->num_functions].name = strdup (name);
      func_reg->functions[func_reg->num_functions].function = function;
      func_reg->num_functions++;
    }

  return true;
}

rec_aggregate_t
rec_aggregate_reg_get (rec_aggregate_reg_t func_reg,
                       const char *name)
{
  size_t i = 0;
  rec_aggregate_t res = NULL;

  for (i = 0; i < func_reg->num_functions; i++)
    if (strcasecmp (func_reg->functions[i].name, name) == 0)
      {
        res = func_reg->functions[i].function;
        break;
      }

  return res;
}

void
rec_aggregate_reg_add_standard (rec_aggregate_reg_t func_reg)
{
  size_t i = 0;

  for (i = 0; i < NUM_STD_AGGREGATES; i++)
    rec_aggregate_reg_add (func_reg, std_aggregates[i].name, std_aggregates[i].func);
}

bool
rec_aggregate_std_p (const char *name)
{
  bool found = false;
  size_t i   = 0;

  for (i = 0; i < NUM_STD_AGGREGATES; i++)
    if (strcasecmp (name, std_aggregates[i].name) == 0)
      {
        found = true;
        break;
      }

  return found;
}

/*
 * Private functions.
 */

static char *
rec_aggregate_std_count (rec_rset_t rset,
                         rec_record_t record,
                         const char *field_name)
{
  char *result = NULL;
  size_t count = 0;

  if (record)
    count = rec_record_get_num_fields_by_name (record, field_name);
  else if (rset)
    {
      rec_record_t rec = NULL;
      rec_mset_iterator_t iter = rec_mset_iterator (rec_rset_mset (rset));

      while (rec_mset_iterator_next (&iter, MSET_RECORD, (void *) &rec, NULL))
        count = count + rec_record_get_num_fields_by_name (rec, field_name);
      rec_mset_iterator_free (&iter);
    }

  /* Return the count as a string.  Note that if NULL is returned it
     will be returned by this function below to signal the
     end-of-memory condition.  */
      
  asprintf (&result, "%zu", count);
  return result;
}

static char *
rec_aggregate_std_avg (rec_rset_t rset,
                       rec_record_t record,
                       const char *field_name)
{
  char *result = NULL;
  double avg = 0;

  if (record)
    avg = rec_aggregate_std_avg_record (record, field_name);
  else if (rset)
    {
      size_t num_records = 0;
      rec_record_t rec = NULL;
      rec_mset_iterator_t iter = rec_mset_iterator (rec_rset_mset (rset));

      while (rec_mset_iterator_next (&iter, MSET_RECORD, (void *) &rec, NULL))
        {
          avg = avg + rec_aggregate_std_avg_record (rec, field_name);
          num_records++;
        }
      rec_mset_iterator_free (&iter);

      if (num_records != 0)
        avg = avg / num_records;
    }

  /* Return the average as a string.  Note that if NULL is returned it
     will be returned by this function below to signal the
     end-of-memory condition.  */
  asprintf (&result, "%g", avg);

  return result;
}

static double
rec_aggregate_std_avg_record (rec_record_t record,
                              const char *field_name)
{
  double avg = 0;
  rec_field_t field;
  size_t num_fields = 0;
  rec_mset_iterator_t iter = rec_mset_iterator (rec_record_mset (record));

  while (rec_mset_iterator_next (&iter, MSET_FIELD, (void *) &field, NULL))
    {
      double field_value_double = 0;
      const char *field_value = rec_field_value (field);

      if (rec_field_name_equal_p (rec_field_name (field), field_name)
          && rec_atod (field_value, &field_value_double))
        {
          avg = avg + field_value_double;
          num_fields++;
        }
    }
  rec_mset_iterator_free (&iter);

  if (num_fields != 0)
    avg = avg / num_fields;

  return avg;
}

#define REC_AGGREGATE_ACCUM_FUNC(NAME, OP, INIT_VAL)                    \
  static char *                                                         \
  rec_aggregate_std_##NAME (rec_rset_t rset,                            \
                            rec_record_t record,                        \
                            const char *field_name)                     \
  {                                                                     \
    char *result = NULL;                                                \
    double val   = INIT_VAL;                                            \
                                                                        \
    if (record)                                                         \
      val = rec_aggregate_std_##NAME##_record (record, field_name);     \
    else if (rset)                                                      \
      {                                                                 \
        rec_record_t rec = NULL;                                        \
        rec_mset_iterator_t iter = rec_mset_iterator (rec_rset_mset (rset)); \
        while (rec_mset_iterator_next (&iter, MSET_RECORD, (void *) &rec, NULL)) \
          val = OP (val, rec_aggregate_std_##NAME##_record (rec, field_name)); \
        rec_mset_iterator_free (&iter);                                 \
      }                                                                 \
                                                                        \
    /* Return the val as a string.  Note that if NULL is returned it */ \
    /* will be returned by this function below to signal the */         \
    /* end-of-memory condition.  */                                     \
    asprintf (&result, "%g", val);                                      \
                                                                        \
    return result;                                                      \
  }                                                                     \
                                                                        \
  static double                                                         \
  rec_aggregate_std_##NAME##_record (rec_record_t record,               \
                                     const char *field_name)            \
  {                                                                     \
    /* Calculate the val of the fields in a given record.  Fields */    \
    /* not representing a real value are ignored.  */                   \
                                                                        \
  double val = INIT_VAL;                                                \
  rec_field_t field;                                                    \
  rec_mset_iterator_t iter = rec_mset_iterator (rec_record_mset (record)); \
                                                                        \
  while (rec_mset_iterator_next (&iter, MSET_FIELD, (void *) &field, NULL)) \
    {                                                                   \
      const char *field_value = rec_field_value (field);                \
      double field_value_double = 0;                                    \
                                                                        \
      if (rec_field_name_equal_p (rec_field_name (field), field_name)   \
          && rec_atod (field_value, &field_value_double))               \
        val = OP (val, field_value_double);                             \
    }                                                                   \
  rec_mset_iterator_free (&iter);                                       \
                                                                        \
  return val;                                                           \
  }

/*
 * Aggregate: Sum(Field)
 */

static inline double
op_sum (double op1, double op2)
{
  return op1 + op2;
}

REC_AGGREGATE_ACCUM_FUNC(sum, op_sum, 0);

/*
 * Aggregate: Min(Field)
 * Aggregate: Max(Field)
 */

REC_AGGREGATE_ACCUM_FUNC(min, MIN, DBL_MAX);
REC_AGGREGATE_ACCUM_FUNC(max, MAX, DBL_MIN);

/* End of rec-aggregate.c */
