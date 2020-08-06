/* -*- mode: C -*-
 *
 *       File:         rec.h
 *       Date:         Fri Feb 27 20:04:59 2009
 *
 *       GNU recutils - Main Header
 *
 */

/* Copyright (C) 2009-2019 Jose E. Marchesi */

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

#ifndef GNU_REC_H
#define GNU_REC_H

#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>

/*
 * rec format version implemented by this library.
 */

#define REC_VERSION_MAJOR 1
#define REC_VERSION_MINOR 0
#define REC_VERSION_STRING "1.0"

/*
 * INITIALIZATION of the library
 */

void rec_init (void);
void rec_fini (void);

/*
 * HETEROGENEOUS ORDERED SETS (MULTI-SETS)
 *
 * Element types: A, B, C
 *
 *    type   value     next_A   next_B   next_C
 *   +-----+----------+-------+--------+--------+
 *   |     |          |       |        |        |
 *   +-----+----------+-------+--------+--------+
 *   .     .          .       .        .        .
 *   |     |          |       |        |        |
 *   +-----+----------+-------+--------+--------+
 */

/* Opaque data type representing a multi-set.  */

typedef struct rec_mset_s *rec_mset_t;

/* Opaque data type representing an element which is stored in the
   multi-set.  */

typedef struct rec_mset_elem_s *rec_mset_elem_t;

/* Structure to hold iterators in the stack.  Note that the inner
   structure must have the same structure than the gl_list_iterator_t
   structure in the internal (and not distributed) gl_list.h.  This
   structure must be keep up to date.  */

typedef struct
{
  void *vtable;
  void *list;
  size_t count;
  void *p; void *q;
  size_t i; size_t j;
} rec_mset_list_iter_t;

typedef struct
{
  rec_mset_t mset;
  rec_mset_list_iter_t list_iter;
} rec_mset_iterator_t;


/* Data types for the callbacks that can be registered in the
   multi-set and will be triggered to some events.  */

typedef void  (*rec_mset_disp_fn_t)    (void *data);
typedef bool  (*rec_mset_equal_fn_t)   (void *data1, void *data2);
typedef void *(*rec_mset_dup_fn_t)     (void *data);
typedef int   (*rec_mset_compare_fn_t) (void *data1, void *data2, int type2);


/* Data type representing an element type in a multi-set.  This type
   is assured to be a scalar and thus it is possible to use the
   comparison operators, but otherwise its contents must be
   opaque.  */

typedef int rec_mset_type_t;
#define MSET_ANY 0

/*************** Creating and destroying multi-sets *****************/

/* Create a new empty multi-set and return a reference to it.  NULL is
   returned if there is no enough memory to complete the
   operation.  */

rec_mset_t rec_mset_new (void);

/* Destroy a multi-set, freeing all used resources.  This disposes all
   the memory used by the mset internals, but not the data elements
   stored in the multi-set.  */

void rec_mset_destroy (rec_mset_t mset);

/* Create a copy of a multi-set and return a reference to it.  This
   operation performs a deep copy using the user-provided callback to
   duplicate the elements stored in the set.  NULL is returned if
   there is no enough memory to complete the operation.  */

rec_mset_t rec_mset_dup (rec_mset_t mset);

/*************** Registering Types in a multi-set *****************/

/* Return true if the multi-set has the specified TYPE registered.
   Return false otherwise.  Note that this function always returns
   true when TYPE is MSET_ANY.  */

bool rec_mset_type_p (rec_mset_t mset, rec_mset_type_t type);

/* Register a type in a multi-set.  NAME must be a NULL-terminated
   string with a unique name that will identify the type.  The
   provided callbacks will be called when needed.  This function
   returns an integer value that will identify the newly created type.
   The only assumption user code can make about this number is that it
   cant equal MSET_ANY.  */

rec_mset_type_t rec_mset_register_type (rec_mset_t            mset,
                                        char                 *name,
                                        rec_mset_disp_fn_t    disp_fn,
                                        rec_mset_equal_fn_t   equal_fn,
                                        rec_mset_dup_fn_t     dup_fn,
                                        rec_mset_compare_fn_t compare_fn);

/* Return the number of elements of the given type stored in a
   multi-set.  If TYPE is MSET_ANY then the total number of elements
   stored in the set is returned, regardless their type.  If the
   specified type does not exist in the multi-set then this function
   returns 0.  */

size_t rec_mset_count (rec_mset_t mset, rec_mset_type_t type);

/*************** Getting, inserting and removing elements **********/

/* Get the data stored at a specific position in a mset.  The returned
   data occupies the POSITIONth position in the internal list of
   elements of the specified type.  If there is no element stored at
   POSITION this function returns NULL.  */

void *rec_mset_get_at (rec_mset_t      mset,
                       rec_mset_type_t type,
                       size_t          position);

/* Create a new element at a specific position in a mset, storing a
   given data.  If POSITION is 0 then the element is prepended.  If
   POSITION is equal or bigger than the number of the existing
   elements with the same type in the mset then the new element is
   appended.  The function returns the newly created element, or NULL
   if there is not enough memory to perform the operation.  */

rec_mset_elem_t rec_mset_insert_at (rec_mset_t       mset,
                                    rec_mset_type_t  type,
                                    void            *data,
                                    size_t           position);

/* Insert some given data just after another element in a mset.  The
   function returns the newly created element, or NULL if there was no
   enough memory to perform the operation.  */

rec_mset_elem_t rec_mset_insert_after (rec_mset_t       mset,
                                       rec_mset_type_t  type,
                                       void            *data,
                                       rec_mset_elem_t  elem);

/* Append some given daata to a mset.  This is equivalent to call
   rec_mset_insert_at specifying a position equal or bigger than the
   number of the existing elements with type TYPE in the mset.  The
   function returns the newly created element, or NULL if there was no
   enough memory to perform the operation.  */

rec_mset_elem_t rec_mset_append (rec_mset_t       mset,
                                 rec_mset_type_t  elem_type,
                                 void            *data,
                                 rec_mset_type_t  type);

/* Add some given data to a mset.  The position where the new element
   is inserted depends on the sorting criteria implemented by the
   compare_fn callback for the element type.  The function returns the
   newly created element, or NULL if there was no enough memory to
   perform the operation.  */

rec_mset_elem_t rec_mset_add_sorted (rec_mset_t       mset,
                                     rec_mset_type_t  type,
                                     void            *data);

/* Remove the element occupying the specified position from a record
   set.  This function returns true if the element was removed, and
   false if there were no element stored at the specified
   position.  */

bool rec_mset_remove_at (rec_mset_t      mset,
                         rec_mset_type_t type,
                         size_t          position);

/* Remove an element from the multi-set.  The function returns true if
   the element was found in the list and removed.  */

bool rec_mset_remove_elem (rec_mset_t mset, rec_mset_elem_t elem);

/* Search for an element storing the specified data in a mset and
   return it.  NULL is returned in case no element in the record set
   is storing DATA.  */

rec_mset_elem_t rec_mset_search (rec_mset_t mset, void *data);

/*************** Iterating on mset elements *************************/

/* Create and return an iterator traversing elements in the multi-set.
   The mset contents must not be modified while the iterator is in
   use, except for replacing or removing the last returned
   element.  */

rec_mset_iterator_t rec_mset_iterator (rec_mset_t mset);

/* Advance the iterator to the next element of the given type.  The
   data stored by the next element is stored in *DATA if DATA is
   non-NULL and a reference to the element in *ELEM if ELEM is
   non-NULL.  The function returns true if there is a next element to
   which iterate to, and false otherwise.  */

bool rec_mset_iterator_next (rec_mset_iterator_t *iterator,
                             rec_mset_type_t type,
                             const void **data,
                             rec_mset_elem_t *elem);

/* Free an iterator.  */

void rec_mset_iterator_free (rec_mset_iterator_t *iterator);
                             
/*************** Managing mset elements ******************************/

/* Return the type of the given multi-set element.  Since every
   element must be of some concrete type, the returned value cannot be
   equal to MSET_ANY.  */

rec_mset_type_t rec_mset_elem_type (rec_mset_elem_t elem);

/* Set the type of the given multi-set element.  This function is
   useful to transform records into comments.  */

void rec_mset_elem_set_type (rec_mset_elem_t elem, rec_mset_type_t type);

/* Return a void pointer pointing to the data stored in the given mset
   element.  If no data was stored in the element then this function
   returns NULL.  */

void *rec_mset_elem_data (rec_mset_elem_t elem);

/* Set the data stored in a multi-set element.  The memory pointed by
   the previous value of the internal pointer is not freed or altered
   in any other way by this operation.  */

void rec_mset_elem_set_data (rec_mset_elem_t elem, void *data);

/* Determine whether the values stored in two multi-set elements are
   equal.  The comparison is performed using the user-provided
   compare_fn callback.  */

bool rec_mset_elem_equal_p (rec_mset_elem_t elem1, rec_mset_elem_t elem2);

/* Create a copy of the data stored in a mset element and return a
   reference to it. This uses the user-provided callback to duplicate
   the data.  NULL is returned if there is no enough memory to
   complete the operation.  */

void *rec_mset_elem_dup_data (rec_mset_elem_t elem);

/************** Sorting, grouping and other operations **************/

/* Sort a given multi-set using the compare_fn callbacks provided by
   the user when defining the types of the elements stored.  This is a
   destructive operation.  Returns a copy of the mset argument if the
   operation suceeded, NULL if there is not enough memory to perform
   the operation.  */

rec_mset_t rec_mset_sort (rec_mset_t mset);

/************************* Debugging ********************************/

/* Dump the contents of a multi-set to the terminal.  For debugging
   purposes.  */

void rec_mset_dump (rec_mset_t mset);

/*
 * FLEXIBLE BUFFERS
 *
 * A flexible buffer (rec_buf_t) is a buffer to which stream-like
 * operations can be applied.  Its size will grow as required.
 */

typedef struct rec_buf_s *rec_buf_t;

rec_buf_t rec_buf_new (char **data, size_t *size);
void rec_buf_close (rec_buf_t buffer);

/* rec_buf_putc returns the character written as an unsigned char cast
   to an int, or EOF on error.  */
int rec_buf_putc (int c, rec_buf_t buffer);
/* rec_buf_puts returns a non-negative number on success (number of
   characters written), or EOF on error.  */
int rec_buf_puts (const char *s, rec_buf_t buffer);

void rec_buf_rewind (rec_buf_t buf, int n);

/*
 * COMMENTS
 *
 * A comment is a block of text.  The printed representation of a
 * comment includes a sharp (#) character after each newline (\n)
 * character.
 */

typedef char *rec_comment_t;

/* Create a new comment and return it.  NULL is returned if there is
   not enough memory to perform the operation.  */

rec_comment_t rec_comment_new (char *text);

/* Destroy a comment, freeing all used resources.  */

void rec_comment_destroy (rec_comment_t comment);

/* Make a copy of the passed comment and return it.  NULL is returned
   if there is not enough memory to perform the operation.  */

rec_comment_t rec_comment_dup (rec_comment_t comment);

/* Return a string containing the text in the comment.  */

char *rec_comment_text (rec_comment_t comment);

/* Set the text of a comment.  Any previous text associated with the
   comment is destroyed and its memory freed.  */

void rec_comment_set_text (rec_comment_t *comment, char *text);

/* Determine whether the texts stored in two given comments are
   equal.  */

bool rec_comment_equal_p (rec_comment_t comment1, rec_comment_t comment2);

/* FIELD NAMES
 *
 */

/******************* Regexps for field names *******************/

#define REC_FNAME_RE "[a-zA-Z%][a-zA-Z0-9_]*"

#define REC_TYPE_NAME_RE "[a-zA-Z][a-zA-Z0-9_-]*"
#define REC_URL_REGEXP "(file|http|ftp|https)://[^ \t]+"
#define REC_FILE_REGEXP "(/?[^/ \t\n]+)+"

/******************* Field data types **************************/

/*
 * The following enumeration contains identifiers for the standard
 * fields used by the library.
 *
 * Changes to this enumerated value will require some fixes in
 * rec-field-name.c.
 */

enum rec_std_field_e
{ 
  REC_FIELD_AUTO = 0,
  REC_FIELD_CONFIDENTIAL,
  REC_FIELD_KEY,
  REC_FIELD_MANDATORY,
  REC_FIELD_PROHIBIT,
  REC_FIELD_REC,
  REC_FIELD_SIZE,
  REC_FIELD_SORT,
  REC_FIELD_TYPE,
  REC_FIELD_TYPEDEF,
  REC_FIELD_UNIQUE,
  REC_FIELD_CONSTRAINT,
  REC_FIELD_ALLOWED
};

/******************* Field name utilities **********************/

/* Determine whether two given strings contain the same field name.
   Note that this function does not check wheter the strings actually
   contain valid field names.  */

bool rec_field_name_equal_p (const char *name1, const char *name2);

/* Determine whether a given string is a correct field name.  */

bool rec_field_name_p (const char *str);

/* Normalise a field name.  Any non alphanumeric character, including
   '_', '-' and '%', are transformed into '_'.  This function returns
   NULL if there is not enough memory to perform the operation.  */

char *rec_field_name_normalise (const char *str);

/* Return the field name corresponding to a standard field, as defined
   above.  */

const char *rec_std_field_name (enum rec_std_field_e std_field);

/*
 * FIELD EXPRESSIONS
 *
 * A Field expression is composed by a sequence of "elements".  Each
 * element makes a reference to one or more fields in a record.
 */

/* Opaque data types for field expressions and the elements stored in
   them.  */

typedef struct rec_fex_s *rec_fex_t;
typedef struct rec_fex_elem_s *rec_fex_elem_t;

enum rec_fex_kind_e
{
  REC_FEX_SIMPLE,
  REC_FEX_CSV,
  REC_FEX_SUBSCRIPTS
};

/* Regular expressions matching written fexes.  */

#define REC_FNAME_FEX_RE      REC_FNAME_RE "(\\." REC_FNAME_RE ")?"
#define REC_FNAME_LIST_RE     REC_FNAME_RE "([ \n\t]+" REC_FNAME_RE ")*"
#define REC_FNAME_LIST_CS_RE  REC_FNAME_FEX_RE "(," REC_FNAME_FEX_RE ")*"
#define REC_FNAME_SUB_RE      REC_FNAME_FEX_RE "(\\[[0-9]+(-[0-9]+)?\\])?"
#define REC_FEX_FUNCTION_NAME "[a-zA-Z_][a-zA-Z0-9_]*"
#define REC_FEX_CALL          REC_FEX_FUNCTION_NAME "\\(" REC_FNAME_FEX_RE "\\)"
#define REC_FNAME_LIST_SUB_ELEM_RE "(" REC_FNAME_SUB_RE "|" REC_FEX_CALL ")" "(:" REC_FNAME_FEX_RE ")?"
#define REC_FNAME_LIST_SUB_RE REC_FNAME_LIST_SUB_ELEM_RE "(," REC_FNAME_LIST_SUB_ELEM_RE ")*"

/*********** Creating and destroying field expressions ************/

/* Parse and create a field expression, and return it.  A fex kind
   shall be specified in KIND.  If STR does not contain a valid FEX of
   the given kind then NULL is returned.  If there is not enough
   memory to perform the operation then NULL is returned.  If STR is
   NULL then an empty fex is returned.  */

rec_fex_t rec_fex_new (const char *str, enum rec_fex_kind_e kind);

/* Destroy a field expression, freeing any used resource. */

void rec_fex_destroy (rec_fex_t fex);

/* Create a copy of a given fex and return a reference to it.  If
   there is not enough memory to perform the operation then NULL is
   returned.  */

rec_fex_t rec_fex_dup (rec_fex_t fex);

/********* Getting and setting field expression properties **********/

/* Get the number of elements stored in a field expression.  */

size_t rec_fex_size (rec_fex_t fex);

/* Check whether a given field (or set of fields) identified by their
   name and indexes, are contained in a fex.  */

bool rec_fex_member_p (rec_fex_t fex, const char *fname, int min, int max);

/* Get the element of a field expression occupying the given position.
   If the position is invalid then NULL is returned.  */

rec_fex_elem_t rec_fex_get (rec_fex_t fex, size_t position);

/* Append an element at the end of the fex and return it.  This
   function returns NULL if there is not enough memory to perform the
   operation.  */

rec_fex_elem_t rec_fex_append (rec_fex_t fex, const char *fname,
                               int min, int max);

/* Determine whether all the elements of the given FEX are function
   calls.  */

bool rec_fex_all_calls_p (rec_fex_t fex);

/**************** Accessing field expression elements **************/

/* Return the name of the field(s) referred by a given fex
   element.  */

const char *rec_fex_elem_field_name (rec_fex_elem_t elem);

/* Set the name of the field(s) referred by a given fex element.  This
   function returns 'false' if there is not enough memory to perform
   the operation.  */

bool rec_fex_elem_set_field_name (rec_fex_elem_t elem, const char *fname);

/* Get the 'min' index associated with the field(s) referred by a
   given fex element.  */

int rec_fex_elem_min (rec_fex_elem_t elem);

/* Get the 'max' index associated with the field(s) referred by a
   given fex element.  Note that if the index is unused (the element
   refers to just one field) then -1 is returned.  */

int rec_fex_elem_max (rec_fex_elem_t elem);

/* Get the 'rewrite_to' field name associated with the field(s) referred
   by a given fex element.  If no rewrite rule was specified in the
   fex entry then NULL is returned.  */

const char *rec_fex_elem_rewrite_to (rec_fex_elem_t elem);

/* Get the function name associated with a given fex element.  If the
   fex entry is not a function call then NULL is returned.  */

const char *rec_fex_elem_function_name (rec_fex_elem_t elem);

/* Get the pointer to the context data to be used in the function
   call, if any.  */

void **rec_fex_elem_function_data (rec_fex_elem_t elem);

/*********** Miscellaneous field expressions functions ************/

/* Check whether a given string STR contains a proper fex description
   of type KIND.  */

bool rec_fex_check (const char *str, enum rec_fex_kind_e kind);

/* Sort the elements of a fex using the 'min' index of the elements as
   the sorting criteria.  */

void rec_fex_sort (rec_fex_t fex);

/* Get the written form of a field expression.  This function returns
   NULL if there is not enough memory to perform the operation.  */

char *rec_fex_str (rec_fex_t fex, enum rec_fex_kind_e kind);

/*
 * FIELD TYPES
 *
 */

enum rec_type_kind_e
  {
    /* Unrestricted.  */
    REC_TYPE_NONE = 0,
    /* An integer number.  */
    REC_TYPE_INT,
    /* A Boolean.  */
    REC_TYPE_BOOL,
    /* An integer number within a given range.  */
    REC_TYPE_RANGE,
    /* A real number.  */
    REC_TYPE_REAL,
    /* A string with a limitation on its size.  */
    REC_TYPE_SIZE,
    /* A line.  */
    REC_TYPE_LINE,
    /* A regexp.  */
    REC_TYPE_REGEXP,
    /* A date.  */
    REC_TYPE_DATE,
    /* An Enumeration.  */
    REC_TYPE_ENUM,
    /* A field name.  */
    REC_TYPE_FIELD,
    /* An email.  */
    REC_TYPE_EMAIL,
    /* An universally unique identifier (uuid).  */
    REC_TYPE_UUID,
    /* A foreign key.  */
    REC_TYPE_REC
  };

typedef struct rec_type_s *rec_type_t;

/* Create a new type based on the textual description in STR.  */
rec_type_t rec_type_new (const char *str);

/* Destroy a type.  */
void rec_type_destroy (rec_type_t type);

/* Determine whether a string contains a valid type description.  */
bool rec_type_descr_p (const char *str);

/* Get the kind of the type.  The _str version returns a string with
   the name of the type.  */
enum rec_type_kind_e rec_type_kind (rec_type_t type);
char *rec_type_kind_str (rec_type_t type);

/* Get the min and max parametes of a range type.  If the type does
   not define a range then -1 is returned.  */
int rec_type_min (rec_type_t type);
int rec_type_max (rec_type_t type);

/* Get the record set name of a rec type.  If the type does not define
   a rec then NULL is returned.  */
const char *rec_type_rec (rec_type_t type);

/* Get and set the name of a type.  Types are created anonymous by
   rec_type_new, so the getter will return NULL unless a name is
   set.  */
const char *rec_type_name (rec_type_t type);
void rec_type_set_name (rec_type_t type, const char *name);

/* Determine whether two types are the same type.
 *
 * Two types are equal if,
 *
 * - They are of the same kind, and
 *
 * - Depending on the kind of types:
 *
 *   + For sizes
 *
 *     The maximum size specified in both types is the same.
 *
 *   + For ranges
 *
 *     The ranges specified in both types are the same.
 *
 *   + For enums
 *
 *     Both enums have the same number of entries, they are identical
 *     and in the same order.
 *
 *   + For regexps
 *
 *     They are never equal.
 */
bool rec_type_equal_p (rec_type_t type1, rec_type_t type2);

/* Check the contents of a string against a type.  In case some error
   arises, return it in ERROR_STR if it is not NULL.  */

bool rec_type_check (rec_type_t type, const char *str, char **error_str);

/* Compare two values of a given type.  The comparison criteria will
   vary depending of the given type: numerical for ints and reals,
   time comparison for dates, etc.  If TYPE is NULL then a
   lexicographic comparison is performed.  Return -1 if VAL1 < VAL2, 0
   if VAL1 == VAL2 and 1 if VAL1 > VAL2.  */

int rec_type_values_cmp (rec_type_t type, const char *val1, const char *val2);

/*
 * TYPE REGISTRIES.
 *
 * Type registries are collections of named types. The following API
 * provides facilities to maintain type registries.
 */

typedef struct rec_type_reg_s *rec_type_reg_t;

/* Create and return an empty type registry.  NULL is returned if
   there is not enough memory to perform the operation.  */

rec_type_reg_t rec_type_reg_new (void);

/* Destroy a type registry, freeing resources.  */
void rec_type_reg_destroy (rec_type_reg_t reg);

/* Insert a new type in the type registry.  If a type with the same
   name already exists in the registry then it gets replaced.  */
void rec_type_reg_add (rec_type_reg_t reg, rec_type_t type);

/* Insert a new type in the type registry as a synonim of another
   type.  If a type with the same name already exists in the registry
   then it gets replaced.  */
void rec_type_reg_add_synonym (rec_type_reg_t reg, const char *type_name,
                               const char *to_name);

/* Get the type named TYPE_NAME stored in REG.  If it does not exist
   NULL is returned.  */
rec_type_t rec_type_reg_get (rec_type_reg_t reg, const char *type_name);

/*
 * FIELDS
 *
 * A field is an association between a label and a value.
 */

/* Opaque data type representing a field.  */

typedef struct rec_field_s *rec_field_t;

/*************** Creating and destroying fields *****************/

/* Create a new field and return a reference to it.  NULL is returned
   if there is no enough memory to perform the operation.  */

rec_field_t rec_field_new (const char *name, const char *value);

/* Destroy a field freeing all used resources.  This disposes all the
   memory used by the field internals.  */

void rec_field_destroy (rec_field_t field);

/* Create a copy of a field and return a reference to it.  This
   operation performs a deep copy of the contents of the field.  NULL
   is returned if there is no enough memory to perform the
   operation.  */

rec_field_t rec_field_dup (rec_field_t field);

/******************** Comparing fields  ****************************/

/* Determine wether two given fields are equal (i.e. they have equal
   names but possibly different values).  */

bool rec_field_equal_p (rec_field_t field1, rec_field_t field2);

/************ Getting and Setting field properties *****************/

/* Return a NULL terminated string containing the name of a field.
   Note that this function can't return the empty string for a
   properly initialized field.  */

const char *rec_field_name (rec_field_t field);

/* Set the name of a field.  This function returns 'false' if there is
   not enough memory to perform the operation.  */

bool rec_field_set_name (rec_field_t field, const char *name);

/* Return a NULL terminated string containing the value of a field,
   i.e. the string stored in the field.  The returned string may be
   empty if the field has no value, but never NULL.  */

const char *rec_field_value (rec_field_t field);

/* Set the value of a given field to the given string.  This function
   returns 'false' if there is not enough memory to perform the
   operation.  */

bool rec_field_set_value (rec_field_t field, const char *value);

/* Return a string describing the source of the field.  The specific
   meaning of the source depends on the user: it may be a file name,
   or something else.  This function returns NULL for a field for
   which a source was never set.  */

const char *rec_field_source (rec_field_t field);

/* Set a string describing the source of the field.  Any previous
   string associated to the field is destroyed and the memory it
   occupies is freed.  This function returns 'false' if there is not
   enough memory to perform the operation.  */

bool rec_field_set_source (rec_field_t field, const char *source);

/* Return an integer representing the location of the field within its
   source.  The specific meaning of the location depends on the user:
   it may be a line number, or something else.  This function returns
   0 for fields not having a defined source.  */

size_t rec_field_location (rec_field_t field);

/* Return the textual representation for the location of a field
   within its source.  This function returns NULL for fields not
   having a defined source.  */

const char *rec_field_location_str (rec_field_t field);

/* Set a number as the new location for the given field.  Any
   previously stored location is forgotten.  This function returns
   'false' if there is not enough memory to perform the operation.  */

bool rec_field_set_location (rec_field_t field, size_t location);

/* Return an integer representing the char location of the field
   within its source.  The specific meaning of the location depends on
   the user, usually being the offset in bytes since the beginning of
   a file or memory buffer.  This function returns 0 for fields not
   having a defined source.  */

size_t rec_field_char_location (rec_field_t field);

/* Return the textual representation for the char location of a field
   within its source.  This function returns NULL for fields not
   having a defined source.  */

const char *rec_field_char_location_str (rec_field_t field);

/* Set a number as the new char location for the given field.  Any
   previously stored char location is forgotten.  This function
   returns 'false' if there is not enough memory to perform the
   operation.  */

bool rec_field_set_char_location (rec_field_t field, size_t location);

/* Get/set the mark of a given field, which is an integer associated
   to the field ADT.  */

void rec_field_set_mark (rec_field_t field, int mark);
int rec_field_mark (rec_field_t field);

/********************* Transformations in fields ********************/

/* Get the textual representation of a field and make it a comment
   variable.  This function returns NULL if there is no enough memory
   to perform the operation.  */

rec_comment_t rec_field_to_comment (rec_field_t field);

/*
 * RECORDS
 *
 * A record is an ordered set of one or more fields intermixed with
 * comment blocks.
 */

/* Opaque data type representing a record.  */

typedef struct rec_record_s *rec_record_t;

/* Record mset types.  Note that the following constants are relying
   on the fact the multi-sets assign consecutive type ids starting
   with 1.  This is done this way for performance reasons, but it
   means that this constants must be ajusted in case the order in
   which the types are registered in rec_record_new changes.  */

#define MSET_FIELD   1
#define MSET_COMMENT 2

/*************** Creating and destroying records *****************/

/* Create a new empty record and return a reference to it.  NULL is
   returned if there is no enough memory to perform the operation.  */

rec_record_t rec_record_new (void);

/* Destroy a record, freeing all used resources.  This disposes all
   the memory used by the record internals, including any stored field
   or comment.  */

void rec_record_destroy (rec_record_t record);

/* Create a copy of a record and return a reference to it.  This
   operation performs a deep copy of the contained fields and
   comments.  NULL is returned if there is no enough memory to perform
   the operation.  */

rec_record_t rec_record_dup (rec_record_t record);

/******************** Comparing records  ***************************/

/* Determine whether a given record is a subset of another record.  A
   record 'A' is a subset of a record 'B' if and only if for every
   field or comment contained in 'A' there is an equivalent field or
   comment in 'B'.  The order of the elements is not relevant.  */

bool rec_record_subset_p (rec_record_t record1, rec_record_t record2);

/* Determine whether a given record is equal to another record.  A
   record 'A' is equal to a record 'B' if the 'A' is a subset of 'B'
   and 'B' is a subset of 'A'.  */

bool rec_record_equal_p (rec_record_t record1, rec_record_t record2);

/************ Getting and Setting record properties ****************/

/* Return the multi-set containing the elements stored by the given
   record.  */

rec_mset_t rec_record_mset (rec_record_t record);

/* Return the number of elements stored in the given record, of any
   type.  */

size_t rec_record_num_elems (rec_record_t record);

/* Return the number of fields stored in the given record.  */

size_t rec_record_num_fields (rec_record_t record);

/* Return the number of comments stored in the given record.  */

size_t rec_record_num_comments (rec_record_t record);

/* Return a string describing the source of the record.  The specific
   meaning of the source depends on the user: it may be a file name,
   or something else.  This function returns NULL for a record for
   which a source was never set.  */

char *rec_record_source (rec_record_t record);

/* Set a string describing the source of the record.  Any previous
   string associated to the record is destroyed and the memory it
   occupies is freed.  */

void rec_record_set_source (rec_record_t record, char *source);

/* Return an integer representing the location of the record within
   its source.  The specific meaning of the location depends on the
   user: it may be a line number, or something else.  This function
   returns 0 for records not having a defined source.  */

size_t rec_record_location (rec_record_t record);

/* Return the textual representation for the location of a record
   within its source.  This function returns NULL for records not
   having a defined source.  */

char *rec_record_location_str (rec_record_t record);

/* Set a number as the new location for the given record.  Any
   previously stored location is forgotten.  */

void rec_record_set_location (rec_record_t record, size_t location);

/* Return an integer representing the char location of the record
   within its source.  The specific meaning of the location depends on
   the user, usually being the offset in bytes since the beginning of
   a file or memory buffer.  This function returns 0 for records not
   having a defined source.  */

size_t rec_record_char_location (rec_record_t record);

/* Return the textual representation for the char location of a record
   within its source.  This function returns NULL for records not
   having a defined source.  */

char *rec_record_char_location_str (rec_record_t record);

/* Set a number as the new char location for the given record.  Any
   previously stored char location is forgotten.  */

void rec_record_set_char_location (rec_record_t record, size_t char_location);


/* Return the position occupied by the specified field in the
   specified records, not considering comments.  */

size_t rec_record_get_field_index (rec_record_t record, rec_field_t field);

/* Return the position occupied by the specified field in the
   specified record among the fields having the same name.  Thus, if
   the provided field is the first having its name in the record then
   the function returns 0.  If it is the third then the function
   returns 2.  */

size_t rec_record_get_field_index_by_name (rec_record_t record, rec_field_t field);

/* Determine whether a record contains some field whose value is STR.
   The string comparison can be either case-sensitive or
   case-insensitive.  */

bool rec_record_contains_value (rec_record_t record, const char *value, bool case_insensitive);

/* Determine whether a record contains a field whose name is
   FIELD_NAME and value FIELD_VALUE.  */

bool rec_record_contains_field (rec_record_t record, const char *field_name, const char *field_value);

/* Determine whether a given record contains a field named after a
   given field name.  */

bool rec_record_field_p (rec_record_t record, const char *field_name);

/* Return the number of fields name after a given field name stored in
   a record.  */

size_t rec_record_get_num_fields_by_name (rec_record_t record,
                                          const char *field_name);

/* Return the Nth field named after the given field name in a record.
   This function returns NULL if there is no such a field.  */

rec_field_t rec_record_get_field_by_name (rec_record_t record,
                                          const char *field_name,
                                          size_t n);

/* Remove the Nth field named after the given field name in a
   record.  */

void rec_record_remove_field_by_name (rec_record_t record,
                                      const char *field_name,
                                      size_t n);

/* Return the 'container pointer' of a record.  It is a pointer which
   is used by the user of the record.  This function returns NULL if
   no container pointer has been set in the record.  */

void *rec_record_container (rec_record_t record);

/* Set the 'container pointer' of a record, replacing any previous
   value.  */

void rec_record_set_container (rec_record_t record, void *container);

/********************* Transformations in records *******************/

/* Get the textual representation of a record and make it a comment
   variable.  This function returns NULL if there is no enough memory
   to perform the operation.  */

rec_comment_t rec_record_to_comment (rec_record_t record);

/* Remove duplicated fields in a given record.  Fields are compared by
   field name and value.  */

void rec_record_uniq (rec_record_t record);

/* Append two records.  This function adds all the fields in
   SRC_RECORD to DEST_RECORD.  */

void rec_record_append (rec_record_t dest_record, rec_record_t src_record);

/********************* Field Marks in records **********************/

/* Reset the marks of all fields in a given records, setting all the
   marks to 0.  */

void rec_record_reset_marks (rec_record_t record);

/* Set the mark of a given field.  Return true if the field is marked
   as desired.  Return false if the field is not stored in the
   record.  */

bool rec_record_mark_field (rec_record_t record, rec_field_t field, int mark);

/* Get the mark associated to a field in a record.  If the given field
   is not found in the record then return 0.  */

int rec_record_field_mark (rec_record_t record, rec_field_t field);

/*
 * RECORD SETS
 *
 * A record set is an ordered set of zero or more records and comments
 * maybe preceded by a record descriptor.
 */

#define REC_RECORD_TYPE_RE REC_FNAME_RE

/* Opaque data type representing a record set.  */

typedef struct rec_rset_s *rec_rset_t;

/* Opaque data type representing a selection expression.  This is
   placed here as a forward declaration.  See below in this file for
   the definition of the selection expressions stuff.  */

typedef struct rec_sex_s *rec_sex_t;

/* Record set mset types.  MSET_COMMENT is defined above.  */

#define MSET_RECORD 1

/************ Creating and destroying record sets **************/

/* Create a new empty record set and return a reference to it.  NULL
   is returned if there is no enough memory to perform the
   operation.  */

rec_rset_t rec_rset_new (void);

/* Destroy a record set, freeing all user resources.  This disposes
   all the memory used by the record internals, including any stored
   record or comment.  */

void rec_rset_destroy (rec_rset_t rset);

/* Create a copy of a record set and return a reference to it.  This
   operation performs a deep copy of the contained records and
   comments.  NULL is returned if there is no enough memory to perform
   the operation.  */

rec_rset_t rec_rset_dup (rec_rset_t rset);

/********* Getting and Setting record set properties *************/

/* Return the multi-set containing the elements stored by the given
   record set.  */

rec_mset_t rec_rset_mset (rec_rset_t rset);

/* Return the number of elements stored in the given record set, of
   any type.  */

size_t rec_rset_num_elems (rec_rset_t rset);

/* Return the number of records stored in the given record set.  */

size_t rec_rset_num_records (rec_rset_t rset);

/* Return the number of comments stored in the given record set.  */

size_t rec_rset_num_comments (rec_rset_t rset);

/***************** Record descriptor management ******************/

/* Return the record descriptor of a given record set.  NULL is
   returned if the record set does not feature a record
   descriptor.  */

rec_record_t rec_rset_descriptor (rec_rset_t rset);

/* Set a new record descriptor for a given record set.  If there was
   previously a record descriptor in the rset then it is destroyed.
   This function performs all the requires updates to the semantics
   associated with record sets, such as the type registry, size
   constraints, etc.  If RECORD is NULL then the record set wont
   feature a record descriptor.  */

void rec_rset_set_descriptor (rec_rset_t rset, rec_record_t record);

/* Return the relative position of the descriptor with respect the
   first element in the record set.  For example, if there are two
   comments before the record descriptor in the record set then this
   function returns 3.  */

size_t rec_rset_descriptor_pos (rec_rset_t rset);

/* Set the relative position of the descriptor with respect the first
   element in the record set.  See the documentation for
   rec_rset_descriptor_pos for details.  */

void rec_rset_set_descriptor_pos (rec_rset_t rset, size_t position);

/* Return the URL associated with a record set (external descriptor).
   NULL is returned if the record set does not feature a record
   descriptor, or if the record set is not featuring an external
   descriptor.  */

char *rec_rset_url  (rec_rset_t rset);

/* Return the type name of a record set.  NULL is returned if the
   record set does not feature a record descriptor.  */

char *rec_rset_type (rec_rset_t rset);

/* Set the type name of a record set.  If there was not a record
   descriptor in the rset then it is created with a single %rec field.
   In case there was an existing descriptor in the rset then it is
   updated to reflect the new name.  */

void rec_rset_set_type (rec_rset_t rset, const char *type);

/************ Management of the type registry ***********************/

/* Return the type registry of a record set.  Note that the registry
   will be empty for a newly created rset.  */

rec_type_reg_t rec_rset_get_type_reg (rec_rset_t rset);

/* Return the declared type for fields named after the provided field
   name in a record set.  NULL is returned if no such a type is
   found.  */

rec_type_t rec_rset_get_field_type (rec_rset_t rset,
                                    const char *field_name);

/********************** Size constraints ****************************/

/* Return the minimum number of records allowed for a rset in its
   record descriptor.  This is 0 for record sets for which no size
   constraints have been defined.  */

size_t rec_rset_min_records (rec_rset_t rset);

/* Return the maximum number of records allowed for a rset in its
   record descriptor.  This is SIZE_MAX for record sets for which no
   size constraints have been defined.  */

size_t rec_rset_max_records (rec_rset_t rset);

/********************** Sex constraints *****************************/

/* Return the number of sex constraints defined in a record set.  This
   is 0 for record sets for which no sex constraints have been
   defined.  */

size_t rec_rset_num_sex_constraints (rec_rset_t rset);

/* Return a given sex constraint defined in a record sex.  The
   provided index must be between 0 and the value returned by
   rec_rset_num_sex_constraints - 1.  */

rec_sex_t rec_rset_sex_constraint (rec_rset_t rset, size_t index);


/********************** Other functionality *************************/

/* Rename a field in a record descriptor.  Field names are not
   modified in the records themselves, but only in the record
   descriptor.  Note that the comparisons of the field names are
   EQL.  */

void rec_rset_rename_field (rec_rset_t rset,
                            const char *field_name,
                            const char *new_field_name);

/* Return a fex with the names of all the fields defined as
   auto-incremented fields in a record set.  */

rec_fex_t rec_rset_auto (rec_rset_t rset);

/* Return the name of the key field of the record set.  If the record
   set does not have a key defined then return NULL.  */

const char *rec_rset_key (rec_rset_t rset);

/* Return a fex with the names of all the fields defined as
   confidential fields in a record set.  */

rec_fex_t rec_rset_confidential (rec_rset_t rset);

/* Determine whether a given field name corresponds to a confidential
   field in a record set.  */

bool rec_rset_field_confidential_p (rec_rset_t rset, const char *field_name);

/* Return a string describing the source of the record set.  The
   specific meaning of the source depends on the user: it may be a
   file name, or something else.  This function returns NULL for a
   record set for which a source was never set.  */

char *rec_rset_source (rec_rset_t rset);

/* Set  an orderd  set of  of field  names that  will be  used as  the
   sorting  criteria for  a record  set.   The field  names will  take
   precedence to any other way to define the sorting criteria, such as
   the %sort  special field in  the record descriptor.   This function
   returns  'false' if  there  is  not enough  memory  to perform  the
   operation.  */

bool rec_rset_set_order_by_fields (rec_rset_t rset, rec_fex_t field_names);

/* Return the field names that are used to sort a record set.  */

rec_fex_t rec_rset_order_by_fields (rec_rset_t rset);

/* Sort a  record set.  The  SORT_BY parameter is  a fex that,  if non
   NULL, contains  the field names which  will be used as  the sorting
   criteria.  If no SORT_BY fields  is specified then whatever sorting
   criteria  specified in  the  record  set is  used.   If no  sorting
   criteria exists then the function is a no-op.  The function returns
   a copy of RSET or NULL if there is not enough memory to perform the
   operation.  */

rec_rset_t rec_rset_sort (rec_rset_t rset, rec_fex_t sort_by);

/* Group the records of a record  set by the given fields in GROUP_BY.
   The given  record set must be  sorted by GROUP_BY.  Note  that this
   function  uses the  first field  with the  given names  found in  a
   record, ignoring  any subsequent field.   It is  up to the  user to
   provide the right records in order to get the desired results.  The
   function returns  a copy of  RSET or NULL  if there was  not enough
   memory to perform the operation.  */

rec_rset_t rec_rset_group (rec_rset_t rset, rec_fex_t group_by);

/* Add missing auto fields defined in a record set to a given record.
   The record could not be stored in the record set used to determine
   which auto fields to add.  This function is a no-operation if the
   given record set is not defining any auto field, or if the passed
   record already contains all fields marked as auto in the record
   set.  The function returns a copy of RSET or NULL if there was not
   enough memory to perform the operation.  */

rec_rset_t rec_rset_add_auto_fields (rec_rset_t rset, rec_record_t record);

/*
 * DATABASES
 *
 * A database is an ordered set of zero or more record sets.
 */

/* Opaque type representing a database.  */

typedef struct rec_db_s *rec_db_t;

/* Opaque data type representing a registry of aggregates.  This is
   placed here as a forward declaration.  See below in this file for
   the description of field functions.  */

typedef struct rec_aggregate_reg_s *rec_aggregate_reg_t;

/************ Creating and destrying databases *********************/

/* Create a new empty database and return it.  This function returns
   NULL if there is not enough memory to perform the operation.  */

rec_db_t rec_db_new (void);

/* Destroy a database, freeing any used memory.  This means that all
   the record sets contained in the database are also destroyed.  */

void rec_db_destroy (rec_db_t db);

/*********** Getting and setting properties of databases **********/

/* Return the number of record sets contained in a given record
   set.  */

size_t rec_db_size (rec_db_t db);

/*********** Managing record sets in a database *******************/

/* Return the record set occupying the given position in the database.
   If no such record set is contained in the database then NULL is
   returned.  */

rec_rset_t rec_db_get_rset (rec_db_t db, size_t position);

/* Insert the given record set into the given database at the given
  position.  If POSITION >= rec_rset_size (DB), RSET is appended to
  the list of fields.  If POSITION < 0, RSET is prepended.  Otherwise
  RSET is inserted at the specified position.  If the rset is inserted
  then 'true' is returned. If there is an error then 'false' is
  returned.  */

bool rec_db_insert_rset (rec_db_t db, rec_rset_t rset, size_t position);

/* Remove the record set contained in the given position into the
   given database.  If POSITION >= rec_db_size (DB), the last record
   set is deleted.  If POSITION <= 0, the first record set is deleted.
   Otherwise the record set occupying the specified position is
   deleted.  If a record set has been removed then 'true' is returned.
   If there is an error or the database has no record sets 'false' is
   returned.  */

bool rec_db_remove_rset (rec_db_t db, size_t position);

/* Determine whether an rset named TYPE exists in a database.  If TYPE
   is NULL then it refers to the default record set.  */

bool rec_db_type_p (rec_db_t db, const char *type);

/* Get the rset with the given type from db.  This function returns
NULL if there is no a record set having that type.  */

rec_rset_t rec_db_get_rset_by_type (rec_db_t db, const char *type);

/******************** Miscellaneous database functions ****************/

/* Return the registry of aggregates of the given database.  */

rec_aggregate_reg_t rec_db_aggregates (rec_db_t db);

/******************** Database High-Level functions *******************/

/* Query for some data in a database.  The resulting data is returned
   in a record set.

   This function takes the following arguments:

   DB

      Database to query.

   TYPE 

      The type of records to query.  This string must identify a
      record set contained in the database.  If TYPE is NULL then the
      default record set, if any, is queried.

   JOIN
   
      If not NULL, this argument must be a string denoting a field
      name.  This field name must be a foreign key (field of type
      'rec') defined in the selected record set.  The query operation
      will do an inner join using T1.Field = T2.Field as join
      criteria.

   INDEX

      If not NULL, this argument is a pointer to a buffer containing
      pairs of Min,Max indexes, identifying intervals of valid
      records.  The list of ends with the pair
      REC_Q_NOINDEX,REC_Q_NOINDEX.

      INDEX is mutually exclusive with any other selection option.

   SEX

      Selection expression which is evaluated for every record in the
      referred record set.  If SEX is NULL then all records are
      selected.

      This argument is mutually exclusive with any other selection
      option.

   FAST_STRING

      If this argument is not NULL then it is a string which is used
      as a fixed pattern.  Records featuring fields containing
      FAST_STRING as a substring in their values are selected.

      This argument is mutually exclusive with any other selection
      option.

   RANDOM

      If not 0, this argument indicates the number of random records
      to select from the referred record set.
 
      This argument is mutually exclusive with any other selection
      option.

   FEX

      Field expression to apply to the matching records to build the
      records in the result record set.  If FEX is NULL then the
      matching records are unaltered.

   PASSWORD

      Password to use to decrypt confidential fields.  If the password
      does not work then the encrypted fields are returned as-is.  If
      PASSWORD is NULL, or if it is the empty string, then no attempt
      to decrypt encrypted fields will be performed.

   GROUP_BY

      If not NULL, group the record set by the given field names.
 
   SORT_BY

      If not NULL, sort the record set by the given field names.

   FLAGS

      ORed value of any of the following flags:

      REC_Q_DESCRIPTOR

      If set returned record set will feature a record descriptor.  If
      the query is involving a single record set then the descriptor
      will be a copy of the descriptor of the referred record set, and
      will feature the same record type name.  Otherwise it will be
      built from the several descriptors of the involved record sets,
      and the record type name will be formed concatenating the type
      names of the involved record sets.  If this flag is not
      activated then the returned record set won't feature a record
      descriptor.

      REC_Q_ICASE

      If set the string operations in the selection expression will be
      case-insensitive.  If FALSE any string operation will be
      case-sensitive.

  This function returns NULL if there is not enough memory to
  perform the operation.  */

#define REC_F_DESCRIPTOR 1
#define REC_F_ICASE      2
#define REC_F_UNIQ       4

#define REC_Q_NOINDEX ((size_t)-1)

rec_rset_t rec_db_query (rec_db_t     db,
                         const char  *type,
                         const char  *join,
                         size_t      *index,
                         rec_sex_t    sex,
                         const char  *fast_string,
                         size_t       random,
                         rec_fex_t    fex,
                         const char  *password,
                         rec_fex_t    group_by,
                         rec_fex_t    sort_by,
                         int          flags);

/* Insert a new record into a database, either appending it to some
   record set or replacing one or more existing records.

   This function takes the following arguments:
   
   DB

      Database where to insert the record.

   TYPE

      Type of the new record.  If there is an existing record set
      holding records of that type then the record is added to it.
      Otherwise a new record set is appended into the database.

   INDEX

      If not NULL, this argument is a pointer to a buffer containing
      pairs of Min,Max indexes, identifying intervals of records that
      will be replaced by copies of the provided record. The list of
      ends with the pair REC_Q_NOINDEX,REC_Q_NOINDEX.

      INDEX is mutually exclusive with any other selection option.

   SEX

      Selection expression which is evaluated for every record in the
      referred record set.  If SEX is NULL then all records are
      selected.

      This argument is mutually exclusive with any other selection
      option.

   FAST_STRING

      If this argument is not NULL then it is a string which is used
      as a fixed pattern.  Records featuring fields containing
      FAST_STRING as a substring in their values are selected.

      This argument is mutually exclusive with any other selection
      option.

   RANDOM

      If not 0, this argument indicates the number of random records
      to select from the referred record set.
 
      This argument is mutually exclusive with any other selection
      option.

   PASSWORD

      Password to use to crypt confidential fields.  If PASSWORD is
      NULL, or if it is the empty string, then no attempt to crypt
      confidential fields will be performed.

   RECORD

      Record to insert.  If more than one record is replaced in the
      database they will be substitued with copies of this record.

   FLAGS

      ORed value of any of the following flags:

      REC_F_ICASE

      If set the string operations in the selection expression will be
      case-insensitive.  If FALSE any string operation will be
      case-sensitive.

      REC_F_NOAUTO

      If set then no auto-fields will be added to the newly created
      records in the database.

   If no selection option is used then the new record is appended to
   either an existing record set identified by TYPE or to a newly
   created record set.  If some selection option is used then the
   matching existing records will be replaced.

   This function returns 'false' if there is not enough memory to
   perform the operation.  */

#define REC_F_NOAUTO 8

bool rec_db_insert (rec_db_t     db,
                    const char  *type,
                    size_t      *index,
                    rec_sex_t    sex,
                    const char  *fast_string,
                    size_t       random,
                    const char  *password,
                    rec_record_t record,
                    int flags);

/* Delete records from a database, either physically removing them or
   commenting them out.

   This function takes the following arguments:

   DB

      Database where to remove records.

   TYPE

      Type of the records to remove.

   INDEX

      If not NULL, this argument is a pointer to a buffer containing
      pairs of Min,Max indexes, identifying intervals of records that
      will be deleted or commented out. The list of ends with the pair
      REC_Q_NOINDEX,REC_Q_NOINDEX.

      INDEX is mutually exclusive with any other selection option.

   SEX

      Selection expression which is evaluated for every record in the
      referred record set.  If SEX is NULL then all records are
      selected.

      This argument is mutually exclusive with any other selection
      option.

   FAST_STRING

      If this argument is not NULL then it is a string which is used
      as a fixed pattern.  Records featuring fields containing
      FAST_STRING as a substring in their values are selected.

      This argument is mutually exclusive with any other selection
      option.
 
   RANDOM

      If not 0, this argument indicates the number of random records
      to select for deletion in the referred record set.
 
      This argument is mutually exclusive with any other selection
      option.

   FLAGS

      ORed value of any of the following flags:

      REC_F_ICASE

      If set the string operations in the selection expression will be
      case-insensitive.  If FALSE any string operation will be
      case-sensitive.

      REC_F_COMMENT_OUT

      If set the selected records will be commented out instead of physically
      removed from the database.

  This function returns 'false' if there is not enough memory to
  perform the operation.  */

#define REC_F_COMMENT_OUT 16

bool rec_db_delete (rec_db_t     db,
                    const char  *type,
                    size_t      *index,
                    rec_sex_t    sex,
                    const char  *fast_string,
                    size_t       random,
                    int          flags);

/* Manipulate the fields of the selected records in a database: remove
   them, set their values or rename them.

   This function takes the following arguments:

   DB

      Database where to set fields.

   TYPE

      Type of the records to act in.

   INDEX

      If not NULL, this argument is a pointer to a buffer containing
      pairs of Min,Max indexes, identifying intervals of records that
      will be deleted or commented out. The list of ends with the pair
      REC_Q_NOINDEX,REC_Q_NOINDEX.

      INDEX is mutually exclusive with any other selection option.

   SEX

      Selection expression which is evaluated for every record in the
      referred record set.  If SEX is NULL then all records are
      selected.

      This argument is mutually exclusive with any other selection
      option.

   FAST_STRING

      If this argument is not NULL then it is a string which is used
      as a fixed pattern.  Records featuring fields containing
      FAST_STRING as a substring in their values are selected.

      This argument is mutually exclusive with any other selection
      option.
 
   RANDOM

      If not 0, this argument indicates the number of random records
      to select for manipulation in the referred record set.
 
      This argument is mutually exclusive with any other selection
      option.
   
   FEX

      Field expression selecting the fields in the selected records
      which will be modified.

   ACTION

      Action to perform to the selected fields.  Valid values for this
      argument are:

      REC_SET_ACT_RENAME

      Rename the matching fields to the string pointed by ACTION_ARG.

      REC_SET_ACT_SET

      Set the value of the matching fields to the string pointed by
      ACTION_ARG.

      REC_SET_ACT_ADD

      Add new fields with the names specified in the fex to the
      selected records.  The new fields will have the string pointed
      by ACTION_ARG as their value.

      REC_SET_ACT_SETADD

      Set the selected fields to the value pointed by ACTION_ARG.  IF
      the fields dont exist then create them with that value.

      REC_SET_ACT_DELETE

      Delete the selected fields.  ACTION_ARG is ignored by this
      action.

      REC_SET_ACT_COMMENT

      Comment out the selected fields.  ACTION_ARG is ignored by this
      action.
      
   ACTION_ARG

      Argument to the selected action.  It is ok to pass NULL for
      actions which dont require an argument.

   FLAGS

      ORed value of any of the following flags:

      REC_F_ICASE

      If set the string operations in the selection expression will be
      case-insensitive.  If FALSE any string operation will be
      case-sensitive.

   This function return s'false' if there is not enough memory to
   perform the operation.
*/

#define REC_SET_ACT_NONE    0
#define REC_SET_ACT_RENAME  1
#define REC_SET_ACT_SET     2
#define REC_SET_ACT_ADD     3
#define REC_SET_ACT_SETADD  4
#define REC_SET_ACT_DELETE  5
#define REC_SET_ACT_COMMENT 6

bool rec_db_set (rec_db_t    db,
                 const char *type,
                 size_t     *index,
                 rec_sex_t   sex,
                 const char *fast_string,
                 size_t      random,
                 rec_fex_t   fex,
                 int         action,
                 const char *action_arg,
                 int         flags);

/*
 * INTEGRITY.
 *
 */

/* Check the integrity of all the record sets stored in a given
   database.  This function returns the number of errors found.
   Descriptive messages about the errors are appended to ERRORS.  */

int rec_int_check_db (rec_db_t db,
                      bool check_descriptors_p,
                      bool remote_descriptors_p,
                      rec_buf_t errors);

/* Check the integrity of a given record set.  This function returns
   the number of errors found.  Descriptive messages about the errors
   are appended to ERRORS.  */

int rec_int_check_rset (rec_db_t db,
                        rec_rset_t rset,
                        bool check_descriptor_p,
                        bool remote_descriptor_p,
                        rec_buf_t errors);

/* Check the integrity of a database provided ORIG_REC is replaced by
   REC.  This function returns the number of errors found.
   Descriptive messages about the errors are appended to ERRORS.  */

int rec_int_check_record (rec_db_t db,
                          rec_rset_t rset,
                          rec_record_t orig_rec,
                          rec_record_t rec,
                          rec_buf_t errors);

/* Check the type of a given field.  This function returns the number
   of errors found.  Descriptive messages about the errors are
   appended to ERRORS.  */

bool rec_int_check_field_type (rec_db_t db,
                               rec_rset_t rset,
                               rec_field_t field,
                               rec_buf_t errors);

/*
 * PARSER
 *
 * The rec parser provides functions to parse field, records and
 * entire record sets from a file stream or a memory buffer.
 */

/* Opaque data type representing a parser.  */

typedef struct rec_parser_s *rec_parser_t;

/**************** Creating and destroying parsers ******************/

/* Create a parser associated with a given file stream that will be
   used as the source for the tokens.  If not enough memory, return
   NULL. */

rec_parser_t rec_parser_new (FILE *in, const char *source);

/* Create a parser associated with a given buffer that will be used as
   the source for the tokens.  The buffer is of specified size and
   doesn't have to be null-terminated.  If not enough memory, return
   NULL.  */

rec_parser_t rec_parser_new_mem (const char *buffer, size_t size, const char *source);

/* Create a parser associated with a given null-terminated buffer that
   will be used as the source for the tokens.  If not enough memory,
   return NULL.  */

rec_parser_t rec_parser_new_str (const char *buffer, const char *source);

/* Destroy a parser, freeing all used resources.  Note that this call
   is not closing the associated file stream or the associated memory
   buffer.  */

void rec_parser_destroy (rec_parser_t parser);

/*********************** Parsing routines **************************/

/* Parse a field name and return it in FNAME.  This function returns
   'false' and the value in FNAME is undefined if a parse error is
   found.  */

bool rec_parse_field_name (rec_parser_t parser, char **fname);

/* Parse a field name from a string and return it.  This function
   returns NULL if a parse error is found.  */

char *rec_parse_field_name_str (const char *str);

/* Parse a field and return it in FIELD.  This function returns
   'false' and the value in FIELD is undefined if a parse error is
   found.  */

bool rec_parse_field (rec_parser_t parser, rec_field_t *field);

/* Parse a record and return it in RECORD.  This function returns
   'false' and the value in RECORD is undefined if a parse error is
   found.  */

bool rec_parse_record (rec_parser_t parser, rec_record_t *record);

/* Parse a record from a string and return it.  This function builds
   up an ephimeral parser internally in order to do the parsing.  This
   function returns NULL if a parse error is found.  */

rec_record_t rec_parse_record_str (const char *str);

/* Parse a record set and return it in RSET.  This function returns
   'false' and the value in RSET is undefined if a parse error is
   found.  */

bool rec_parse_rset (rec_parser_t parser, rec_rset_t *rset);

/* Parse a database and return it in DB.  This function returns
   'false' and the value in DB is undefined if a parse error is
   found.  */

bool rec_parse_db (rec_parser_t parser, rec_db_t *db);

/************ Getting and Setting properties of parsers *************/

/* Determine whether a given parser is in an EOF (end of file)
   state.  */

bool rec_parser_eof (rec_parser_t parser);

/* Determine whether a given parser is in an error state.  If the
   parser is in an error state then rec_parser_perror can be used to
   get a string describing the error.  */

bool rec_parser_error (rec_parser_t parser);

/* Reset the error status and EOF of a parser. */

void rec_parser_reset (rec_parser_t parser);

/* Print a message with details on the last parser error.  This
 * function produces a message on the standard error output,
 * describing the last error encountered while parsing.  First, if FMT
 * is not NULL, it is printed along with any remaining argument.  Then
 * a colon and a space are printed, and finally an error message
 * describing what went wrong.
 */

void rec_parser_perror (rec_parser_t parser, const char *fmt, ...);

/* Change the position in file of the parser to a given offset from
   the start of the input.  The line number is only used to store it
   in the parsed records.  Return 'false' on error, e.g. when the
   stream used is not seekable or when the position is outside the
   buffer. */
bool rec_parser_seek (rec_parser_t parser, size_t line_number, size_t position);

/* Return the current position in the file of the parser or -1 on error. */
long rec_parser_tell (rec_parser_t parser);

/*
 * WRITER
 *
 * The rec writer provides functions to generate the written form of
 * rec objects such as fields and records.
 */

/* Opaque data type representing a rec writer.  */

typedef struct rec_writer_s *rec_writer_t;

/* Enumerated value identifying the operation modes of the writers.
   The operation mode defines what kind of output the writer will
   generate.  */

enum rec_writer_mode_e
{
  REC_WRITER_NORMAL,  /* Generate output in rec format.  */
  REC_WRITER_VALUES,  /* Generate output in values format.  */
  REC_WRITER_VALUES_ROW, /* Generate output in row format.  */
  REC_WRITER_SEXP     /* Generate output in sexps.  */
};

typedef enum rec_writer_mode_e rec_writer_mode_t;

/**************** Creating and destroying writers ******************/

/* Create a writer associated with a given file stream.  If not enough
   memory, return NULL. */

rec_writer_t rec_writer_new (FILE *out);

/* Create a writer associated with a given string.  If not enough
   memory, return NULL.  */

rec_writer_t rec_writer_new_str (char **str, size_t *str_size);

/* Destroy a writer, freeing any used resources.  Note that this call
   is not closing the associated file stream, nor free the associated
   string.  */

void rec_writer_destroy (rec_writer_t writer);

/************ Getting and setting writer properties ***************/

/* Set whether the writer must "collapse" the output records when
   writing record sets.  */

void rec_writer_set_collapse (rec_writer_t writer, bool value);

/* Set whether the writer must skip comments when outputting record
   sets and records.  */

void rec_writer_set_skip_comments (rec_writer_t writer, bool value);

/* Set the operation mode of the writer.  See the enumerated type
   defined above for a list of allowed modes.  Note that the mode can
   be changed at any time.  */

void rec_writer_set_mode (rec_writer_t writer, enum rec_writer_mode_e mode);

/************** Getting the properties of a writer ****************/

/* Determine whether a given writer is in an EOF (end-of-file)
   state.  */

bool rec_writer_eof (rec_writer_t writer);

/********************** Writing routines **************************/

/* Write a string in the given writer.  This function returns 'false'
   if there was an EOF condition.  */

bool rec_write_string (rec_writer_t writer, const char *str);

/* Write a comment in the given writer.  This function returns 'false'
   if there was an EOF condition.  */

bool rec_write_comment (rec_writer_t writer, rec_comment_t comment);

/* Write a field name in the given writer.  This function returns
   'false' if there was an EOF condition.  */

bool rec_write_field_name (rec_writer_t writer, const char *field_name);

/* Write a field in the given writer.  If NAME is not NULL, use it
   instead of the proper name of the field.  This function returns
   'false' if there was an EOF condition.  */

bool rec_write_field (rec_writer_t writer, rec_field_t field);

/* Write a record in the given writer.  This function returns 'false'
   if there was an EOF condition.  */

bool rec_write_record (rec_writer_t writer, rec_record_t record);

/* Write a record set to the given writer.  This function returns
   'false' if there was an EOF condition.  */

bool rec_write_rset (rec_writer_t writer, rec_rset_t rset);

/* Write a database to the given writer.  This function returns
   'false' if there was an EOF condition.  */

bool rec_write_db (rec_writer_t writer, rec_db_t db);

/* Create a string with the written representation of a field name and
   return it.  This function returns NULL if there is not enough
   memory to perform the operation.  */

char *rec_write_field_name_str (const char *field_name, rec_writer_mode_t mode);

/* Create a string with the written representation of a field and
   return it.  This function returns NULL if there is not enough
   memory to perform the operation.  */

char *rec_write_field_str (rec_field_t field, rec_writer_mode_t mode);

/* Create a string with the written representation of a comment and
   return it.  This function returns NULL if there is not enough
   memory to perform the operation.  */

char *rec_write_comment_str (rec_comment_t comment, rec_writer_mode_t mode);

/*
 * SELECTION EXPRESSIONS
 *
 * A selection expression is an expression that can be applied to a
 * record.  The result of the evaluation is a boolean value indicating
 * whether the record matches the expression.
 *
 * The abbreviated term to refer to a selection expression is 'a sex'.
 * The plural form is 'sexes'.
 */

/* The opaque type rec_sex_t is defined above in this file as a
   forward declaration.  */

/**************** Creating and destroying sexes ******************/

/* Create a new selection expression and return it.  If there is not
   enough memory to create the sex, then return NULL.  */

rec_sex_t rec_sex_new (bool case_insensitive);

/* Destroy a sex, freeing any used resources.  */

void rec_sex_destroy (rec_sex_t sex);

/**************** Compiling and applying sexes ******************/

/* Compile a sex.  Sexes must be compiled before being used.  If there
   is a parse error return false.  */

bool rec_sex_compile (rec_sex_t sex, const char *expr);

/* Apply a sex expression to a record, setting STATUS in accordance:
   'true' if the record matched the sex, 'false' otherwise.  The
   function returns the same value that is stored in STATUS.  */

bool rec_sex_eval (rec_sex_t sex, rec_record_t record, bool *status);

/* Apply a sex expression and get the result as an allocated
   string.  */

char *rec_sex_eval_str (rec_sex_t sex, rec_record_t record);

/**************** Miscellaneous sexes functions ******************/

/* Print the abstract syntax tree of a compiled sex.  This function is
   intended to be used for debugging purposes.  */

void rec_sex_print_ast (rec_sex_t sex);


/*
 * ENCRYPTION
 *
 * The following routines encrypt and decrypt fields in rec data.
 *
 * If librec was built without encryption support, all of them will do
 * nothing and return 'false' as if an error occurred.
 */

/* Prefix used in the encrypted and ASCII encoded field values, which
   is used to recognize encrypted values.  */

#define REC_ENCRYPTED_PREFIX "encrypted-"

/**************** Encryption routines *******************************/

/* Encrypt a given buffer and place the encrypted data in an allocated
   output buffer.  This function returns 'false' if there was an error
   while performing the encryption, such as an incorrect password or
   an out-of-memory condition.  */

bool rec_encrypt (char *in, size_t in_size, const char *password,
                  char **out, size_t *out_size);


/* Encrypt and ASCII-encode the value of a field used the provided
   password.  The REC_ENCRYPTED_PREFIX is prepended to the result.
   This function returns 'false' if there was an error while
   performing the encryption, such as an incorrect password or an
   out-of-memory condition.  */

bool rec_encrypt_field (rec_field_t field, const char *password);

/* Encrypt and ASCII-encode the fields of a record marked as
   "confidential" in a given record set, using the provided password.
   The REC_ENCRYPTED_PREFIX is prepended to the result.  This function
   returns 'false' if there was an error while performing the
   encryption, such as an incorrect password or an out-of-memory
   condition.  */

bool rec_encrypt_record (rec_rset_t rset, rec_record_t record,
                           const char *password);

/**************** Decryption routines *******************************/

/* Decrypt a given buffer and place the decrypted data in an allocated
   output buffer.  This function returns 'false' if there was an error
   while performing the decryption, such as an incorrect password or
   an out-of-memory condition.  */

bool rec_decrypt (char *in, size_t in_size, const char *password,
                  char **out, size_t *out_size);

/* Decrypt the value of a field used the provided password.  This
   function returns 'false' if there was an error while perfoming the
   decryption, such as an incorrect password or an out-of-memory
   condition.  Note that this function uses the REC_ENCRYPTED_PREFIX
   in order to determine whether a field is already encrypted.  */

bool rec_decrypt_field (rec_field_t field, const char *password);

/* Decrypt the encrypted fields of a record marked as "confidential"
   in a given record set, using the provided password.  This function
   returns 'false' if there was an error while performing the
   decryption, such as an incorrect password or an out-of-memory
   condition.  Note that this function uses the REC_ENCRYPTED_PREFIX
   in order to determine whether a field is already encrypted.  */

bool rec_decrypt_record (rec_rset_t rset, rec_record_t record,
                         const char *password);

/*
 * AGGREGATES
 *
 * The following routines and data types provide support for
 * "aggregate functions".  Aggregate functions are applied to sets of
 * fields and return a single value.
 */

/* Data type representing an aggregate function.  Aggregate functions
   get a record set, a record, a field name and subscripts which can
   be -1.  The field functions must return a string, which may be
   empty.  If an out-of-memory condition occurs then they return
   NULL.  */

typedef char *(*rec_aggregate_t) (rec_rset_t    rset,
                                  rec_record_t  record,
                                  const char   *field_name);

/*
 * AGGREGATES REGISTRIES
 *
 * The following data types and functions provide support for
 * maintaining registries with collections of aggregate functions.
 * The aggregate functions are identified by a constant string that
 * must be unique in the registry.
 */

/* See the definition of rec_aggregate_reg_t above.  */

/******** Creating and destroying function registries ************/

/* Create a new, empty aggregates registry.  If there is not error to
   perform the operation then NULL is returned.  */

rec_aggregate_reg_t rec_aggregate_reg_new (void);

/* Destroy a functions registry, freeing any used resources.  */

void rec_aggregate_reg_destroy (rec_aggregate_reg_t func_reg);

/********* Registering functions and fetching them ***************/

/* Register a field function into a functions register, associating it
   with a given constant string.  If a function associated with the
   given string already exists in the registry then it is substitued
   by the provided function.  The function true if the operation was
   successful, and false if there was not enough memory to perform the
   operation.  */

bool rec_aggregate_reg_add (rec_aggregate_reg_t func_reg, const char *name, rec_aggregate_t function);

/* Fetch a field function from a functions registry.  If no function
   associated with NAME is found in the registry then NULL is
   returned.  */

rec_aggregate_t rec_aggregate_reg_get (rec_aggregate_reg_t func_get, const char *name);

/* Register the standard built-in functions shipped with librec in the
   given aggregate register.  */

void rec_aggregate_reg_add_standard (rec_aggregate_reg_t func_reg);

/* Determine whether a given function name is a standard aggregate.
   The comparison is case-insensitive.  */

bool rec_aggregate_std_p (const char *name);

#endif /* !GNU_REC_H */

/* End of rec.h */
