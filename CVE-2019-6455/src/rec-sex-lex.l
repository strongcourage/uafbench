/* -*- mode: C -*-
 *
 *       File:         rec-sex.l
 *       Date:         Sat Jan  9 16:35:18 2010
 *
 *       GNU recutils - Selection Expressions lexer
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

/* Automake needs this.  */
%option outfile="lex.yy.c"
%option prefix="sex"
%option pointer
%option noyywrap
%option reentrant
%option bison-bridge
%option extra-type="void *"

%option header-file="rec-sex-lex.h"
%option nounput
%option noinput
%top {
   /* This code goes at the "top" of the generated file.  */
   #include <config.h>
}

%{
#include <rec-utils.h>

#include <rec-sex-ast.h>
#include <rec-sex-parser.h>
#include <rec-sex-tab.h>

#define YY_INPUT(buf,result,max_size)                                   \
   {                                                                    \
     int ci;                                                            \
     if ((ci = rec_sex_parser_getc ((rec_sex_parser_t) yyextra)) == -1)     \
       {                                                                \
         result = YY_NULL;                                              \
       }                                                                \
     else                                                               \
       {                                                                \
         buf[0] = (char) ci;                                            \
         result = 1;                                                    \
     }                                                                  \
   }

   /* Forward prototypes for functions defined below.  */
   char *rec_sex_lex_extract_name (char *str);
   char *rec_sex_lex_extract_subname (char *str);
   bool rec_sex_lex_extract_index (char *str, int *num);

%}

NEWLINE            \n
BLANK              [ \t\n]
DIGIT              [0-9]
LETTER             [a-zA-Z]
FIELD_NAME         {LETTER}[a-zA-Z0-9_]*
STRING             '([^']|\\(.|\n))*'|\"([^"]|\\(.|\n))*\"

/* Please do not touch this comment' */

%%

{BLANK}  { /* Ignore whitespace */ }

"-"?{DIGIT}*\.{DIGIT}+ {
  /*
   * Create a real node.
   */
  double real_value;
  
  yylval->node = rec_sex_ast_node_new ();
  rec_atod (yytext, &real_value);
  rec_sex_ast_node_set_real (yylval->node, real_value);

  return REC_SEX_TOK_REAL;
}

"-"?((0x[0-9a-fA-F]+)|{DIGIT}+) {
  /*
   * Create an integer node.
   */
  int integer_value;

  yylval->node = rec_sex_ast_node_new ();
  rec_atoi (yytext, &integer_value);
  rec_sex_ast_node_set_int (yylval->node, integer_value);

  return REC_SEX_TOK_INT;    
}

"+"      { return REC_SEX_TOK_ADD; }
"-"      { return REC_SEX_TOK_SUB; }
"/"      { return REC_SEX_TOK_DIV; }
"%"      { return REC_SEX_TOK_MOD; }
">>"     { return REC_SEX_TOK_AFTER; }
"<<"     { return REC_SEX_TOK_BEFORE; }
"<="     { return REC_SEX_TOK_LTE; }
">="     { return REC_SEX_TOK_GTE; }
"=="     { return REC_SEX_TOK_SAMETIME; }
"=>"     { return REC_SEX_TOK_IMPLIES; }
"*"      { return REC_SEX_TOK_MUL; }
"="      { return REC_SEX_TOK_EQL; }
"!="     { return REC_SEX_TOK_NEQ; }
"!"      { return REC_SEX_TOK_NOT; }
"~"      { return REC_SEX_TOK_MAT; }
">"      { return REC_SEX_TOK_GT; }
"<"      { return REC_SEX_TOK_LT; }
"("      { return REC_SEX_TOK_BP; }
")"      { return REC_SEX_TOK_EP; }
"&&"     { return REC_SEX_TOK_AND; }
"||"     { return REC_SEX_TOK_OR; }
"#"      { return REC_SEX_TOK_SHARP; }
"?"      { return REC_SEX_TOK_QM; }
":"      { return REC_SEX_TOK_COLON; }
"&"      { return REC_SEX_TOK_AMP; }

{FIELD_NAME}(\.{FIELD_NAME})?(\[[0-9]+\])? {
  int res;
  char *match;
  char *name, *subname;
  int index = -1;

  match = strdup (yytext);
  rec_sex_lex_extract_index (match, &index);
  name = rec_sex_lex_extract_name (match);
  subname = rec_sex_lex_extract_subname (match);

  /* Create a name node.  */
  yylval->node = rec_sex_ast_node_new ();
  rec_sex_ast_node_set_name (yylval->node, name, subname);
  rec_sex_ast_node_set_index (yylval->node, index);
  res = REC_SEX_TOK_NAM;

  free (name);
  free (match);

  return res;
}

{STRING} {
  /*
   * Create a string node.
   */
  
  /* Strip the quoting characters */
  yytext[strlen(yytext) - 1] = 0;

  yylval->node = rec_sex_ast_node_new ();
  rec_sex_ast_node_set_str (yylval->node, yytext + 1);

  return REC_SEX_TOK_STR;
}

.        { return REC_SEX_TOK_ERR; }

%%

char *
rec_sex_lex_extract_name (char *str)
{
  size_t size;
  char *res;
  char *p;

  p = str;
  while ((*p != '[') && (*p != '.') && (*p != 0))
    {
      p++;
    }

  size = p - str;
  res = malloc (size + 1);
  memcpy (res, str, size);
  res[size] = 0;

  return res;
}

char *
rec_sex_lex_extract_subname (char *str)
{
  char *p;

  /* If there is not a subname denoted in STR then this function
     returns NULL.  */
  
  p = str;
  while ((*p != '.') && (*p != '\0'))
    {
      p++;
    }

  if (*p == '\0')
    {
      /* There is not a second name.  */
      return NULL;
    }

  p++;  /* Skip the dot separator.  */

  return rec_sex_lex_extract_name (p);
}

bool
rec_sex_lex_extract_index (char *str,
                           int *num)
{
  bool res;
  char *p;
  char aux[100];
  int aux_size;

  /* Note that this function assumes syntax correctness in STR if a
     '[' is found.  */  

  aux_size = 0;
  p = str;
  while ((*p != 0) && (*p != '['))
    {
      p++;
    }

  if (*p == 0)
    {
      res = false;
    }
  else
    {
      p++; /* Pass the [.  */
      while (*p != ']')
        {
          aux[aux_size++] = *p;
          p++;
        }
      aux[aux_size] = 0;

      if (!rec_atoi (aux, num))
        {
          res = false;
        }

      res = true;
    }

  return res;
}

/* End of rec-sex.l */
