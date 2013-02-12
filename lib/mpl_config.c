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
 *   Author: Per Sigmond <per@sigmond.no>
 *   Author: Harald Johansen <harald.johansen@stericsson.com>
 *   Author: Emil B. Viken <emil.b.viken@gmail.com>
 *
 */

/*************************************************************************
 *
 * File name: mpl_config.c
 *
 * Description: MPL config system implementation
 *
 **************************************************************************/
//lint -e818 Ignore Info about const decleration

//lint -e750 Macro not referenced, Ignored
#define IP_DEBUG_UNIT DEFINE_IP_DEBUG_UNIT(mpl_config)

/*****************************************************************************
 *
 * Include files
 *
 *****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpl_config.h"
#include "mpl_file.h"
#include "mpl_param.h"
#include "mpl_dbgtrace.h"

/*****************************************************************************
 *
 * Defines & Type definitions
 *
 *****************************************************************************/

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
 * mpl_config_init - Initiate library
 **/
int mpl_config_init(mpl_config_t *config_p)
{
  *config_p = NULL;
  return 0;
}

int mpl_config_read_config_bl(char* config_path,
                              mpl_config_t *config_p,
                              int param_set_id,
                              mpl_blacklist_t blacklist)
{
  FILE *f;
  int res;

  f = fopen(config_path, "r");
  if (f == NULL)
  {
    MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                        ("Open config file error, no config file loaded!\n"));
    mpl_set_errno(E_MPL_FAILED_OPERATION);
    return -1;
  }

  res = mpl_config_read_config_bl_fp(f, config_p, param_set_id, blacklist);

  fclose(f);

  return res;
}

int mpl_config_read_config_bl_fp(FILE* fp,
                              mpl_config_t *config_p,
                              int param_set_id,
                              mpl_blacklist_t blacklist)
{
  int res;
  mpl_list_t *elem_p;
  mpl_list_t *tmp_p;
  mpl_param_element_t *param_element_p;

  if (fp == NULL)
  {
    MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                        ("File pointer is NULL, no config file loaded!\n"));
    mpl_set_errno(E_MPL_INVALID_PARAMETER);
    return -1;
  }

  /* Read parameters considering the blacklist */
  res = mpl_file_read_params_bl(fp, config_p, param_set_id, blacklist);

  if (res < 0)
    return res;

  /* Remove any elements that are not configurable */
  MPL_LIST_FOR_EACH_SAFE(*config_p, elem_p, tmp_p)
  {
    param_element_p = MPL_LIST_CONTAINER(elem_p,
                                         mpl_param_element_t,
                                         list_entry);
    if (!mpl_param_allow_config(param_element_p->id))
    {
      (void)mpl_list_remove(config_p, &param_element_p->list_entry);
      mpl_param_element_destroy(param_element_p);
    }
  }

  return res;
}

/**
 * mpl_config_write_config_bl_fp - write config file
 *
 **/
int mpl_config_write_config_bl_fp(FILE *fp,
                                  mpl_config_t config,
                                  int param_set_id,
                                  mpl_blacklist_t blacklist)
{
  int index;
  int id;
  mpl_param_element_t *def_param_p;
  mpl_list_t *conf_list_p;
  mpl_list_t *tmp_p;
  int res;

  if (fp == NULL)
  {
    MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                        ("File pointer is NULL, no config file written!\n"));
    mpl_set_errno(E_MPL_INVALID_PARAMETER);
    return -1;
  }

  /* Loop over all parameters in the set */
  for (index = 0; index < mpl_param_num_parameters(param_set_id); index++)
  {
    id = mpl_param_index_to_paramid(index, param_set_id);

    /* Skip parameters in the blacklist */
    if (mpl_param_list_find(id, blacklist))
      continue;

    /* Skip parameters that are not configurable */
    if (!mpl_param_allow_config(id))
      continue;

    /* Skip virtual parameters */
    if (MPL_PARAMID_IS_VIRTUAL(id))
      continue;

    def_param_p = mpl_param_element_get_default_bl(id, blacklist);

    fprintf(fp, "#Parameter %s (%s):\n",
            mpl_param_id_get_string(id),
            mpl_param_id_get_type_string(id));
    if(def_param_p != NULL)
    {
      fprintf(fp, "#Default: ");
      res = mpl_file_write_params_no_prefix(fp, &def_param_p->list_entry);
      if (res < 0)
      {
        mpl_param_element_destroy(def_param_p);
        return -1;
      }
      mpl_param_element_destroy(def_param_p);
      fprintf(fp, "\n");
    }
    else
    {
      fprintf(fp, "#No default\n");
    }

    conf_list_p = mpl_param_list_find_all(id, config);
    MPL_LIST_FOR_EACH(conf_list_p, tmp_p)
    {
      mpl_param_element_t *conf_param_p;

      conf_param_p =
          mpl_param_element_clone(MPL_LIST_CONTAINER(tmp_p,
                                                     mpl_param_element_t,
                                                     list_entry));
      res = mpl_file_write_params_no_prefix(fp, &conf_param_p->list_entry);
      mpl_param_element_destroy(conf_param_p);
      if (res < 0)
      {
        mpl_param_list_destroy(&conf_list_p);
        return -1;
      }
      fprintf(fp, "\n");
    }
    mpl_param_list_destroy(&conf_list_p);
    fprintf(fp, "\n");
  }

  return 0;
}

static char *pack_into_buffer(mpl_list_t *param_list_p, int no_prefix)
{
    int buflen;
    char *buf_p;
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    options.no_prefix = no_prefix;
    options.message_delimiter = '\n';

    /* Calculate buffer space */
    buflen = mpl_param_list_pack_extended(param_list_p,
                                          NULL,
                                          0,
                                          &options
                                         );
    if (buflen <= 0)
        return NULL;

    buf_p = malloc((size_t)(buflen + 1));
    if (buf_p == NULL)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,("buf_p\n"));
        mpl_set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return NULL;
    }

    /* Pack list in to the buffer */
    buflen = mpl_param_list_pack_extended(param_list_p,
                                          buf_p,
                                          buflen + 1,
                                          &options
                                         );
    if (buflen <= 0)
    {
        free(buf_p);
        return NULL;
    }

    return buf_p;
}


static int get_param_set_ids_in_config(mpl_config_t config,
                                       int param_set_ids[],
                                       int *num_param_sets_p)
{
    mpl_list_t *tmp_p;
    int num = 0;

    MPL_LIST_FOR_EACH(config, tmp_p)
    {
        mpl_param_element_t *param_elem_p;
        int found;
        int i;
        int param_set_id;

        param_elem_p = MPL_LIST_CONTAINER(tmp_p,
                                          mpl_param_element_t,
                                          list_entry);
        param_set_id = MPL_PARAMID_TO_PARAMSET(param_elem_p->id);
        for (i = 0, found = 0; i < num; i++) {
            if (param_set_id == param_set_ids[i])
                found = 1;
        }
        if (found)
            continue;

        if (num == *num_param_sets_p)
            return -1;

        param_set_ids[num] = param_set_id;
        num++;
    }
    *num_param_sets_p = num;
    return 0;
}

int mpl_config_write_config_bl_func(mpl_config_log_func_t func,
                                    mpl_config_t config,
                                    int default_param_set_id,
                                    mpl_blacklist_t blacklist)
{
  int i;
  int id;
  int index;
  mpl_param_element_t *def_param_p;
  mpl_list_t *conf_list_p;
  mpl_list_t *tmp_p;
  int len;
  char *buf_p;
  char *defstr_p;
  int param_set_ids[10];
  int num_param_sets = 10;

  if (get_param_set_ids_in_config(config, param_set_ids, &num_param_sets) < 0) {
      MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,("Too many parameter sets\n"));
      return -1;
  }

  for (i = 0; i < num_param_sets; i++) {

      /* Loop over all parameters in the set */
      for (index = 0; index < mpl_param_num_parameters(param_set_ids[i]); index++)
      {
          id = mpl_param_index_to_paramid(index, param_set_ids[i]);

          /* Skip parameters in the blacklist */
          if (mpl_param_list_find(id, blacklist))
              continue;

          /* Skip parameters that are not configurable */
          if (!mpl_param_allow_config(id))
              continue;

          /* Skip virtual parameters */
          if (MPL_PARAMID_IS_VIRTUAL(id))
              continue;

          def_param_p = mpl_param_element_get_default_bl(id, blacklist);

          if(def_param_p != NULL)
          {

              defstr_p = pack_into_buffer(&def_param_p->list_entry,
                                          (param_set_ids[i] ==
                                           default_param_set_id));
              if (defstr_p == NULL)
              {
                  mpl_param_element_destroy(def_param_p);
                  return -1;
              }
              mpl_param_element_destroy(def_param_p);

              len = snprintf(NULL, 0, "#%s", defstr_p);

              buf_p = malloc(len + 1);
              if (buf_p == NULL) {
                  MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                                      ("buf_p\n"));
                  mpl_set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
                  return -1;
              }
              if (snprintf(buf_p, len + 1, "#%s", defstr_p) != len) {
                  MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,("snprintf\n"));
                  mpl_set_errno(E_MPL_FAILED_OPERATION);
                  free(defstr_p);
                  free(buf_p);
                  return -1;
              }

              free(defstr_p);
              func(buf_p);
              free(buf_p);
          }

          conf_list_p = mpl_param_list_find_all(id, config);
          MPL_LIST_FOR_EACH(conf_list_p, tmp_p)
          {
              mpl_param_element_t *conf_param_p;
              char *tmpstr_p;

              conf_param_p =
                  mpl_param_element_clone(MPL_LIST_CONTAINER(
                                                 tmp_p,
                                                 mpl_param_element_t,
                                                 list_entry));
              tmpstr_p = pack_into_buffer(&conf_param_p->list_entry,
                                          (param_set_ids[i] ==
                                           default_param_set_id));
              mpl_param_element_destroy(conf_param_p);
              if (tmpstr_p == NULL)
              {
                  mpl_param_list_destroy(&conf_list_p);
                  return -1;
              }
              func(tmpstr_p);
              free(tmpstr_p);
          }
          mpl_param_list_destroy(&conf_list_p);
      }
  }

  return 0;
}


/**
 * mpl_config_write_blacklist_fp - write blacklist file
 *
 **/
int mpl_config_write_blacklist_fp(FILE *fp,
                                  mpl_blacklist_t blacklist,
                                  int param_set_id)
{
  int id;
  int index;
  mpl_param_element_t *bl_param_p;
  int res;

  /* Loop over all parameters in the set */
  for (index = 0; index < mpl_param_num_parameters(param_set_id); index++)
  {
     id = mpl_param_index_to_paramid(index, param_set_id);

    /* Skip parameters that are not configurable */
    if (!mpl_param_allow_config(id))
      continue;

    /* Skip virtual parameters */
    if (MPL_PARAMID_IS_VIRTUAL(id))
        continue;

    if (mpl_param_list_find(id, blacklist))
      bl_param_p = mpl_param_element_create_empty(id);
    else
      bl_param_p = NULL;

    fprintf(fp, "#Parameter %s (%s):\n",
            mpl_param_id_get_string(id),
            mpl_param_id_get_type_string(id));
    if(bl_param_p != NULL)
    {
      res = mpl_file_write_params_no_prefix(fp, &bl_param_p->list_entry);
      if (res < 0)
      {
        mpl_param_element_destroy(bl_param_p);
        return -1;
      }
      mpl_param_element_destroy(bl_param_p);
      fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
  }

  return 0;
}



mpl_param_element_t* mpl_config_get_para_bl(mpl_param_element_id_t param_id,
                                            mpl_config_t *config_p,
                                            mpl_blacklist_t blacklist)
{
  mpl_param_element_t *res;

  /* Is the parameter in the blacklist? */
  if (mpl_param_list_find(param_id, blacklist))
    return NULL;

  res = mpl_param_list_find(param_id, *config_p);
  if (res == NULL)
  {
    res = mpl_param_element_get_default_bl(param_id, blacklist);
    if(res == NULL)
      return res;
    mpl_list_add(config_p,&res->list_entry);
  }
  return res;
}

mpl_param_element_t*
    mpl_config_get_para_bl_tag(mpl_param_element_id_t param_id,
                               int tag,
                               mpl_config_t *config_p,
                               mpl_blacklist_t blacklist)
{
  mpl_param_element_t *res;

  /* Is the parameter in the blacklist? */
  if (mpl_param_list_find_tag(param_id, tag, blacklist))
    return NULL;

  res = mpl_param_list_find_tag(param_id, tag, *config_p);
  if (res == NULL)
  {
    res = mpl_param_element_get_default_bl(param_id, blacklist);
    if(res == NULL)
      return res;
    (void)mpl_param_element_set_tag(res, tag);
    mpl_list_add(config_p,&res->list_entry);
  }
  return res;
}

mpl_param_element_t*
    mpl_config_tuple_key_get_para_bl(mpl_param_element_id_t param_id,
                                     char *key_p,
                                     char *wildcard_p,
                                     mpl_config_t *config_p,
                                     mpl_blacklist_t blacklist)
{
  mpl_param_element_t *res;

  /* Is the parameter in the blacklist? */
  if (mpl_param_list_find(param_id, blacklist))
    return NULL;

  res =mpl_param_list_tuple_key_find_wildcard(param_id,
                                              key_p,
                                              wildcard_p,
                                              *config_p);
  if (res == NULL)
  {
    res = mpl_param_element_get_default_bl(param_id, blacklist);
    if(res == NULL)
      return res;
    mpl_list_add(config_p,&res->list_entry);
  }
  return res;
}


void mpl_config_reset(mpl_config_t *config_p)
{
  if(*config_p!=NULL)
    mpl_param_list_destroy(config_p);

  *config_p = NULL;
}

int mpl_config_merge(mpl_config_t *to_p, mpl_config_t from)
{
    mpl_list_t *tmp_p;

    MPL_LIST_FOR_EACH(from, tmp_p)
    {
      mpl_param_element_t *new_param_p;

      new_param_p =
          mpl_param_element_clone(MPL_LIST_CONTAINER(tmp_p,
                                                     mpl_param_element_t,
                                                     list_entry));
      if (new_param_p == NULL) {
          return -1;
      }
      MPL_DESTROY_PARAM_FROM_LIST_TAG(new_param_p->id, new_param_p->tag, *to_p);
      mpl_list_add(to_p, &new_param_p->list_entry);
    }

    return 0;
}

/****************************************************************************
 *
 * Private Functions
 *
 ****************************************************************************/
