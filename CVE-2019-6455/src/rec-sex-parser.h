/* -*- mode: C -*-
 *
 *       File:         rec-sex-parser.h
 *       Date:         Tue Jan 12 17:59:45 2010
 *
 *       GNU recutils - Sexy parser
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


#ifndef REC_SEX_PARSER_H
#define REC_SEX_PARSER_H

#include <config.h>

#include <stdbool.h>

#include <rec-sex-ast.h>

typedef struct rec_sex_parser_s *rec_sex_parser_t;

/*
 * Public functions
 */

rec_sex_parser_t rec_sex_parser_new (void);
void rec_sex_parser_destroy (rec_sex_parser_t parser);

bool rec_sex_parser_case_insensitive (rec_sex_parser_t parser);
void rec_sex_parser_set_case_insensitive (rec_sex_parser_t parser,
                                          bool case_insensitive);
void rec_sex_parser_set_in (rec_sex_parser_t parser,
                            const char *str);
int rec_sex_parser_getc (rec_sex_parser_t parser);
rec_sex_ast_t rec_sex_parser_ast (rec_sex_parser_t parser);
void rec_sex_parser_set_ast (rec_sex_parser_t parser, rec_sex_ast_t ast);
bool rec_sex_parser_run (rec_sex_parser_t parser, const char *expr);
void *rec_sex_parser_scanner (rec_sex_parser_t parser);

void rec_sex_parser_print_ast (rec_sex_parser_t parser);

#endif /* rec-sex-parser.h */

/* End of rec-sex-parser.h */
