/* -*- mode: C -*-
 *
 *       File:         rec-fex-new.c
 *       Date:         Tue Nov  9 14:04:42 2010
 *
 *       GNU recutils - rec_fex_new unit tests.
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
#include <string.h>
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_fex_new_single
 * Unit: rec_fex_new
 * Description:
 * + Create field expressions composed by a single
 * + field.  It shall work for any kind of field name.
 */
START_TEST(rec_fex_new_single)
{
  rec_fex_t fex;
  const char *fname_foo;
  const char *fname_foobar;
  const char *fname_foobarbaz;

  fex = rec_fex_new (NULL, REC_FEX_SIMPLE);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 0);
  rec_fex_destroy (fex);

  fname_foo = "foo";
  fname_foobar = "foobar";
  fname_foobarbaz = "foobarbaz";

  fex = rec_fex_new ("foo", REC_FEX_SIMPLE);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 1);
  fail_if (!rec_field_name_equal_p (fname_foo,
                                    rec_fex_elem_field_name (rec_fex_get (fex, 0))));
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 0)) != -1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 0)) != -1);
  rec_fex_destroy (fex);

  fex = rec_fex_new ("foobarbaz", REC_FEX_SIMPLE);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 1);
  fail_if (!rec_field_name_equal_p (fname_foobarbaz,
                                    rec_fex_elem_field_name (rec_fex_get (fex, 0))));
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 0)) != -1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 0)) != -1);
  rec_fex_destroy (fex);
}
END_TEST

/*-
 * Test: rec_fex_new_simple
 * Unit: rec_fex_new
 * Description:
 * + Create field expressions of the SIMPLE variety.
 */
START_TEST(rec_fex_new_simple)
{
  rec_fex_t fex;
  const char *fname_foo;
  const char *fname_foobar;
  const char *fname_foobarbaz;

  fname_foo = "foo";
  fname_foobar = "foobar";
  fname_foobarbaz = "foobarbaz";

  fex = rec_fex_new ("foo foobar foobarbaz", REC_FEX_SIMPLE);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 3);
  fail_if (!rec_field_name_equal_p (fname_foo,
                                    rec_fex_elem_field_name (rec_fex_get (fex, 0))));
  fail_if (!rec_field_name_equal_p (fname_foobar,
                                    rec_fex_elem_field_name (rec_fex_get (fex, 1))));
  fail_if (!rec_field_name_equal_p (fname_foobarbaz,
                                    rec_fex_elem_field_name (rec_fex_get (fex, 2))));
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 0)) != -1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 0)) != -1);
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 1)) != -1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 1)) != -1);
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 2)) != -1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 2)) != -1);
  rec_fex_destroy (fex);

  fex = rec_fex_new ("\tfoo foobar\n ", REC_FEX_SIMPLE);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 2);
  fail_if (!rec_field_name_equal_p (fname_foo,
                                  rec_fex_elem_field_name (rec_fex_get (fex, 0))));
  fail_if (!rec_field_name_equal_p (fname_foobar,
                                  rec_fex_elem_field_name (rec_fex_get (fex, 1))));
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 0)) != -1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 0)) != -1);
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 1)) != -1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 1)) != -1);
  rec_fex_destroy (fex);

  fex = rec_fex_new ("foo\nfoobar\nfoobarbaz", REC_FEX_SIMPLE);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 3);
  fail_if (!rec_field_name_equal_p (fname_foo,
                                  rec_fex_elem_field_name (rec_fex_get (fex, 0))));
  fail_if (!rec_field_name_equal_p (fname_foobar,
                                  rec_fex_elem_field_name (rec_fex_get (fex, 1))));
  fail_if (!rec_field_name_equal_p (fname_foobarbaz,
                                  rec_fex_elem_field_name (rec_fex_get (fex, 2))));
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 0)) != -1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 0)) != -1);
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 1)) != -1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 1)) != -1);
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 2)) != -1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 2)) != -1);
  rec_fex_destroy (fex);

  fex = rec_fex_new ("\n\t        foo\nfoobarbaz\n", REC_FEX_SIMPLE);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 2);
  fail_if (!rec_field_name_equal_p (fname_foo,
                                  rec_fex_elem_field_name (rec_fex_get (fex, 0))));
  fail_if (!rec_field_name_equal_p (fname_foobarbaz,
                                  rec_fex_elem_field_name (rec_fex_get (fex, 1))));
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 0)) != -1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 0)) != -1);
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 1)) != -1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 1)) != -1);
  rec_fex_destroy (fex);
}
END_TEST

/*-
 * Test: rec_fex_new_csv
 * Unit: rec_fex_new
 * Description:
 * + Create field expressions of the CSV variety.
 */
START_TEST(rec_fex_new_csv)
{
  rec_fex_t fex;

  fex = rec_fex_new ("foo", REC_FEX_CSV);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 1);
  rec_fex_destroy (fex);

  fex = rec_fex_new ("foo,bar,baz", REC_FEX_CSV);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 3);
  rec_fex_destroy (fex);

  fex = rec_fex_new ("foo,bar", REC_FEX_CSV);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 2);
  rec_fex_destroy (fex);
}
END_TEST

/*-
 * Test: rec_fex_new_subscripts
 * Unit: rec_fex_new
 * Description:
 * + Create field expressions featuring subscripts.
 */
START_TEST(rec_fex_new_subscripts)
{
  rec_fex_t fex;
  const char *fname_foo;
  const char *fname_foobar;
  const char *fname_foobarbaz;

  fname_foo = "foo";
  fname_foobar = "foobar";
  fname_foobarbaz = "foobarbaz";

  fex = rec_fex_new ("foo", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 1);
  fail_if (!rec_field_name_equal_p (fname_foo,
                                    rec_fex_elem_field_name (rec_fex_get (fex, 0))));
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 0)) != -1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 0)) != -1);
  rec_fex_destroy (fex);

  fex = rec_fex_new ("foo[1]", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 1);
  fail_if (!rec_field_name_equal_p (fname_foo,
                                    rec_fex_elem_field_name (rec_fex_get (fex, 0))));
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 0)) != 1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 0)) != -1);
  rec_fex_destroy (fex);

  fex = rec_fex_new ("foo[1],foobar[0-23],foobarbaz", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 3);
  fail_if (!rec_field_name_equal_p (fname_foo,
                                    rec_fex_elem_field_name (rec_fex_get (fex, 0))));
  fail_if (!rec_field_name_equal_p (fname_foobar,
                                    rec_fex_elem_field_name (rec_fex_get (fex, 1))));
  fail_if (!rec_field_name_equal_p (fname_foobarbaz,
                                    rec_fex_elem_field_name (rec_fex_get (fex, 2))));
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 0)) != 1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 0)) != -1);
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 1)) != 0);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 1)) != 23);
  fail_if (rec_fex_elem_min (rec_fex_get (fex, 2)) != -1);
  fail_if (rec_fex_elem_max (rec_fex_get (fex, 2)) != -1);
  rec_fex_destroy (fex);
}
END_TEST

/*-
 * Test: rec_fex_new_invalid
 * Unit: rec_fex_new
 * Description:
 * + Try to create field expressions from an invalid string.
 * + The creation of the fex shall fail.
 */
START_TEST(rec_fex_new_invalid)
{
  rec_fex_t fex;

  fex = rec_fex_new ("", REC_FEX_SIMPLE);
  fail_if (fex != NULL);

  fex = rec_fex_new ("?", REC_FEX_SIMPLE);
  fail_if (fex != NULL);

  fex = rec_fex_new ("foo&", REC_FEX_SIMPLE);
  fail_if (fex != NULL);

  fex = rec_fex_new ("foo foo:bar", REC_FEX_SIMPLE);
  fail_if (fex != NULL);

  fex = rec_fex_new ("foo,bar,!,baz", REC_FEX_CSV);
  fail_if (fex != NULL);

  fex = rec_fex_new ("foo[]", REC_FEX_SUBSCRIPTS);
  fail_if (fex != NULL);

  fex = rec_fex_new ("foo[10 10]", REC_FEX_SUBSCRIPTS);
  fail_if (fex != NULL);

  fex = rec_fex_new ("bar[[10]", REC_FEX_SUBSCRIPTS);
  fail_if (fex != NULL);

  fex = rec_fex_new ("bar,foo[10]]", REC_FEX_SUBSCRIPTS);
  fail_if (fex != NULL);

  fex = rec_fex_new ("foo[10-]", REC_FEX_SUBSCRIPTS);
  fail_if (fex != NULL);

  fex = rec_fex_new ("foo[-]", REC_FEX_SUBSCRIPTS);
  fail_if (fex != NULL);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_fex_new (void)
{
  TCase *tc = tcase_create ("rec_fex_new");
  tcase_add_test (tc, rec_fex_new_single);
  tcase_add_test (tc, rec_fex_new_simple);
  tcase_add_test (tc, rec_fex_new_csv);
  tcase_add_test (tc, rec_fex_new_subscripts);
  tcase_add_test (tc, rec_fex_new_invalid);

  return tc;
}

/* End of rec-fex-new.c */
