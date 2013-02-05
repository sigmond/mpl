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
 * File name: mpl_list.c
 *
 * Description: MPL list implementation
 *
 **************************************************************************/
//lint -e750 Macro not referenced, Ignored
#define IP_DEBUG_UNIT DEFINE_IP_DEBUG_UNIT(mpl_list)
/*****************************************************************************
 *
 * Include files
 *
 *****************************************************************************/

#include <stdlib.h>
#include <assert.h>
#include "mpl_list.h"

/*****************************************************************************
 *
 * Defines & Type definitions
 *
 *****************************************************************************/
#define DBG_ASSERT(EXPR) assert(EXPR)

/*****************************************************************************
 *
 * Local variables
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Private function prototypes
 *
 *****************************************************************************/

/****************************************************************************
 *
 * Public Functions
 *
 ****************************************************************************/

/**
 * mpl_list_add
 */
void mpl_list_add( mpl_list_t **list_pp, mpl_list_t *entry_p )
{
  DBG_ASSERT( entry_p != NULL );

  entry_p->next_p = *list_pp;
  *list_pp = entry_p;
}

/**
 * mpl_list_remove
 */
mpl_list_t *mpl_list_remove( mpl_list_t **list_pp, mpl_list_t *entry_p )
{
  if (entry_p == NULL)
  {
    /* Remove first */
    entry_p = *list_pp;
  }

  while (*list_pp)
  {
    if (*list_pp == entry_p)
    {
      *list_pp = entry_p->next_p;
      entry_p->next_p = NULL;
      return entry_p;
    }
    list_pp = &(*list_pp)->next_p;
  }

  return NULL;
}

/**
 * mpl_list_append
 */
void mpl_list_append( mpl_list_t **first_list_pp, mpl_list_t * second_list_p )
{
  DBG_ASSERT(first_list_pp != NULL);

  if (*first_list_pp == NULL)
  {
    *first_list_pp = second_list_p;
    return;
  }

  if (second_list_p == NULL)
    return;

  mpl_list_add(&second_list_p, mpl_list_last(*first_list_pp));
}



/**
 * mpl_list_len
 */
size_t mpl_list_len(const mpl_list_t *list_p)
{
  const mpl_list_t *tmp_p = list_p;
  size_t len = 0;

  MPL_LIST_FOR_EACH(list_p,tmp_p)
  {
    len++;
  }
  return len;
}


/**
 * mpl_is_on_list
 */
//lint -e818 Ignore lint's const recomandations
bool mpl_is_on_list(const mpl_list_t *list_p, const mpl_list_t *entry_p)
{
  const mpl_list_t *tmp_p = list_p;

  if (entry_p == NULL)
    return false;

  MPL_LIST_FOR_EACH(list_p,tmp_p)
  {
    if (tmp_p == entry_p)
      return true;
  }
  return false;
}

/**
 * mpl_list_last
 *
 */
mpl_list_t *mpl_list_last(mpl_list_t *list_p)
{
  mpl_list_t *tmp_p = list_p;

  if (list_p == NULL)
    return NULL;

  MPL_LIST_FOR_EACH(list_p,tmp_p)
  {
    if (tmp_p->next_p == NULL)
      return tmp_p;
  }
  return NULL;
}

/****************************************************************************
 *
 * Private Functions
 *
 ****************************************************************************/
