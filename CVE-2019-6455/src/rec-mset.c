/* -*- mode: C -*-
 *
 *       File:         rec-mset.c
 *       Date:         Thu Apr  1 17:07:00 2010
 *
 *       GNU recutils - Ordered Heterogeneous Set
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
#include <stdio.h>

#include <rec.h>

#include <gl_array_list.h>
#include <gl_list.h>

/*
 * Data types.
 */

#define MAX_NTYPES 4

struct rec_mset_elem_s
{
  rec_mset_type_t type;
  void *data;

  gl_list_node_t list_node;

  /* Containing multi-set.  */
  rec_mset_t mset;
};

struct rec_mset_s
{
  int ntypes;

  /* Properties of the element types.  */
  char *name[MAX_NTYPES];
  rec_mset_disp_fn_t disp_fn[MAX_NTYPES];
  rec_mset_equal_fn_t equal_fn[MAX_NTYPES];
  rec_mset_dup_fn_t dup_fn[MAX_NTYPES];
  rec_mset_compare_fn_t compare_fn[MAX_NTYPES];

  /* Statistics.  */
  size_t count[MAX_NTYPES];

  gl_list_t elem_list;
};

/*
 * Forward declarations of static functions.
 */

static void rec_mset_init (rec_mset_t mset);

static bool rec_mset_elem_equal_fn (const void *e1,
                                    const void *e2);
static void rec_mset_elem_dispose_fn (const void *e);
static int  rec_mset_elem_compare_fn (const void *e1, const void *e2);

static rec_mset_list_iter_t rec_mset_iter_gl2mset (gl_list_iterator_t  list_iter);
static gl_list_iterator_t   rec_mset_iter_mset2gl (rec_mset_list_iter_t mset_iter);

/* Create a new element to be stored in a given mset, of the givent
   type, and return it.  NULL is returned if there is no enough memory
   to perform the operation.  */

static rec_mset_elem_t rec_mset_elem_new (rec_mset_t mset,
                                          rec_mset_type_t type,
                                          void *data);

/* Destroy the resources used by a mset element, freeing any used
   memory.  The element reference becomes invalid after executing this
   function.  */

static void rec_mset_elem_destroy (rec_mset_elem_t elem);

/*
 * Public functions.
 */

rec_mset_t
rec_mset_new (void)
{
  rec_mset_t new;
  int i;

  new = malloc (sizeof (struct rec_mset_s));
  if (new)
    {
      rec_mset_init (new);

      new->ntypes = 1;

      for (i = 0; i < MAX_NTYPES; i++)
        {
          new->count[i] = 0;
          new->name[i] = NULL;
          new->equal_fn[i] = NULL;
          new->disp_fn[i] = NULL;
          new->dup_fn[i] = NULL;
          new->compare_fn[i] = NULL;
        }

      new->elem_list = gl_list_nx_create_empty (GL_ARRAY_LIST,
                                                rec_mset_elem_equal_fn,
                                                NULL,
                                                rec_mset_elem_dispose_fn,
                                                true);

      if (new->elem_list == NULL)
        {
          /* Out of memory.  */
          rec_mset_destroy (new);
          new = NULL;
        }
    }

  return new;
}

void
rec_mset_destroy (rec_mset_t mset)
{
  if (mset)
    {
      int i;

      for (i = 0; i < mset->ntypes; i++)
        free(mset->name[i]);
      gl_list_free (mset->elem_list);
      free (mset);
    }
}

rec_mset_t
rec_mset_dup (rec_mset_t mset)
{
  rec_mset_t new;
  rec_mset_elem_t elem;
  gl_list_iterator_t iter;
  int i;
  
  new = rec_mset_new ();

  if (new)
    {
      /* Register the types.  */
      new->ntypes = mset->ntypes;
      for (i = 0; i < new->ntypes; i++)
        {
          new->count[i] = 0;
          if (mset->name[i])
            {
              new->name[i] = strdup (mset->name[i]);
              if (!new->name[i])
                {
                  /* Out of memory.  */
                  rec_mset_destroy (new);
                  return NULL;
                }
            }
          new->disp_fn[i] = mset->disp_fn[i];
          new->equal_fn[i] = mset->equal_fn[i];
          new->dup_fn[i] = mset->dup_fn[i];
          new->compare_fn[i] = mset->compare_fn[i];
        }

      /* Duplicate the elements.  */

      iter = gl_list_iterator (mset->elem_list);
      while (gl_list_iterator_next (&iter, (const void **) &elem, NULL))
        {
          void *data = NULL;

          /* Set the data.  */
          if (new->dup_fn[elem->type])
            {
              data = (new->dup_fn[elem->type]) (elem->data);
              if (!data)
                {
                  /* Out of memory.  */
                  rec_mset_destroy (new);
                  return NULL;
                }
            }
          else
            {
              data = elem->data;
            }

          /* Append the new data into a new element.  */

          rec_mset_append (new, elem->type, data, MSET_ANY);
        }

      gl_list_iterator_free (&iter);
    }

  return new;
}

rec_mset_t
rec_mset_sort (rec_mset_t mset)
{
  rec_mset_elem_t elem;
  gl_list_iterator_t iter;
  gl_list_t list;

  /* Save a reference to the old gnulib list and create a new, empty
     one.  */

  list = mset->elem_list;
  mset->elem_list = gl_list_nx_create_empty (GL_ARRAY_LIST,
                                             rec_mset_elem_equal_fn,
                                             NULL,
                                             rec_mset_elem_dispose_fn,
                                             true);
  if (!mset->elem_list)
    {
      /* Out of memory.  */
      return NULL;
    }

  /* Iterate on the old list getting the data of the elements and
     inserting it into the new sorted gl_list.  */

  iter = gl_list_iterator (list);
  while (gl_list_iterator_next (&iter, (const void **) &elem, NULL))
    {
      /* Create a new node list with the proper data and insert it
         into the list using whatever sorting criteria is implemented
         by compare_fn.  */

      if (!rec_mset_add_sorted (mset, elem->type, elem->data))
        {
          /* Out of memory.  Delete the new list and restore the old
             one.  */

          gl_list_free (mset->elem_list);
          mset->elem_list = list;
          return NULL;
        }

      /* We don't want the memory used by the element to be disposed
         when the old list gets destroyed.  The generic element
         disposal function always checks if the data is NULL before
         invoking the corresponding disp_fn callback.  */

      elem->data = NULL;
    }
  gl_list_iterator_free (&iter);

  /* Destroy the memory used by the old list, but removing the
     dispose_fn callbacks first for the proper types in order to avoid
     the disposal of the elements!.  */

  gl_list_free (list);

  return mset;
}

bool
rec_mset_type_p (rec_mset_t mset,
                 rec_mset_type_t type)
{
  return type < mset->ntypes;
}

rec_mset_type_t
rec_mset_register_type (rec_mset_t mset,
                        char *name,
                        rec_mset_disp_fn_t disp_fn,
                        rec_mset_equal_fn_t equal_fn,
                        rec_mset_dup_fn_t dup_fn,
                        rec_mset_compare_fn_t compare_fn)
{
  rec_mset_type_t new_type;

  new_type = mset->ntypes++;
  mset->count[new_type] = 0;
  mset->name[new_type] = strdup (name);
  mset->disp_fn[new_type] = disp_fn;
  mset->equal_fn[new_type] = equal_fn;
  mset->dup_fn[new_type] = dup_fn;
  mset->compare_fn[new_type] = compare_fn;

  return new_type;
}

size_t
rec_mset_count (rec_mset_t mset,
                rec_mset_type_t type)
{
  return mset->count[type];
}

void *
rec_mset_get_at (rec_mset_t mset,
                 rec_mset_type_t type,
                 size_t position)
{
  void *result;
  rec_mset_elem_t elem;

  if ((position < 0) || (position >= mset->count[type]))
    {
      /* Invalid position.  */
      return NULL;
    }

  if (type == MSET_ANY)
    {
      /* An element of any type was requested.  Simply call the gnulib
         list get_at function, that will use the most efficient way to
         retrieve the element.  */

      elem = (rec_mset_elem_t) gl_list_get_at (mset->elem_list,
                                               position);

    }
  else
    {
      /* Iterate on the elements in the gnulib list until the
         POSITIONth element of the specified type is found.  */

      rec_mset_elem_t cur_elem;
      gl_list_node_t node;
      gl_list_iterator_t iter; 
      int count[MAX_NTYPES];
      int i = 0;

      elem = NULL;
      for (i = 0; i < MAX_NTYPES; i++)
        {
          count[i] = 0;
        }
      
      iter = gl_list_iterator (mset->elem_list);
      while (gl_list_iterator_next (&iter, (const void **) &cur_elem, &node))
        {
          if ((type == MSET_ANY)
              || ((type == cur_elem->type) && (count[cur_elem->type] == position)))
            {
              elem = cur_elem;
              break;
            }
          else
            {
              count[cur_elem->type]++;
              count[0]++;
            }
        }
    }

  if (elem)
    {
      result = elem->data;
    }
  else
    {
      result = NULL;
    }

  return result;
}

bool
rec_mset_remove_at (rec_mset_t mset,
                    rec_mset_type_t type,
                    size_t position)
{
  rec_mset_elem_t elem;
  void *data;
  bool removed = false;

  if (mset->count[type] > 0)
    {
      if (position < 0)
        {
          position = 0;
        }
      if (position >= mset->count[type])
        {
          position = mset->count[type] - 1;
        }

      data = rec_mset_get_at (mset, type, position);
      elem = rec_mset_search (mset, data);
      if (rec_mset_remove_elem (mset, elem))
        {
          removed = true;
        }
    }

  return removed;
}

rec_mset_elem_t
rec_mset_insert_at (rec_mset_t mset,
                    rec_mset_type_t type,
                    void *data,
                    size_t position)
{
  rec_mset_elem_t elem = NULL;
  gl_list_node_t node;

  node = NULL;

  /* Create the mset element to insert in the gl_list, returning NULL
     if there is no enough memory.  */
  
  elem = rec_mset_elem_new (mset, type, data);
  if (!elem)
    {
      return NULL;
    }

  /* Insert the element at the proper place in the list.  */

  if (position < 0)
    {
      node = gl_list_nx_add_first (mset->elem_list,
                                   (void *) elem);
    }
  else if (position >= mset->count[0])
    {
      node = gl_list_nx_add_last (mset->elem_list,
                                  (void *) elem);
    }
  else
    {
      node = gl_list_nx_add_at (mset->elem_list,
                                position,
                                (void *) elem);
    }

  if (node == NULL)
    {
      rec_mset_elem_destroy (elem);
      elem = NULL;
    }
  else
    {
      elem->list_node = node;

      mset->count[0]++;
      if (elem->type != MSET_ANY)
        {
          mset->count[elem->type]++;
        }
    }

  return elem;
}

rec_mset_elem_t
rec_mset_append (rec_mset_t mset,
                 rec_mset_type_t elem_type,
                 void *data,
                 rec_mset_type_t type)
{
  return rec_mset_insert_at (mset,
                             elem_type,
                             data,
                             rec_mset_count (mset, type));
}

bool
rec_mset_remove_elem (rec_mset_t mset,
                      rec_mset_elem_t elem)
{
  rec_mset_type_t type = elem->type;
  bool res = gl_list_remove_node (mset->elem_list, elem->list_node);
  if (res)
    {
      /* Update statistics.  */

      mset->count[type]--;
      if (type != MSET_ANY)
        {
          mset->count[MSET_ANY]--;
        }
    }
  
  return res;
}

rec_mset_elem_t
rec_mset_insert_after (rec_mset_t mset,
                       rec_mset_type_t type,
                       void *data,
                       rec_mset_elem_t elem)
{
  rec_mset_elem_t new_elem;
  gl_list_node_t node;

  /* Create the mset element to insert in the gl_list, returning NULL
     if there is no enough memory.  */

  new_elem = rec_mset_elem_new (mset, type, data);
  if (!new_elem)
    {
      return NULL;
    }

  /* Find the requested place where to insert the new element.  If
     ELEM is not found in the multi-set then the new element is
     appended to the multi-set.  */

  node = gl_list_search (mset->elem_list, (void *) elem);
  if (node)
    {
      node = gl_list_nx_add_after (mset->elem_list,
                                   node,
                                   (void *) new_elem);
      if (!node)
        {
          /* Out of memory.  */
          rec_mset_elem_destroy (new_elem);
          return NULL;
        }

      new_elem->list_node = node;

      mset->count[0]++;
      if (new_elem->type != MSET_ANY)
        {
          mset->count[new_elem->type]++;
        }
    }
  else
    {
      node = gl_list_nx_add_last (mset->elem_list, (void *) elem);
      if (!node)
        {
          /* Out of memory.  */
          rec_mset_elem_destroy (new_elem);
          return NULL;
        }

      new_elem->list_node = node;
    }

  return new_elem;
}

rec_mset_elem_t
rec_mset_search (rec_mset_t mset,
                 void *data)
{
  rec_mset_elem_t result = NULL;
  rec_mset_elem_t elem;
  gl_list_iterator_t iter;

  iter = gl_list_iterator (mset->elem_list);
  while (gl_list_iterator_next (&iter, (const void **) &elem, NULL))
    {
      if (elem->data == data)
        {
          result = elem;
          break;
        }
    }

  gl_list_iterator_free (&iter);

  return result;
}

rec_mset_iterator_t
rec_mset_iterator (rec_mset_t mset)
{
  gl_list_iterator_t  list_iter;
  rec_mset_iterator_t mset_iter;

  /* Fill the mset iterator structure.  Note that the list_iter field
     of the mset iterator must have the same size and structure than
     the gl_list_iterator_t structure.  */

  mset_iter.mset = mset;

  list_iter = gl_list_iterator (mset->elem_list);  
  mset_iter.list_iter = rec_mset_iter_gl2mset (list_iter);

  return mset_iter;
}

bool
rec_mset_iterator_next (rec_mset_iterator_t *iterator,
                        rec_mset_type_t type,
                        const void **data,
                        rec_mset_elem_t *elem)
{
  bool found = true;
  rec_mset_elem_t mset_elem;
  gl_list_iterator_t list_iter;
  gl_list_node_t list_node;

  /* Extract the list iterator from the multi-set iterator.  */

  list_iter = rec_mset_iter_mset2gl (iterator->list_iter);

  /* Advance the list iterator until an element of the proper type is
     found.  */

  while ((found = gl_list_iterator_next (&list_iter, (const void**) &mset_elem, &list_node))
         && (type != 0) && (mset_elem->type != type));

  if (found)
    {
      /* Update the multi-set iterator and set both DATA and ELEM.  */
      
      iterator->list_iter = rec_mset_iter_gl2mset (list_iter);
      if (data)
        *data = mset_elem->data;
      if (elem)
        {
          mset_elem->list_node = list_node;
          *elem = mset_elem;
        }
    }

  return found;
}

void
rec_mset_iterator_free (rec_mset_iterator_t *iterator)
{
  gl_list_iterator_t list_iter;

  /* Extract the list iterator, free it and copy it back to the mset
     iterator.  */

  list_iter = rec_mset_iter_mset2gl (iterator->list_iter);
  gl_list_iterator_free (&list_iter);
  iterator->list_iter = rec_mset_iter_gl2mset (list_iter);
}

int
rec_mset_elem_type (rec_mset_elem_t elem)
{
  return elem->type;
}

void
rec_mset_elem_set_type (rec_mset_elem_t elem,
                        rec_mset_type_t type)
{
  elem->mset->count[elem->type]--;
  elem->type = type;
  elem->mset->count[type]++;
}

void *
rec_mset_elem_data (rec_mset_elem_t elem)
{
  return elem->data;
}

void
rec_mset_elem_set_data (rec_mset_elem_t elem,
                        void *data)
{
  elem->data = data;
}

bool
rec_mset_elem_equal_p (rec_mset_elem_t elem1,
                       rec_mset_elem_t elem2)
{
  return rec_mset_elem_equal_fn ((void *) elem1,
                                 (void *) elem2);
}

void *
rec_mset_elem_dup_data (rec_mset_elem_t elem)
{
  return elem->mset->dup_fn[elem->type] (elem->data);
}

void
rec_mset_dump (rec_mset_t mset)
{
  gl_list_iterator_t iter;
  gl_list_node_t node;
  rec_mset_elem_t elem;
  int i;
  
  printf ("MSET:\n");
  printf ("  ntypes: %d\n", mset->ntypes);

  for (i = 0; i < mset->ntypes; i++)
    {
      printf("  type %d:\n", i);
      printf("    count:     %zd\n", mset->count[i]);
      printf("    disp_fn:   %p\n", mset->disp_fn[i]);
      printf("    equal_fn:  %p\n", mset->equal_fn[i]);
      printf("    dup_fn:    %p\n", mset->dup_fn[i]);
    }

  printf("  nodes:\n");
  iter = gl_list_iterator (mset->elem_list);
  while (gl_list_iterator_next (&iter, (const void **) &elem, &node))
    {
      printf("    node=%p elem=%p elem->type=%d elem->data=%p contained=%p\n", node, elem,
             elem->type, elem->data, elem->mset);
      i++;
    }

  printf("END MSET\n");
}

rec_mset_elem_t
rec_mset_add_sorted (rec_mset_t mset,
                     rec_mset_type_t type,
                     void *data)
{
  rec_mset_elem_t elem;
  gl_list_node_t node;

  /* Create the mset element to insert in the gl_list, returning NULL
     if there is no enough memory.  */

  elem = rec_mset_elem_new (mset, type, data);
  if (!elem)
    {
      return NULL;
    }

  /* Insert the element at the proper place in the list.  */

  node = gl_sortedlist_nx_add (mset->elem_list,
                               rec_mset_elem_compare_fn,
                               (void *) elem);
  if (!node)
    {
      rec_mset_elem_destroy (elem);
      return NULL;
    }

  elem->list_node = node;

  mset->count[0]++;
  if (elem->type != MSET_ANY)
    {
      mset->count[elem->type]++;
    }

  return elem;
}

/*
 * Private functions.
 */

static void
rec_mset_init (rec_mset_t mset)
{
  /* Initialize the mset structure so it can be safely passed to
     rec_mset_destroy even if its contents are not completely
     initialized with real values.  */

  memset (mset, 0 /* NULL */, sizeof (struct rec_mset_s));
}

static bool
rec_mset_elem_equal_fn (const void *e1,
                        const void *e2)
{
  rec_mset_elem_t elem1;
  rec_mset_elem_t elem2;

  elem1 = (rec_mset_elem_t) e1;
  elem2 = (rec_mset_elem_t) e2;

  if ((elem1->mset != elem2->mset)
      || (elem1->type != elem2->type))
    {
      return false;
    }

  return (elem1->mset->equal_fn[elem1->type]) (elem1->data,
                                               elem2->data);
}

static void
rec_mset_elem_dispose_fn (const void *e)
{
  rec_mset_elem_t elem;

  elem = (rec_mset_elem_t) e;
  rec_mset_elem_destroy (elem);
}

static int
rec_mset_elem_compare_fn (const void *e1,
                          const void *e2)
{
  int result = 0;
  rec_mset_elem_t elem1;
  rec_mset_elem_t elem2;

  elem1 = (rec_mset_elem_t) e1;
  elem2 = (rec_mset_elem_t) e2;

  if (elem1->mset->compare_fn)
    {
      result = (elem1->mset->compare_fn[elem1->type]) (elem1->data,
                                                       elem2->data,
                                                       elem2->type);
    }

  return result;
}

static rec_mset_list_iter_t
rec_mset_iter_gl2mset (gl_list_iterator_t list_iter)
{
  rec_mset_list_iter_t mset_iter;

  mset_iter.vtable = (void *) list_iter.vtable;
  mset_iter.list   = (void *) list_iter.list;
  mset_iter.count  = list_iter.count;
  mset_iter.p      = list_iter.p;
  mset_iter.q      = list_iter.q;
  mset_iter.i      = list_iter.i;
  mset_iter.j      = list_iter.j;

  return mset_iter;
}

static gl_list_iterator_t
rec_mset_iter_mset2gl (rec_mset_list_iter_t mset_iter)
{
  gl_list_iterator_t list_iter;

  list_iter.vtable = (const struct gl_list_implementation *) mset_iter.vtable;
  list_iter.list  = (gl_list_t) mset_iter.list;
  list_iter.count = mset_iter.count;
  list_iter.p     = mset_iter.p;
  list_iter.q     = mset_iter.q;
  list_iter.i     = mset_iter.i;
  list_iter.j     = mset_iter.j;

  return list_iter;
}

static rec_mset_elem_t
rec_mset_elem_new (rec_mset_t mset,
                   rec_mset_type_t type,
                   void *data)
{
  rec_mset_elem_t new;

  if (type >= mset->ntypes)
    {
      return NULL;
    }

  new = malloc (sizeof (struct rec_mset_elem_s));
  if (new)
    {
      new->type = type;
      new->data = data;
      new->mset = mset;
      new->list_node = NULL;
    }

  return new;
}

static void
rec_mset_elem_destroy (rec_mset_elem_t elem)
{
  if (elem)
    {
      /* Dispose the data stored in the element if a disposal callback
         function was configured by the user.  The callback is never
         invoked if the stored data is NULL.  */
      
      if (elem->data && elem->mset->disp_fn[elem->type])
        {
          elem->mset->disp_fn[elem->type] (elem->data);
        }
      
      free (elem);
    }
}

/* End of rec-mset.c */
