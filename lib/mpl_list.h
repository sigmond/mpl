/*
 *   Copyright 2013 ST-Ericsson SA
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 *   Author: Per Sigmond <per.xx.sigmond@stericsson.com>
 *   Author: Harald Johansen <harald.johansen@stericsson.com>
 *   Author: Emil B. Viken <emil.b.viken@stericsson.com>
 *
 */


/*************************************************************************
 *
 * File name: mpl_list.h
 *
 * Description: MPL list API declarations
 *
 **************************************************************************/
#ifndef _MPL_LIST_H
#define _MPL_LIST_H

/**************************************************************************
 * Includes
 *************************************************************************/
#include <stddef.h>
#include "mpl_stdbool.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @file mpl_list.h
 * @brief MPL list implementation
 */

/** @defgroup MPL_LIST MPL list API
 *  @ingroup MPL
 * The MPL list implementation is small, simple and versatile. It is optimized
 * for small size and flexibility.
 *
 * The struct mpl_list (mpl_list_t) is meant to be a member of the nodes that
 * are to be linked. This makes the list implementation generic, i.e. all kinds
 * of nodes (containers) can be linked in lists (although one particular list
 * will normally link only one type of container).
 *
 * It should be noted that the list implementation itself does \b not allocate or
 * free any memory.
 *
 * A restriction is that one list entry in a container can only make the
 * container member of one list at a time. If one wants a container to be
 * member of several lists at the same time, several list entries must exist
 * in the container.
 * <pre>
 * container (node):
 * +-----------------------------+
 * | contents                    |
 * | .....                       |
 * | ...                         |
 * | mpl_list_t list_entry;      |
 * +-----------------------------+
 *
 * list:         container                container
 *               +--------------------+   +--------------------+
 * list_p --+    |  contents          |   |  contents          |
 *          |    |  .....             |   |  .....             |
 *          |    |  ...               |   |  ...               |
 *          +----|->list_entry.next_p-----|->list_entry.next_p------+
 *               +--------------------+   +--------------------+    |
 *                                                                  |
 *                                   +------------------------------+
 *                                   |
 *                                   |    container
 *                                   |    +--------------------+
 *                                   |    |  contents          |
 *                                   |    |  .....             |
 *                                   |    |  ...               |
 *                                   +----|->list_entry.next_p-----+
 *                                        +--------------------+   |
 *                                                                ---
 * </pre>
 * The container itself is obtained by using the macro MPL_LIST_CONTAINER() on
 * the list pointer.
 *
 */
/*****************************************************************************
 *
 * Defines & Type definitions
 *
 *****************************************************************************/

/**
 * @ingroup MPL_LIST
 * mpl_list_t
 *
 * Structure for linking containers into a list.
 *
 */
typedef struct mpl_list
{
  struct mpl_list *next_p;
} mpl_list_t;


/****************************************************************************
 *
 * Functions and macros
 *
 ****************************************************************************/

/**
 * @ingroup MPL_LIST
 * mpl_list_add
 *
 * Add entry to list.
 *
 * @param list_pp   address of list pointer
 * @param new_p     pointer to list structure to be added (at the beginning)
 *
 */
void mpl_list_add( mpl_list_t **list_pp, mpl_list_t * new_p );


/**
 * @ingroup MPL_LIST
 * mpl_list_remove
 *
 * Remove entry from list.
 *
 * @param list_pp   address of list pointer
 * @param entry_p   pointer to list structure to be removed (NULL means remove
 *                first entry)
 *
 * @return The list structure that was removed
 *
 */
mpl_list_t *mpl_list_remove( mpl_list_t **list_pp, mpl_list_t *entry_p );


/**
 * @ingroup MPL_LIST
 * mpl_list_append
 *
 * Append one list to another.
 *
 * @param first_pp   address of list pointer of first list (may be changed if
 *                 first list is empty)
 * @param second_p   pointer to list structure to be appended (at the end of
 *                 the first one)
 *
 *
 */
void mpl_list_append( mpl_list_t **first_pp, mpl_list_t * second_p );


/**
 * @ingroup MPL_LIST
 * MPL_LIST_CONTAINER
 *
 * Return pointer to container (node) where list structure is
 * member.
 *
 * @param list_p    pointer to list struct inside the container
 * @param type      data type of the container (a struct)
 * @param member    name of the list member of the container
 *
 * @return Pointer to container
 *
 */
#define MPL_LIST_CONTAINER(list_p,type,member) \
    ((type*)(((char*)list_p) - offsetof(type,member))\
     /*lint -e826 suspicisous pointer, but ok*/)


/**
 * @ingroup MPL_LIST
 * MPL_LIST_FOR_EACH
 *
 * Loop through list (use this as loop head, a la while)
 *
 * @param list_p    pointer to list
 * @param tmp_p     pointer that will point to each entry in the list while
 *                looping
 *
 */
#define MPL_LIST_FOR_EACH(list_p,tmp_p) \
    for((tmp_p) = (list_p);(tmp_p)!=NULL;(tmp_p)=(tmp_p)->next_p)


/**
 * @ingroup MPL_LIST
 * MPL_LIST_FOR_EACH_SAFE
 *
 * Loop through list (use this as loop head, a la while)
 * This loop allows the current object to be deleted in the loop
 * block.
 *
 * @param list_p    pointer to list (type mpl_list_p *)
 * @param tmp_p     pointer that will point to each entry in the list while
 *                looping (type mpl_list_p *)
 * @param tmp2_p    pointer that is used internally in the macro
 *                (type mpl_list_p *)
 *
 */
#define MPL_LIST_FOR_EACH_SAFE(list_p,tmp_p,tmp2_p) \
    for((tmp_p) = (list_p),(tmp2_p) = (list_p) ? (list_p)->next_p : NULL;\
        (tmp_p)!=NULL;\
        (tmp_p) = (tmp2_p), (tmp2_p) = (tmp2_p) ? (tmp2_p)->next_p : NULL)


/**
 * @ingroup MPL_LIST
 * mpl_list_len
 *
 * Return length of list.
 *
 * @param list_p pointer to list
 *
 * @return Number of elements in list.
 *
 */
size_t mpl_list_len(const mpl_list_t *list_p);

/**
 * @ingroup MPL_LIST
 * mpl_is_on_list
 *
 * Return TRUE if entry_p is member of list list_p.
 *
 * @param list_p   pointer to list
 * @param entry_p  pointer to list entry
 *
 * @return true if the entry is member of the list
 *
 */
bool mpl_is_on_list(const mpl_list_t *list_p, const mpl_list_t *entry_p);


/**
 * @ingroup MPL_LIST
 * mpl_list_last
 *
 * Return last element of list.
 *
 * @param list_p   pointer to list
 *
 * @return Last element of list or NULL if list is empty
 *
 */
mpl_list_t *mpl_list_last(mpl_list_t *list_p);


#ifdef  __cplusplus
}
#endif
#endif
