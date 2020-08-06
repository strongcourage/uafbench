/* -*- mode: C -*-
 *
 *       File:         mdb2rec.c
 *       Date:         Fri Aug 20 22:46:31 2010
 *
 *       GNU recutils - mdb to rec converter.
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

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <xalloc.h>
#include <gettext.h>
#define _(str) gettext (str)

#include <glib.h>
#include <mdbtools.h>

#include <rec.h>
#include <recutl.h>

/* Forward declarations. */
static void parse_args (int argc, char **argv);
static rec_db_t process_mdb (void);
static rec_rset_t process_table (MdbCatalogEntry *entry);
static char *get_field_name (MdbHandle *mdb, const char *table_name, const char *col_name);
static void get_relationships (MdbHandle *mdb);

/*
 * Data types
 */

struct relationship_s
{
  char *table;
  char *column;
  char *referenced_table;
  char *referenced_column;
};

/*
 * Global variables
 */

char *mdb2rec_mdb_file = NULL;
char *mdb2rec_mdb_table = NULL;
bool mdb2rec_include_system = false;
bool mdb2rec_keep_empty_fields = false;
bool mdb2rec_list_tables = false;
struct relationship_s *relationships;
size_t num_relationships = 0;

/*
 * Command line options management
 */

enum
  {
    COMMON_ARGS,
    SYSTEM_TABLES_ARG,
    KEEP_EMPTY_FIELDS_ARG,
    LIST_TABLES_ARG
  };

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    {"system-tables", no_argument, NULL, SYSTEM_TABLES_ARG},
    {"keep-empty-fields", no_argument, NULL, KEEP_EMPTY_FIELDS_ARG},
    {"list-tables", no_argument, NULL, LIST_TABLES_ARG},
    {NULL, 0, NULL, 0}
  };

/*
 * Functions.
 */

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, mdb2rec synopsis.
     no-wrap */
  printf (_("\
Usage: mdb2rec [OPTIONS]... MDB_FILE [TABLE]\n"));

  /* TRANSLATORS: --help output, mdb2rec short description.
     no-wrap */
  fputs (_("\
Convert an mdb file into a rec file.\n"), stdout);

  puts ("");
  /* TRANSLATORS: --help output, mdb2rec options.
     no-wrap */
  fputs (_("\
  -s, --system-tables                 include system tables.\n\
  -e, --keep-empty-fields             don't prune empty fields in the rec\n\
                                        output.\n\
  -l, --list-tables                   dump a list of the table names contained\n\
                                        in the mdb file.\n"),
         stdout);

  recutl_print_help_common ();
  puts ("");
  recutl_print_help_footer ();
}

static void
parse_args (int argc,
            char **argv)
{
  int ret;
  char c;

  while ((ret = getopt_long (argc,
                             argv,
                             "sel",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          COMMON_ARGS_CASES
        case SYSTEM_TABLES_ARG:
        case 's':
          {
            mdb2rec_include_system = true;
            break;
          }
        case KEEP_EMPTY_FIELDS_ARG:
        case 'e':
          {
            mdb2rec_keep_empty_fields = true;
            break;
          }
        case LIST_TABLES_ARG:
        case 'l':
          {
            mdb2rec_list_tables = true;
            break;
          }
        default:
          {
            exit (EXIT_FAILURE);
          }
        }
    }

  /* Read the name of the mdb file.  */
  if ((argc - optind) > 2)
    {
      recutl_print_help ();
      exit (EXIT_FAILURE);
    }
  else
    {
      mdb2rec_mdb_file = argv[optind++];

      if ((argc - optind) > 0)
        {
          mdb2rec_mdb_table = argv[optind++];
        }
    }
}

static void
get_relationships (MdbHandle *mdb)
{
  char *bound[4];
  MdbTableDef *table;
  size_t i;

  table = mdb_read_table_by_name (mdb,
                                  "MsysRelationships",
                                  MDB_TABLE);
  if ((!table) || (table->num_rows == 0))
    {
      return;
    }

  mdb_read_columns (table);
  for (i = 0; i < 4; i++)
    {
      bound[i] = xmalloc (MDB_BIND_SIZE);
    }

  mdb_bind_column_by_name (table, "szColumn", bound[0], NULL);
  mdb_bind_column_by_name (table, "szObject", bound[1], NULL);
  mdb_bind_column_by_name (table, "szReferencedColumn", bound[2], NULL);
  mdb_bind_column_by_name (table, "szReferencedObject", bound[3], NULL);
  mdb_rewind_table (table);
  
  num_relationships = table->num_rows;
  relationships = xmalloc (sizeof (struct relationship_s) * num_relationships);

  i = 0;
  while (mdb_fetch_row (table))
    {
      relationships[i].column = xstrdup (bound[0]);
      relationships[i].table = xstrdup (bound[1]);
      relationships[i].referenced_column = xstrdup (bound[2]);
      relationships[i].referenced_table = xstrdup (bound[3]);
      i++;
    }
}

static char *
get_field_name (MdbHandle *mdb,
                const char *table_name,
                const char *col_name)
{
  char *field_name;
  char *referenced_table;
  char *referenced_column;
  size_t i;

  /* If this field is a relationship to other table, build a compound
     field name.  */

  referenced_table = NULL;
  referenced_column = NULL;
  for (i = 0; i < num_relationships; i++)
    {
      if ((strcmp (relationships[i].table, table_name) == 0) 
          && (strcmp (relationships[i].column, col_name) == 0))
        {
          referenced_table =
            rec_field_name_normalise (relationships[i].referenced_table);
          if (!referenced_table)
            {
              recutl_fatal (_("failed to normalise record type name %s\n"),
                            relationships[i].referenced_table);
            }

          referenced_column = 
            rec_field_name_normalise (relationships[i].referenced_column);
          if (!referenced_column)
            {
              recutl_fatal (_("failed to normalise field name %s\n"),
                            relationships[i].referenced_column);
            }

          break;
        }
    }

  field_name = rec_field_name_normalise (col_name);
  if (!field_name)
    {
      recutl_fatal (_("failed to normalise field name %s\n"),
                    table_name);
    }

  if (referenced_table && referenced_column)
    {
      /* TODO: handle foreign keys.  */
    }

  return field_name;
}

static rec_rset_t
process_table (MdbCatalogEntry *entry)
{
  rec_rset_t rset;
  MdbTableDef *table;
  MdbHandle *mdb;
  size_t i;
  MdbColumn *col;
  char *table_name;
  char *column_name;
  char *field_name;
  char *field_value;
  char **bound_values;
  char *normalised;
  int *bound_lens;
#define TYPE_VALUE_SIZE 256
  char type_value[TYPE_VALUE_SIZE];
  rec_record_t record;
  rec_field_t field;

  mdb = entry->mdb;
  table_name = entry->object_name;
  table = mdb_read_table (entry);

  /* Create the record set.  */
  rset = rec_rset_new ();
  if (!rset)
    recutl_out_of_memory ();

  /* Create the record descriptor and add the %rec: entry.  */
  field_name = rec_field_name_normalise (table_name);
  if (!field_name)
    {
      recutl_fatal (_("failed to normalise record type name %s\n"),
                    table_name);
    }

  record = rec_record_new ();
  field = rec_field_new ("%rec", field_name);
  rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY);
  free (field_name);

  /* Get the columns of the table.  */
  mdb_read_columns (table);

  /* Loop on the columns.  We will define the key and the types.  */
  for (i = 0; i < table->num_cols; i++)
    {
      col = g_ptr_array_index (table->columns, i);
      column_name = col->name;
      type_value[0] = 0;
      normalised = rec_field_name_normalise (column_name);
      if (!normalised)
        {
          recutl_fatal (_("failed to normalise the field name %s\n"),
                        column_name);
        }

      /* Emit a field type specification.  */
      switch (col->col_type)
        {
        case MDB_BOOL:
          {
            snprintf (type_value, TYPE_VALUE_SIZE,
                      "%s bool", normalised);
            break;
          }
        case MDB_BYTE:
          {
            snprintf (type_value, TYPE_VALUE_SIZE,
                      "%s range 256", normalised);
            break;
          }
        case MDB_INT:
        case MDB_LONGINT:
        case MDB_NUMERIC:
          {
            snprintf (type_value, TYPE_VALUE_SIZE,
                      "%s int", normalised);
            break;
          }
        case MDB_MONEY:
        case MDB_FLOAT:
        case MDB_DOUBLE:
          {
            snprintf (type_value, TYPE_VALUE_SIZE,
                      "%s real", normalised);
            break;
          }
        case MDB_DATETIME:
          {
            snprintf (type_value, TYPE_VALUE_SIZE,
                      "%s date", normalised);
            break;
          }
        case MDB_TEXT:
          {
            if (col->col_size > 0)
              {
                snprintf (type_value, TYPE_VALUE_SIZE,
                          "%s size %d", normalised, col->col_size);
              }
            break;
          }
        case MDB_REPID:
        case MDB_MEMO:
        case MDB_OLE:
        default:
          {
            break;
          }
        }

      if (type_value[0] != 0)
        {
          /* Insert a type field for this column.  */
          field = rec_field_new ("%type", type_value);
          rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY);
        }
    }

  rec_rset_set_descriptor (rset, record);
  
  /* Add the records for this table.  */
  mdb_rewind_table (table);

  bound_values = xmalloc (table->num_cols * sizeof(char *));
  bound_lens = xmalloc(table->num_cols * sizeof(int));
  for (i = 0; i < table->num_cols; i++)
    {
      bound_values[i] = xmalloc (MDB_BIND_SIZE);
      mdb_bind_column (table, i+1, bound_values[i], &bound_lens[i]);
    }

  while (mdb_fetch_row (table))
    {
      record = rec_record_new ();
      if (!record)
        recutl_out_of_memory ();

      for (i = 0; i < table->num_cols; i++)
        {
          col = g_ptr_array_index (table->columns, i);

          if (col->col_type == MDB_OLE)
            {
              continue;
            }

          /* Compute the value of the field.  */
          field_value = xmalloc (bound_lens[i] + 1);
          memcpy (field_value, bound_values[i], bound_lens[i]);
          field_value[bound_lens[i]] = '\0';

          if (mdb2rec_keep_empty_fields || (strlen (field_value) > 0))
            {
              /* Create the field and append it into the record.  */
              field = rec_field_new (get_field_name (mdb, table_name, column_name),
                                     field_value);
              if (!field)
                {
                  recutl_fatal (_("invalid field name %s.\n"), column_name);
                }

              rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY);
            }

          free (field_value);
        }

      rec_record_set_container (record, rset);
      rec_mset_append (rec_rset_mset (rset), MSET_RECORD, (void *) record, MSET_ANY);
    }

  mdb_free_tabledef (table);

  return rset;
}

static rec_db_t
process_mdb (void)
{
  rec_db_t db;
  MdbHandle *mdb;
  MdbCatalogEntry *entry;
  int i;
  char *table_name;

  /* Create the rec database.  */
  db = rec_db_new ();
  if (!db)
    recutl_out_of_memory ();

  /* Initialize libmdb and open the input file.  */
  mdb_init();
  mdb_set_date_fmt ("%Y-%m-%dT%H:%M:%S%z"); /* ISO 8601 */

  mdb = mdb_open (mdb2rec_mdb_file, MDB_NOFLAGS);
  if (!mdb)
    {
      recutl_fatal (_("could not open file %s\n"),
                    mdb2rec_mdb_file);
    }

  /* Read the catalog.  */
  if (!mdb_read_catalog (mdb, MDB_TABLE))
    {
      recutl_fatal (_("file does not appear to be an Access database\n"));
    }

  /* Read relationships from the database.  Relationships in mdb files
     are stored in the MSysRelationships table.  */
  get_relationships (mdb);

  /* Iterate on the catalogs.  */
  for (i = 0; i < mdb->num_catalog; i++)
    {
      entry = g_ptr_array_index (mdb->catalog, i);

      table_name = rec_field_name_normalise (entry->object_name);

      if ((entry->object_type == MDB_TABLE)
          && (mdb_is_user_table (entry) || mdb2rec_include_system)
          && (!mdb2rec_mdb_table || (strcmp (mdb2rec_mdb_table, table_name) == 0)))
        {
          if (mdb2rec_list_tables)
            {
              fprintf (stdout, "%s\n", table_name);
            }
          else
            {
              rec_db_insert_rset (db,
                                  process_table (entry),
                                  rec_db_size (db));
            }
        }
    }

  return db;
}

int
main (int argc, char *argv[])
{
  int ret;
  rec_db_t db;
  rec_writer_t writer;

  recutl_init ("mdb2rec");

  parse_args (argc, argv);
  db = process_mdb ();

  if (db)
    {
      writer = rec_writer_new (stdout);
      rec_write_db (writer, db);
      
      rec_writer_destroy (writer);
      rec_db_destroy (db);

      ret = EXIT_SUCCESS;
    }
  else
    {
      ret = EXIT_FAILURE;
    }

  return ret;
}

/* End of mdb2rec.c */
