/* -*- mode: C -*-
 *
 *       File:         rec-type-check.c
 *       Date:         Fri Oct 29 18:50:01 2010
 *
 *       GNU recutils - rec_type_check unit tests
 *
 */

/* Copyright (C) 2010-2015 Jose E. Marchesi */

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
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_type_check_int
 * Unit: rec_type_check
 * Description:
 * + Check strings of type int.
 */
START_TEST(rec_type_check_int)
{
  rec_type_t type;
  char *err_str;

  type = rec_type_new ("int");
  fail_if (type == NULL);

  /* Positive tests.  */
  fail_if (!rec_type_check (type, "10", NULL));
  fail_if (!rec_type_check (type, "01", NULL));
  fail_if (!rec_type_check (type, "-10", NULL));
  fail_if (!rec_type_check (type, "  10  \n\t ", NULL));

  /* Negative tests.  */
  fail_if (rec_type_check (type, "abc", NULL));
  fail_if (rec_type_check (type, "a10", NULL));
  fail_if (rec_type_check (type, "10a", &err_str));
  fail_if (err_str == NULL);
  free (err_str);

  rec_type_destroy (type);
}
END_TEST

/*-
 * Test: rec_type_check_bool
 * Unit: rec_type_check
 * Description:
 * + Check strings of type bool.
 */
START_TEST(rec_type_check_bool)
{
  rec_type_t type;

  type = rec_type_new ("bool");
  fail_if (type == NULL);

  /* Positive tests.  */
  fail_if (!rec_type_check (type, "true", NULL));
  fail_if (!rec_type_check (type, "false", NULL));
  fail_if (!rec_type_check (type, "1", NULL));
  fail_if (!rec_type_check (type, "0", NULL));
  fail_if (!rec_type_check (type, "yes", NULL));
  fail_if (!rec_type_check (type, "no", NULL));
  fail_if (!rec_type_check (type, "  yes  \n\t ", NULL));

  rec_type_destroy (type);
}
END_TEST

/*-
 * Test: rec_type_check_range
 * Unit: rec_type_check
 * Description:
 * + Check strings of type range.
 */
START_TEST(rec_type_check_range)
{
  rec_type_t type;

  type = rec_type_new ("range -10 10");
  fail_if (type == NULL);

  /* Positive tests.  */
  fail_if (!rec_type_check (type, "-10", NULL));
  fail_if (!rec_type_check (type, "10", NULL));
  fail_if (!rec_type_check (type, "010", NULL));
  fail_if (!rec_type_check (type, "5", NULL));
  fail_if (!rec_type_check (type, "  5   \n\t  ", NULL));

  /* Negative tests.  */
  fail_if (rec_type_check (type, "-100", NULL));
  fail_if (rec_type_check (type, "100", NULL));
  fail_if (rec_type_check (type, "-11", NULL));
  fail_if (rec_type_check (type, "11", NULL));
  fail_if (rec_type_check (type, "abc", NULL));

  rec_type_destroy (type);
}
END_TEST

/*-
 * Test: rec_type_check_real
 * Unit: rec_type_check
 * Description:
 * + Check strings of type real.
 */
START_TEST(rec_type_check_real)
{
  rec_type_t type;

  type = rec_type_new ("real");
  
  /* Positive tests.  */
  fail_if (!rec_type_check (type, "3", NULL));
  fail_if (!rec_type_check (type, "-3", NULL));
  fail_if (!rec_type_check (type, "03", NULL));
  fail_if (!rec_type_check (type, "3.14", NULL));
  fail_if (!rec_type_check (type, "-3.14", NULL));
  fail_if (!rec_type_check (type, ".10", NULL));
  fail_if (!rec_type_check (type, "-.10", NULL));
  fail_if (!rec_type_check (type, "  3.14  \n\t  ", NULL));

  /* Negative tests.  */
  fail_if (rec_type_check (type, "abc", NULL));
  fail_if (rec_type_check (type, "3.14a", NULL));
  
  rec_type_destroy (type);
}
END_TEST

/*-
 * Test: rec_type_check_size
 * Unit: rec_type_check
 * Description:
 * + Check strings of type size.
 */
START_TEST(rec_type_check_size)
{
  rec_type_t type;

  type = rec_type_new ("size 5");
  fail_if (type == NULL);

  /* Positive tests.  */
  fail_if (!rec_type_check (type, "", NULL));
  fail_if (!rec_type_check (type, "abcde", NULL));
  fail_if (!rec_type_check (type, "ab", NULL));

  /* Negative tests.  */
  fail_if (rec_type_check (type, " abcde ", NULL));
  fail_if (rec_type_check (type, "abcdef", NULL));
  
  rec_type_destroy (type);
}
END_TEST

/*-
 * Test: rec_type_check_line
 * Unit: rec_type_check
 * Description:
 * + Check strings of type line.
 */
START_TEST(rec_type_check_line)
{
  rec_type_t type;

  type = rec_type_new ("line");
  fail_if (type == NULL);

  /* Positive tests.  */
  fail_if (!rec_type_check (type, "", NULL));
  fail_if (!rec_type_check (type, "A line.", NULL));

  /* Negative tests.  */
  fail_if (rec_type_check (type, "\n", NULL));
  fail_if (rec_type_check (type, "several\nlines\n", NULL));

  rec_type_destroy (type);
}
END_TEST

/*-
 * Test: rec_type_check_regexp
 * Unit: rec_type_check
 * Description:
 * + Check strings of type regexp.
 */
START_TEST(rec_type_check_regexp)
{
  rec_type_t type;

  type = rec_type_new ("regexp /[abc][abc][abc]$/");
  fail_if (type == NULL);

  /* Positive tests.  */
  fail_if (!rec_type_check (type, "abc", NULL));

  /* Negative tests.  */
  fail_if (rec_type_check (type, "", NULL));
  fail_if (rec_type_check (type, "abcx", NULL));
  fail_if (rec_type_check (type, "abc ", NULL));

  rec_type_destroy (type);
}
END_TEST

/*-
 * Test: rec_type_check_date
 * Unit: rec_type_check
 * Description:
 * + Check strings of type date.
 */
START_TEST(rec_type_check_date)
{
  rec_type_t type;

  type = rec_type_new ("date");
  fail_if (type == NULL);

  /* Positive tests.  */
  fail_if (!rec_type_check (type, "13 August 1980", NULL));

  /* Negative tests.  */
  fail_if (rec_type_check (type, "", NULL));
  fail_if (rec_type_check (type, "1000 November -10", NULL));

  rec_type_destroy (type);
}
END_TEST

/*-
 * Test: rec_type_check_enum
 * Unit: rec_type_check
 * Description:
 * + Check strings of type enum.
 */
START_TEST(rec_type_check_enum)
{
  rec_type_t type;

  type = rec_type_new ("enum A B C");
  fail_if (type == NULL);

  /* Positive tests.  */
  fail_if (!rec_type_check (type, "A", NULL));
  fail_if (!rec_type_check (type, "B", NULL));
  fail_if (!rec_type_check (type, "C", NULL));
  fail_if (!rec_type_check (type, " B  \n\t  ", NULL));

  /* Negative tests.  */
  fail_if (rec_type_check (type, "A B", NULL));
  fail_if (rec_type_check (type, "XXX", NULL));
  fail_if (rec_type_check (type, "", NULL));
  fail_if (rec_type_check (type, " ", NULL));

  rec_type_destroy (type);
}
END_TEST

/*-
 * Test: rec_type_check_field
 * Unit: rec_type_check
 * Description:
 * + Check strings of type field.
 */
START_TEST(rec_type_check_field)
{
  rec_type_t type;

  type = rec_type_new ("field");
  fail_if (type == NULL);

  /* Positive tests.  */
  fail_if (!rec_type_check (type, "a", NULL));
  fail_if (!rec_type_check (type, "   ac \n\t ", NULL));

  /* Negative tests.  */
  fail_if (rec_type_check (type, "", NULL));
  fail_if (rec_type_check (type, " ", NULL));
  fail_if (rec_type_check (type, "a:", NULL));
  fail_if (rec_type_check (type, "a:b:", NULL));
  fail_if (rec_type_check (type, "a%:b:", NULL));

  rec_type_destroy (type);
}
END_TEST

/*-
 * Test: rec_type_check_email
 * Unit: rec_type_check
 * Description:
 * + Check strings of type email.
 */
START_TEST(rec_type_check_email)
{
  rec_type_t type;

  type = rec_type_new ("email");
  fail_if (type == NULL);

  /* Positive tests.  */
  fail_if (!rec_type_check (type, "jemarch@gnu.org", NULL));
  fail_if (!rec_type_check (type, " jemarch@gnu.org  \n\t ", NULL));

  /* Negative tests.  */
  fail_if (rec_type_check (type, "", NULL));
  fail_if (rec_type_check (type, "invalid@@email.com", NULL));

  rec_type_destroy (type);
}
END_TEST

/*
 * Test case creation function.
 */
TCase *
test_rec_type_check (void)
{
  TCase *tc = tcase_create ("rec_type_check");
  tcase_add_test (tc, rec_type_check_int);
  tcase_add_test (tc, rec_type_check_bool);
  tcase_add_test (tc, rec_type_check_range);
  tcase_add_test (tc, rec_type_check_real);
  tcase_add_test (tc, rec_type_check_size);
  tcase_add_test (tc, rec_type_check_line);
  tcase_add_test (tc, rec_type_check_regexp);
  tcase_add_test (tc, rec_type_check_date);
  tcase_add_test (tc, rec_type_check_enum);
  tcase_add_test (tc, rec_type_check_field);
  tcase_add_test (tc, rec_type_check_email);

  return tc;
}

/* End of rec-type-check.c */
