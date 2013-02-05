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
 *   Author: Emil B. Viken <emil.b.viken@stericsson.com>
 *
 */

/*************************************************************************
 *
 * File name: mpl_param.c
 *
 * Description: MPL parameter system implementation
 *
 **************************************************************************/
//lint -e818 Ignore Info about const decleration
//lint -e774 Boolean within if ok
//lint -e740 Use of Ptr ok
//lint -e794 Use of Ptr ok
//lint -e713 Ignore Loss of precision info
//lint -e732 Ignore Loss of sign info
//lint -e737 Ignore Loss of sign info

//lint -e750 Macro not referenced, Ignored
#define IP_DEBUG_UNIT DEFINE_IP_DEBUG_UNIT(mpl_param)

/********************************************************************************
 *
 * Include files
 *
 *****************************************************************************/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <wchar.h>

#include "mpl_inttypes.h"
#include "mpl_param.h"
#include "mpl_dbgtrace.h"
#include "mpl_snprintf.h"
#include "mpl_pthread.h"

/*****************************************************************************
 *
 * Defines & Type definitions
 *
 *****************************************************************************/

#define PARAM_SET_SIZE(param_descr_p) ((param_descr_p)->paramid_enum_size -1)

#define PARAMID_TO_INDEX(paramid) ((int)((paramid) & MPL_PARAMID_TYPE_MASK) - 1)
#define PARAMID_OK(paramid,param_descr_p)       \
    ((PARAMID_TO_INDEX(paramid) >= 0) &&                            \
     (PARAMID_TO_INDEX(paramid) < PARAM_SET_SIZE(param_descr_p)))
#define INDEX_TO_PARAMID(index,param_descr_p)                           \
    ((index) +                                                          \
     MPL_PARAM_SET_ID_TO_PARAMID_BASE((param_descr_p)->param_set_id) +  \
     1 + MPL_PARAMID_POSITION_VIRTUAL(((param_descr_p)->array2[index].is_virtual?1:0)))


typedef struct
{
    mpl_param_descr_set_t *paramset_p;
    mpl_list_t list_entry;
} mpl_paramset_container_t;

#define num_scratch_strings 4
#define initial_scratch_string_len (255+1)

typedef struct
{
    mpl_thread_t pid;
    int error;
    int scratch_string_current;
    char *scratch_string[num_scratch_strings];
    int scratch_string_len[num_scratch_strings];
    mpl_list_t list_entry;
} mpl_pc_t;

/*****************************************************************************
 *
 * Global  variables
 *
 *****************************************************************************/

#define MPL_TYPE_ID_ELEMENT(TYPE) #TYPE,
static const char *mpl_type_names[] =
{
    MPL_TYPE_IDS
};
#undef MPL_TYPE_ID_ELEMENT


/*****************************************************************************
 *
 * Local variables
 *
 *****************************************************************************/

static mpl_mutex_t *mutex;   // The mutex
static bool MutexCreated = false;
static mpl_list_t *mpl_pc_list_p;
static mpl_list_t *paramset_list_p = NULL;

static const char* mpl_names_bool[] =
{
    "false",
    "true"
};

static const char* mpl_names_bool8[] =
{
    "false",
    "true"
};

/*****************************************************************************
 *
 * Private function prototypes
 *
 *****************************************************************************/
/**
 * strchr_escape
 *
 * Description: Like strchr() but supporting escape.
 *              Finds first/last non-escaped character.
 */
static char *strchr_escape(char *s, char c, char escape);
static int snprintf_escape(char delimiter, char escape,
                           char *str, size_t size, const char *format, ...);
static int fill_escape(char *str, int size, char delimiter, char escape);
static char *remove_escape(const char *src, char delimiter, char escape);

static char *get_matching_close_bracket(char open_bracket, char close_bracket, char *str_p, char escape);
static char *get_bracket_contents(const char *str,
                                  char open_bracket,
                                  char close_bracket,
                                  size_t *len_p);
static int convert_int(const char* value_str, int *value_p);
static int convert_int64(const char* value_str, int64_t *value_p);
static int convert_stringarr_to_int(const char* value_str,
                                    int *value_p,
                                    const char* stringarr[],
                                    int stringarr_size);
static int convert_enum_name_to_int64(const char* name_p,
                                      int64_t *value_p,
                                      const mpl_enum_value_t enum_values[],
                                      int enum_values_size);
static const char *convert_enum_value_to_name(int64_t value,
                                              const mpl_enum_value_t enum_values[],
                                              int enum_values_size);
static int check_integer_ranges(int64_t value,
                                const mpl_integer_range_t integer_ranges[],
                                int integer_ranges_size);
static const mpl_field_value_t *get_field_from_id(int field_id,
                                                  const mpl_field_value_t field_values[],
                                                  int field_values_size);
static const mpl_field_value_t *get_field_from_name(const char *field_str,
                                                    size_t field_strlen,
                                                    const mpl_field_value_t field_values[],
                                                    int field_values_size);
static const char *get_field_name(int param_id,
                                  int field_id,
                                  mpl_param_descr_set_t *param_descr_p);
static mpl_param_element_id_t get_field_paramid(int param_id,
                                                int field_id,
                                                mpl_param_descr_set_t *param_descr_p);
static mpl_param_element_id_t get_field_context_paramid(int param_id,
                                                        int field_id,
                                                        mpl_param_descr_set_t *param_descr_p);
static int get_child_paramid(int param_id,
                             int child_idx,
                             mpl_param_descr_set_t *param_descr_p);
static int get_child_index(mpl_param_element_id_t param_id,
                           mpl_param_element_id_t child_id,
                           mpl_param_descr_set_t *param_descr_p);
static void set_errno(int error_value);

static char *get_scratch_string(int len);

static mpl_pc_t* get_pc(void);

#if !defined(__linux__) && !defined(WIN32)
static char *mpl_strdup(const char *s);
#define strdup mpl_strdup
#endif

#ifdef MPL_MODULE_TEST
static void mpl_test_param_deinit(void);
#endif

/**
 * paramset_find
 *
 * param_set_id <= 0 means search using paramid_prefix
 * paramid_prefix == NULL means search using param_set_id
 *
 * If both are defined (<= 0 and not NULL), both are checked
 *
 * Returns parameter descriptor set of NULL if not found.
 *
 */
static mpl_param_descr_set_t* paramset_find(int param_set_id,
                                            char *paramid_prefix);

/**
 * paramset_add
 *
 * Add a parameter set.
 *
 * Returns 0 on success, -1 on failure.
 *
 */
static int paramset_add(mpl_param_descr_set_t* paramset_p);

/* MPL version upgrade */
static int upgrade_param_descr_set(mpl_param_descr_set_t *param_descr_p);
/****************************************************************************
 *
 * Public Functions
 *
 ****************************************************************************/

/**
 * mpl_param_init - Initiate library
 **/
int mpl_param_init(mpl_param_descr_set_t *param_descr_p)
{
    if(!MutexCreated)
    {
        mpl_threads_init();
        (void)mpl_mutex_init(&mutex);
        MutexCreated = true;

        /* Initialize the debugtrace */
        mpl_debugtrace_init();
    }

    /* Already existing? */
    if (NULL != paramset_find(param_descr_p->param_set_id,
                              param_descr_p->paramid_prefix))
    {
#if defined(MPL_PARAM_TRACE_ALREADY_INITIALIZED) || defined(UNIT_TEST)
        // [DOC] The same MPL parameter set is initialized twice
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_OPERATION,
                            ("Parameter set '%s' already initialized\n",
                             param_descr_p->paramid_prefix));
#endif
        return 0;
    }


    /* Check that neither id nor prefix has been registered before
     * (but as another combination) */
    if ((NULL != paramset_find(param_descr_p->param_set_id, NULL)) ||
        (NULL != paramset_find(-1, param_descr_p->paramid_prefix)))
    {
        // [DOC] Two MPL parameter sets exists with either the same prefix or
        // the same
        // [DOC] parameter set ID. This is illegal.
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_OPERATION,
                            ("Param set id or prefix already exists: '%d:%s'\n",
                             param_descr_p->param_set_id,
                             param_descr_p->paramid_prefix));
        return -1;
    }

    if (upgrade_param_descr_set(param_descr_p) < 0) {
        return -1;
    }

    return paramset_add(param_descr_p);
}

void mpl_param_deinit(void)
{
    mpl_list_t *tmp_p;
    mpl_pc_t* pc_p;
    int i;

    pc_p = get_pc();

    (void)mpl_mutex_lock(mutex);
    tmp_p = mpl_list_remove(&mpl_pc_list_p, &pc_p->list_entry);

    if (MPL_LIST_CONTAINER(tmp_p, mpl_pc_t, list_entry) != pc_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Could not remove process context\n"));
        (void)mpl_mutex_unlock(mutex);
        return;
    }

    for (i = 0; i < num_scratch_strings; i++)
        free(pc_p->scratch_string[i]);

    free(pc_p);
    (void)mpl_mutex_unlock(mutex);
#ifdef MPL_MODULE_TEST
    mpl_test_param_deinit();
#endif
    return;
}

#ifdef MPL_MODULE_TEST
static void mpl_test_param_deinit(void)
{
    mpl_list_t *elem_p;
    mpl_list_t *tmp_p;
    mpl_pc_t* pc_p;
    mpl_paramset_container_t *paramset_container_p;
    int i;

    (void)mpl_mutex_lock(mutex);
    MPL_LIST_FOR_EACH_SAFE(mpl_pc_list_p, elem_p, tmp_p)
    {
        pc_p = MPL_LIST_CONTAINER(elem_p, mpl_pc_t, list_entry);
        for (i = 0; i < num_scratch_strings; i++)
            free(pc_p->scratch_string[i]);
        free(pc_p);
    }

    mpl_pc_list_p = NULL;

    MPL_LIST_FOR_EACH_SAFE(paramset_list_p, elem_p, tmp_p)
    {
        paramset_container_p = MPL_LIST_CONTAINER(elem_p,
                                                  mpl_paramset_container_t,
                                                  list_entry);

        if (paramset_container_p->paramset_p->is_dynamic_array2) {
            int size = PARAM_SET_SIZE(paramset_container_p->paramset_p);
            for(i=0;i<size;i++) {
                free((void*)paramset_container_p->paramset_p->array2[i].max_p);
                free((void*)paramset_container_p->paramset_p->array2[i].enum_values);
            }
            free((void*)paramset_container_p->paramset_p->array2);
        }
        free(paramset_container_p);
    }

    (void)mpl_mutex_unlock(mutex);
    (void)mpl_mutex_destroy(mutex);
    mpl_threads_deinit();
    return;
}
#endif

mpl_param_element_id_t mpl_param_index_to_paramid(int index, int param_set_id)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(param_set_id, NULL);

    if (NULL == param_descr_p) {
        // [DOC] The specified MPL parameter set was not found
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Parameter set is NULL, param_set_id=%d\n",
                             param_set_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return MPL_PARAM_ID_UNDEFINED;
    }

    if ((index < 0) || (index >= PARAM_SET_SIZE(param_descr_p))) {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Index out of range, index=%d\n",
                             index));
        set_errno(E_MPL_INVALID_PARAMETER);
        return MPL_PARAM_ID_UNDEFINED;
    }
    return INDEX_TO_PARAMID(index, param_descr_p);
}

int mpl_param_num_parameters(int param_set_id)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(param_set_id, NULL);

    if (NULL == param_descr_p) {
        // [DOC] The specified MPL parameter set was not found
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Parameter set is NULL, param_set_id=%d\n",
                             param_set_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return MPL_PARAM_ID_UNDEFINED;
    }
    return PARAM_SET_SIZE(param_descr_p);
}

mpl_param_element_id_t mpl_param_first_paramid(int param_set_id)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(param_set_id, NULL);

    if (NULL == param_descr_p) {
        // [DOC] The specified MPL parameter set was not found
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Parameter set is NULL, param_set_id=%d\n",
                             param_set_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return MPL_PARAM_ID_UNDEFINED;
    }
    return mpl_param_index_to_paramid(0, param_set_id);
}

mpl_param_element_id_t mpl_param_last_paramid(int param_set_id)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(param_set_id, NULL);

    if (NULL == param_descr_p) {
        // [DOC] The specified MPL parameter set was not found
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Parameter set is NULL, param_set_id=%d\n",
                             param_set_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return MPL_PARAM_ID_UNDEFINED;
    }
    return mpl_param_index_to_paramid(PARAM_SET_SIZE(param_descr_p) - 1, param_set_id);
}

const char *mpl_paramset_prefix(int param_set_id)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(param_set_id, NULL);
    if (param_descr_p == NULL)
    {
        // [DOC] The specified MPL parameter set was not found
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    return param_descr_p->paramid_prefix;
}

int
    mpl_param_pack(const mpl_param_element_t* element_p,
                   char *buf,
                   size_t buflen)
{
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    options.no_prefix = false;
    return mpl_param_pack_internal(element_p,
                                   buf,
                                   buflen,
                                   &options);
}

int
    mpl_param_pack_no_prefix(const mpl_param_element_t* element_p,
                             char *buf,
                             size_t buflen)
{
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    options.no_prefix = true;
    return mpl_param_pack_internal(element_p,
                                   buf,buflen,
                                   &options);
}

/**
 * mpl_param_pack - pack a parameter to be sent to psccd (PS Connection
 *                          Control Daemon)
 */
int
    mpl_param_pack_internal(const mpl_param_element_t* element_p,
                            char *buf,
                            size_t buflen,
                            const mpl_pack_options_t *options_p)
{
    int len;
    int tmp_len;
    mpl_param_descr_set_t *param_descr_p;
    const char *field_str_p = NULL;
    char tag_str[5];
    char *child_str_p = NULL;
    mpl_param_element_id_t field_param_id;
    mpl_param_descr_set_t *context_param_descr_p;
    bool no_pfx = options_p->no_prefix;
    bool fm_ctxt = false;
    int child_idx = -1;
    mpl_param_element_id_t outer_param_id;
    mpl_param_descr_set_t *outer_param_descr_p;

    if (NULL == element_p) {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("element_p is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return (-1);
    }

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(element_p->id),
                                  NULL);

    if (NULL == param_descr_p)
    {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return (-1);
    }

    if (!PARAMID_OK(element_p->id, param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Illegal element id: %d\n",
                                                     element_p->id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return (-1);
    }

    if ((element_p->tag < 0) || (element_p->tag > 99))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Illegal element tag: %d\n",
                                                     element_p->tag));
        set_errno(E_MPL_INVALID_PARAMETER);
        return (-1);
    }

    if (element_p->tag > 0)
    {
        sprintf(tag_str, "[%d]", element_p->tag);
    }
    else
    {
        tag_str[0] = 0;
    }

    if (element_p->context != MPL_PARAM_ID_UNDEFINED) {
        context_param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(element_p->context),
                                              NULL);
        if (NULL == context_param_descr_p)
        {
            // [DOC] The specified MPL parameter has an unrecognized parameter set
            MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
            set_errno(E_MPL_INVALID_PARAMETER);
            return (-1);
        }
        if (!PARAMID_OK(element_p->context, context_param_descr_p))
        {
            MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Illegal context: %d\n",
                                                         element_p->context));
            set_errno(E_MPL_INVALID_PARAMETER);
            return (-1);
        }
        field_param_id = get_field_paramid(element_p->context,
                                           element_p->id_in_context,
                                           context_param_descr_p);
        if (field_param_id == MPL_PARAM_ID_UNDEFINED)
        {
            MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Illegal id in context: %d\n",
                                                         element_p->id_in_context));
            set_errno(E_MPL_INVALID_PARAMETER);
            return (-1);
        }
        field_str_p = get_field_name(element_p->context,
                                     element_p->id_in_context,
                                     context_param_descr_p);
        assert(field_str_p != NULL);

        outer_param_descr_p = context_param_descr_p;
        outer_param_id = element_p->context;

        fm_ctxt = (options_p->field_pack_mode == field_pack_mode_context);

        if (element_p->id != field_param_id) {
            /* Element is a child */
            child_idx = get_child_index(field_param_id,
                                        element_p->id,
                                        paramset_find(MPL_PARAMID_TO_PARAMSET(field_param_id),
                                                      NULL));
            if (child_idx < 0) {
                MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Param '%s.%s' is not child of '%s.%s'\n",
                                                             mpl_param_id_get_prefix(element_p->id),
                                                             mpl_param_id_get_string(element_p->id),
                                                             mpl_param_id_get_prefix(field_param_id),
                                                             mpl_param_id_get_string(field_param_id)));
                set_errno(E_MPL_INVALID_PARAMETER);
                return (-1);
            }
            else {
                mpl_param_element_t *tmp_p;
                mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
                tmp_p = mpl_param_element_create_empty_tag(element_p->id,
                                                           0);
                if (tmp_p == NULL)
                    return -1;
                if (MPL_PARAMID_TO_PARAMSET(element_p->id) ==
                    MPL_PARAMID_TO_PARAMSET(element_p->context)) {
                    options.no_prefix = true;
                }
                len = mpl_param_pack_internal(tmp_p, NULL, 0, &options);
                if (len < 0) {
                    mpl_param_element_destroy(tmp_p);
                    return -1;
                }
                child_str_p = calloc(1, len+1);
                if (child_str_p == NULL) {
                    MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                                        ("Failed allocating memory\n"));
                    set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
                    mpl_param_element_destroy(tmp_p);
                    return -1;
                }
                if (mpl_param_pack_internal(tmp_p,
                                            child_str_p,
                                            len+1,
                                            &options) != len) {
                    mpl_param_element_destroy(tmp_p);
                    free(child_str_p);
                    return -1;
                }
                mpl_param_element_destroy(tmp_p);
            }
        }
    }
    else {
        outer_param_descr_p = param_descr_p;
        outer_param_id = element_p->id;
    }

    len = snprintf(buf, buflen, "%s%s%s%s%s%s%s%s%s",
                   (fm_ctxt || no_pfx) ? "" : outer_param_descr_p->paramid_prefix,
                   (fm_ctxt || no_pfx) ? "" : ".",
                   fm_ctxt ? "" : outer_param_descr_p->array[PARAMID_TO_INDEX(outer_param_id)].name,
                   fm_ctxt ? "" : field_str_p != NULL ? "%" : "",
                   field_str_p != NULL ? field_str_p : "",
                   child_str_p ? "(" : "",
                   child_str_p ? child_str_p : "",
                   child_str_p ? ")" : "",
                   tag_str);
    if (child_str_p)
        free(child_str_p);

    if (len < 0)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,("snprintf failed\n"));
        set_errno(E_MPL_FAILED_OPERATION);
        return (len);
    }

    /* We also accept no value (used by e.g. 'get' command) */
    if (NULL == element_p->value_p)
        return (len);

    if (len >= (int)buflen)
        tmp_len =
            (*param_descr_p->array[PARAMID_TO_INDEX(element_p->id)].pack_func)
            (element_p->value_p,
             NULL,
             0,
             &param_descr_p->array2[PARAMID_TO_INDEX(element_p->id)],
             options_p);
    else
    {
        assert(buf != NULL);
        tmp_len =
            (*param_descr_p->array[PARAMID_TO_INDEX(element_p->id)].pack_func)
            (element_p->value_p,
             buf+len,
             buflen-len,
             &param_descr_p->array2[PARAMID_TO_INDEX(element_p->id)],
             options_p);/*lint !e413 buf is not NULL */
    }
    if (tmp_len < 0)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Param value pack failed for %s%s%s\n",
                             mpl_param_id_get_string(element_p->id),
                             field_str_p != NULL ? "%" : "",
                             field_str_p != NULL ? field_str_p : ""
                            ));
        return (tmp_len);
    }

    len += tmp_len;

    return (len);
}


int
    mpl_param_unpack(const char* id_str,
                     const char* value_str,
                     mpl_param_element_t** element_pp)
{
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    return mpl_param_unpack_internal(id_str,
                                     value_str,
                                     element_pp,
                                     &options,
                                     MPL_PARAM_ID_UNDEFINED);
}

int
    mpl_param_unpack_param_set(const char* id_str,
                               const char* value_str,
                               mpl_param_element_t** element_pp,
                               int param_set_id)
{
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    options.param_set_id = param_set_id;
    return mpl_param_unpack_internal(id_str,
                                     value_str,
                                     element_pp,
                                     &options,
                                     MPL_PARAM_ID_UNDEFINED);
}

/**
 * mpl_param_unpack - unpack received parameter strings
 */
int
    mpl_param_unpack_internal(const char* id_str,
                              const char* value_str,
                              mpl_param_element_t** element_pp,
                              const mpl_pack_options_t *options_p,
                              mpl_param_element_id_t unpack_context)
{
    int id, res;
    mpl_param_element_t* tmp_p;
    mpl_param_descr_set_t *param_descr_p = NULL;
    mpl_list_t *elem_p;
    int prefix_len;
    int size;
    int tag = 0;
    const char *name_str = NULL;
    size_t name_strlen = 0;
    const char *field_str = NULL;
    size_t field_strlen = 0;
    const char *child_str = NULL;
    size_t child_strlen = 0;
    const char *tag_str = NULL;
    size_t tag_strlen = 0;
    mpl_param_element_id_t child_id = MPL_PARAM_ID_UNDEFINED;

    if (NULL == element_pp)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("element_pp is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return (-1);
    }

    (void)mpl_mutex_lock(mutex);
    MPL_LIST_FOR_EACH(paramset_list_p, elem_p)
    {
        param_descr_p = MPL_LIST_CONTAINER(elem_p,
                                           mpl_paramset_container_t,
                                           list_entry)->paramset_p;
        prefix_len = strlen(param_descr_p->paramid_prefix);
        if (!strncmp(param_descr_p->paramid_prefix, id_str, prefix_len) &&
            ('.' == id_str[prefix_len]))
        {
            id_str += (prefix_len + 1);
            break;
        }
        param_descr_p = NULL;
    }
    (void)mpl_mutex_unlock(mutex);

    if (NULL == param_descr_p)
    {
        /* No prefix perhaps? */
        if (options_p->param_set_id > 0)
        {
            param_descr_p = paramset_find(options_p->param_set_id, NULL);
        }

        /* Unpack context set? */
        if (unpack_context != MPL_PARAM_ID_UNDEFINED)
        {
            if (param_descr_p == NULL)
                param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(unpack_context), NULL);
            if (param_descr_p != NULL) {
                name_str = param_descr_p->array[PARAMID_TO_INDEX(unpack_context)].name;
                name_strlen = strlen(name_str);
            }
        }
    }

    if (NULL == param_descr_p)
    {
        // [DOC] The specified MPL parameter set was not found
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return (-1);
    }

    tag_str = get_bracket_contents(id_str, '[', ']', &tag_strlen);
    child_str = get_bracket_contents(id_str, '(', ')', &child_strlen);
    field_str = strchr(id_str, '%');
    if (field_str != NULL) {
        field_str++;
        if (child_str != NULL)
            field_strlen = child_str - field_str - 1;
        else if (tag_str != NULL)
            field_strlen = tag_str - field_str - 1;
        else
            field_strlen = strlen(field_str);
    }
    if (name_str != NULL)
    {
        if (field_str != NULL)
        {
            /* mutual exclusive */
            MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Field separator found in context packed format\n"));
            set_errno(E_MPL_INVALID_PARAMETER);
            return (-1);
        }
        field_str = id_str;
        if (child_str != NULL)
            field_strlen = child_str - field_str - 1;
        else if (tag_str != NULL)
            field_strlen = tag_str - field_str - 1;
        else
            field_strlen = strlen(field_str);
        if (get_field_from_name(field_str,
                                field_strlen,
                                param_descr_p->array2[PARAMID_TO_INDEX(unpack_context)].field_values,
                                param_descr_p->array2[PARAMID_TO_INDEX(unpack_context)].field_values_size
                               ) == NULL) {
            /* Not recognized as a field in the context, back off to normal type decoding */
            field_str = NULL;
            field_strlen = 0;
            name_str = NULL;
            name_strlen = 0;
        }
    }

    if (name_str == NULL) {
        name_str = id_str;
        if (field_str != NULL)
            name_strlen = field_str - id_str - 1;
        else if (child_str != NULL)
            name_strlen = child_str - id_str - 1;
        else if (tag_str != NULL)
            name_strlen = tag_str - id_str - 1;
        else
            name_strlen = strlen(id_str);
    }

    if (tag_str != NULL)
    {
        if (sscanf(tag_str, "%d", &tag) != 1)
        {
            MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                                ("Invalid tag number format: %s\n",
                                 tag_str));
            set_errno(E_MPL_INVALID_PARAMETER);
            return (-1);
        }

        if ((tag < 0) || (tag > 99))
        {
            MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                                ("Invalid tag number value: %d\n", tag));
            set_errno(E_MPL_INVALID_PARAMETER);
            return (-1);
        }
    }

    if (child_str != NULL) {
        mpl_param_element_t *child_elem_p;
        char *tmpstr = calloc(1, child_strlen + 1);
        if (tmpstr == NULL) {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                                ("Failed allocating memory\n"));
            set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
            return -1;
        }
        strncpy(tmpstr, child_str, child_strlen);
        if (mpl_param_unpack_param_set(tmpstr,
                                       NULL,
                                       &child_elem_p,
                                       param_descr_p->param_set_id) < 0) {
            MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                                ("Invalid child: %s\n", tmpstr));
            free(tmpstr);
            set_errno(E_MPL_INVALID_PARAMETER);
            return (-1);
        }
        child_id = child_elem_p->id;
        free(tmpstr);
        mpl_param_element_destroy(child_elem_p);
    }

    size = PARAM_SET_SIZE(param_descr_p);

    for (id=0; id < size; id++)
    {
        int field_id = -1;
        int eff_param_id;
        mpl_param_descr_set_t *eff_param_descr_p;

        if ((strlen(param_descr_p->array[id].name) == name_strlen) &&
            (0 == strncmp(param_descr_p->array[id].name, name_str, name_strlen)))
        {
            if (field_strlen != 0)
            {
                const mpl_field_value_t *field_value_p;
                field_value_p = get_field_from_name(field_str,
                                                    field_strlen,
                                                    param_descr_p->array2[id].field_values,
                                                    param_descr_p->array2[id].field_values_size
                                                   );
                if (field_value_p == NULL)
                {
                    MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                                        ("Invalid field: %s\n", field_str));
                    set_errno(E_MPL_INVALID_PARAMETER);
                    return (-1);
                }
                if (child_id != MPL_PARAM_ID_UNDEFINED) {
                    if (mpl_param_get_child_index(field_value_p->param_id, child_id) < 0) {
                        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Param '%s.%s' is not child of '%s.%s'\n",
                                                                     mpl_param_id_get_prefix(child_id),
                                                                     mpl_param_id_get_string(child_id),
                                                                     mpl_param_id_get_prefix(field_value_p->param_id),
                                                                     mpl_param_id_get_string(field_value_p->param_id)));
                        set_errno(E_MPL_INVALID_PARAMETER);
                        return (-1);
                    }
                    eff_param_id = child_id;
                }
                else {
                    eff_param_id = field_value_p->param_id;
                }
                field_id = field_value_p->field_id;
            }
            else
                eff_param_id = INDEX_TO_PARAMID(id, param_descr_p);

            eff_param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(eff_param_id),
                                              NULL);
            assert(eff_param_descr_p);

            tmp_p =
                mpl_param_element_create_empty_tag(eff_param_id, tag);
            if (NULL == tmp_p)
            {
                return (-1);
            }

            if (field_id >= 0) {
                tmp_p->context = get_field_context_paramid(INDEX_TO_PARAMID(id, param_descr_p),
                                                           field_id,
                                                           param_descr_p);
                tmp_p->id_in_context = field_id;
            }

            /* We also accept no value (used by 'get' command) */
            if (NULL == value_str)
            {
                *element_pp = tmp_p;
                return (0);
            }

            assert(NULL != eff_param_descr_p->array[PARAMID_TO_INDEX(eff_param_id)].unpack_func);

            res = (*eff_param_descr_p->array[PARAMID_TO_INDEX(eff_param_id)].unpack_func)
                  (value_str,
                   &tmp_p->value_p,
                   &eff_param_descr_p->array2[PARAMID_TO_INDEX(eff_param_id)],
                   options_p,
                   eff_param_id);

            if (res < 0)
            {
                mpl_param_element_destroy(tmp_p);
                MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                    ("Param value unpack failed for %s=%s\n",
                                     id_str, value_str));
                return (res);
            }

            *element_pp = tmp_p;
            return (0);
        }
    }

    return (-1);
}

/**
 * mpl_param_id_get_string
 */
const char *
    mpl_param_id_get_string(mpl_param_element_id_t param_id)
{
    mpl_param_descr_set_t *param_descr_p;
    char *id_get_string;
    int len;
    mpl_param_element_t *tmp_elem_p;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(param_id), NULL);

    if (NULL == param_descr_p) {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return "<Error: no parameter set>";
    }

    if (!PARAMID_OK(param_id, param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Unknown parameter id\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return "<Error: unknown param>";
    }

    tmp_elem_p = mpl_param_element_create_empty(MPL_PARAMID_VIRTUAL_CLEAR(param_id));
    if (tmp_elem_p == NULL)
        return "<Error: no memory>";

    len = mpl_param_pack_no_prefix(tmp_elem_p,
                                   NULL,
                                   0);

    id_get_string = get_scratch_string(len + 1);
    if (NULL == id_get_string)
    {
        mpl_param_element_destroy(tmp_elem_p);
        return "<Error: no scratch string available>";
    }

    len = mpl_param_pack_no_prefix(tmp_elem_p,
                                   id_get_string,
                                   len + 1);
    mpl_param_element_destroy(tmp_elem_p);
    return (const char *) id_get_string;
}

/**
 * mpl_param_id_get_prefix
 */
const char *
    mpl_param_id_get_prefix(mpl_param_element_id_t param_id)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(param_id), NULL);

    if (NULL == param_descr_p) {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return "<Error: no parameter set>";
    }

    return (const char *) param_descr_p->paramid_prefix;
}

/**
 * mpl_param_id_get_type
 */
mpl_type_t
    mpl_param_id_get_type(mpl_param_element_id_t param_id)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(param_id), NULL);

    if (NULL == param_descr_p) {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return mpl_type_invalid;
    }

    if (!PARAMID_OK(param_id, param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Unknown parameter id\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return mpl_type_invalid;
    }

    return param_descr_p->array[PARAMID_TO_INDEX(param_id)].type;
}

/**
 * mpl_param_id_get_type_string
 */
const char *
    mpl_param_id_get_type_string(mpl_param_element_id_t param_id)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(param_id), NULL);

    if (NULL == param_descr_p) {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return "<Error: no parameter set>";
    }

    if (!PARAMID_OK(param_id, param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Unknown parameter id\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return "<unknown param>";
    }

    return mpl_type_names[param_descr_p->array[PARAMID_TO_INDEX(param_id)].type];
}

/**
 * mpl_param_id_sizeof_param_value
 */
size_t
    mpl_param_id_sizeof_param_value(mpl_param_element_id_t param_id)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(param_id), NULL);

    if (NULL == param_descr_p)
    {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Parameter set is NULL (paramid=%x)\n", param_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return 0;
    }

    return (*param_descr_p->array[PARAMID_TO_INDEX(param_id)].sizeof_func)
        (&param_descr_p->array2[PARAMID_TO_INDEX(param_id)]);
}

/**
 * mpl_param_value_get_string
 *
 * Description: Get string corresponding to the parameter value
 *
 * Parameters:
 *     param_id:     Param ID
 *     value_p:      Param value
 *
 * Return Values : String
 *
 */
const char *
    mpl_param_value_get_string(mpl_param_element_id_t param_id,
                               void* value_p)
{
    int res;
    mpl_param_descr_set_t *param_descr_p;
    char *value_get_string;
    int len;
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(param_id), NULL);


    if (NULL == param_descr_p) {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return "<Error: no parameter set>";
    }

    if (!PARAMID_OK(param_id, param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Unknown parameter id\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return "<unknown param>";
    }

    /* No value */
    if (NULL == value_p)
        return "<no value>";

    res = (*param_descr_p->array[PARAMID_TO_INDEX(param_id)].pack_func)
          (value_p,
           NULL,
           0,
           &param_descr_p->array2[PARAMID_TO_INDEX(param_id)],
           &options);

    len = res + 1;
    value_get_string = get_scratch_string(len);
    if (NULL == value_get_string)
    {
        return "<Error: no scratch string available>";
    }

    res = (*param_descr_p->array[PARAMID_TO_INDEX(param_id)].pack_func)
          (value_p,
           value_get_string,
           len,
           &param_descr_p->array2[PARAMID_TO_INDEX(param_id)],
           &options);

    if (res <= 0)
        return "<unknown value>";

    /* Remove '=' character!!! */
    return &value_get_string[1];
}

int mpl_param_value_get_range_id(mpl_param_element_id_t param_id,
                                 void* value_p)
{
    mpl_param_descr_set_t *param_descr_p;
    const char *value_str;
    int res;
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    void *dummy_p;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(param_id), NULL);

    if (NULL == param_descr_p)
    {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Parameter set is NULL (paramid=%x)\n", param_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return -1;
    }

    if (!PARAMID_OK(param_id, param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Unknown parameter id\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return -1;
    }

    value_str = mpl_param_value_get_string(param_id, value_p);

    res = (*param_descr_p->array[PARAMID_TO_INDEX(param_id)].unpack_func)
          (value_str,
           &dummy_p,
           &param_descr_p->array2[PARAMID_TO_INDEX(param_id)],
           &options,
           MPL_PARAM_ID_UNDEFINED);

    if (res >= 0)
        param_descr_p->array[PARAMID_TO_INDEX(param_id)].free_func(dummy_p);

    return res;
}

int mpl_param_get_child_id(mpl_param_element_id_t parent_param_id,
                           int child_index)
{
    mpl_param_descr_set_t *parent_param_descr_p;

    parent_param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(parent_param_id), NULL);

    if (NULL == parent_param_descr_p)
    {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Parameter set is NULL (paramid=%x)\n", parent_param_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return -1;
    }

    if (!PARAMID_OK(parent_param_id, parent_param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Unknown parameter id\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return -1;
    }

    return get_child_paramid(parent_param_id, child_index, parent_param_descr_p);
}

int mpl_param_get_child_index(mpl_param_element_id_t parent_param_id,
                              mpl_param_element_id_t child_param_id)
{
    mpl_param_descr_set_t *parent_param_descr_p;

    parent_param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(parent_param_id), NULL);
    if (NULL == parent_param_descr_p)
    {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Parameter set is NULL (paramid=%x)\n", parent_param_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return -1;
    }
    if (!PARAMID_OK(parent_param_id, parent_param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Unknown parameter id\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return -1;
    }

    return get_child_index(parent_param_id, child_param_id, parent_param_descr_p);
}

mpl_param_element_id_t mpl_param_get_bag_field_id(mpl_param_element_id_t bag_param_id,
                                                  int field_index)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(bag_param_id), NULL);

    if (NULL == param_descr_p)
    {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Parameter set is NULL (paramid=%x)\n", bag_param_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return MPL_PARAM_ID_UNDEFINED;
    }

    if (!PARAMID_OK(bag_param_id, param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Unknown parameter id\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return MPL_PARAM_ID_UNDEFINED;
    }

    return get_field_paramid(bag_param_id, field_index, param_descr_p);
}

mpl_param_element_id_t mpl_param_get_bag_field_context(mpl_param_element_id_t bag_param_id,
                                                       int field_index)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(bag_param_id), NULL);

    if (NULL == param_descr_p)
    {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Parameter set is NULL (paramid=%x)\n", bag_param_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return MPL_PARAM_ID_UNDEFINED;
    }

    if (!PARAMID_OK(bag_param_id, param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Unknown parameter id\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return MPL_PARAM_ID_UNDEFINED;
    }

    return get_field_context_paramid(bag_param_id, field_index, param_descr_p);
}

const char *mpl_param_get_bag_field_name(mpl_param_element_id_t bag_param_id,
                                         int field_index)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(bag_param_id), NULL);

    if (NULL == param_descr_p)
    {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Parameter set is NULL (paramid=%x)\n", bag_param_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    if (!PARAMID_OK(bag_param_id, param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Unknown parameter id\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    return get_field_name(bag_param_id, field_index, param_descr_p);
}

/**
 * mpl_param_value_copy_out
 *
 */
int
    mpl_param_value_copy_out(const mpl_param_element_t* element_p,
                             void *to_p,
                             int size)
{
    mpl_param_descr_set_t *param_descr_p;

    if ((NULL == element_p) ||
        ((0 != size) && (NULL == to_p)))
    {
        return -2;
    }

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(element_p->id),
                                  NULL);

    if (NULL == param_descr_p)
    {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return -2;
    }

    if (NULL == element_p->value_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter value is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return -2;
    }

    size = (*param_descr_p->array[PARAMID_TO_INDEX(element_p->id)].copy_func)
           (to_p,
            element_p->value_p,
            size,
            &param_descr_p->array2[PARAMID_TO_INDEX(element_p->id)]);

    if (size < 0)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_OPERATION,
                            ("Param value copy not supported for %s\n",
                             mpl_param_id_get_string(element_p->id)));
    }

    return size;
}

/**
 * mpl_param_allow_get_bl
 */
bool
    mpl_param_allow_get_bl(mpl_param_element_id_t param_id,
                           mpl_blacklist_t blacklist)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(param_id), NULL);

    if (NULL == param_descr_p) {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return false;
    }

    if (!PARAMID_OK(param_id, param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Unknown parameter id\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return false;
    }

    /* Is the parameter in the blacklist? */
    if (mpl_param_list_find(param_id, blacklist))
        return false;

    return param_descr_p->array[PARAMID_TO_INDEX(param_id)].allow_get;
}

/**
 * mpl_param_allow_set_bl
 */
bool
    mpl_param_allow_set_bl(mpl_param_element_id_t param_id,
                           mpl_blacklist_t blacklist)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(param_id), NULL);

    if (NULL == param_descr_p) {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,("Parameter set is NULL\n"));
        return false;
    }

    if (!PARAMID_OK(param_id, param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Unknown parameter id\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return false;
    }

    /* Is the parameter in the blacklist? */
    if (mpl_param_list_find(param_id, blacklist))
        return false;

    return param_descr_p->array[PARAMID_TO_INDEX(param_id)].allow_set;
}

/**
 * mpl_param_allow_config_bl
 */
bool
    mpl_param_allow_config_bl(mpl_param_element_id_t param_id,
                              mpl_blacklist_t blacklist)
{
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(param_id), NULL);

    if (NULL == param_descr_p) {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Parameter set is NULL, param_id=%x\n",
                             param_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return false;
    }

    if (!PARAMID_OK(param_id, param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Unknown parameter id\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return false;
    }

    /* Is the parameter in the blacklist? */
    if (mpl_param_list_find(param_id, blacklist))
        return false;

    return param_descr_p->array[PARAMID_TO_INDEX(param_id)].allow_config;
}

/**
 * mpl_param_element_create_stringn_tag
 *
 */
mpl_param_element_t*
    mpl_param_element_create_stringn_tag(mpl_param_element_id_t param_id,
                                         int tag,
                                         const char* value,
                                         size_t len)
{
    mpl_param_element_t* element_p;
    size_t size;

    size = len + 1;

    element_p = mpl_param_element_create_empty_tag(param_id, tag);
    if (NULL == element_p)
    {
        return NULL;
    }

    element_p->value_p = malloc(size);
    if (NULL == element_p->value_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        mpl_param_element_destroy(element_p);
        return NULL;
    }

    strncpy((char*)element_p->value_p, value, len);

    // add null termination
    ((char*)element_p->value_p)[len] = 0;

    return (element_p);
}

/**
 * mpl_param_element_create_wstringn_tag
 *
 */
mpl_param_element_t*
    mpl_param_element_create_wstringn_tag(mpl_param_element_id_t param_id,
                                          int tag,
                                          const wchar_t* value,
                                          size_t len)
{
    mpl_param_element_t* element_p;
    size_t size;

    size = len + 1;

    element_p = mpl_param_element_create_empty_tag(param_id, tag);
    if (NULL == element_p)
    {
        return NULL;
    }

    element_p->value_p = malloc(size * sizeof(L'\0'));
    if (NULL == element_p->value_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        mpl_param_element_destroy(element_p);
        return NULL;
    }

    (void)wcsncpy((wchar_t*)element_p->value_p, value, size);

    // add null termination
    ((wchar_t*)element_p->value_p)[len] = L'\0';

    return (element_p);
}

/**
 * mpl_param_element_create_empty_tag
 *
 */
mpl_param_element_t*
    mpl_param_element_create_empty_tag(mpl_param_element_id_t param_id, int tag)
{
    mpl_param_element_t* element_p;
    mpl_param_descr_set_t *param_descr_p;

    if (MPL_PARAMID_IS_VIRTUAL(param_id))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter is VIRTUAL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(param_id), NULL);

    if (NULL == param_descr_p) {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    if (!PARAMID_OK(param_id, param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Unknown parameter ID: %x\n", param_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return (NULL);
    }

    if ((tag < 0) || (tag > 99))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Illegal tag value: %d\n", tag));
        set_errno(E_MPL_INVALID_PARAMETER);
        return (NULL);
    }

    element_p = calloc(1, sizeof(mpl_param_element_t));
    if (NULL == element_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return NULL;
    }

    element_p->id = param_id;
    element_p->tag = tag;
    element_p->value_p = NULL;
    element_p->list_entry.next_p = NULL;

    return (element_p);
}

/**
 * mpl_param_element_create_n_tag
 *
 */
mpl_param_element_t*
    mpl_param_element_create_n_tag(mpl_param_element_id_t param_id,
                                   int tag,
                                   const void* value_p,
                                   size_t len)
{
    mpl_param_descr_set_t *param_descr_p;
    size_t size;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(param_id), NULL);

    if (NULL == param_descr_p)
    {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Parameter set is NULL (paramid=%x)\n", param_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    size = (*param_descr_p->array[PARAMID_TO_INDEX(param_id)].sizeof_func)
           (&param_descr_p->array2[PARAMID_TO_INDEX(param_id)]);

    if ((size > 0) && (size != len))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Param length check failed for %s\n",
                             mpl_param_id_get_string(param_id)));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    return mpl_param_element_create_tag(param_id, tag, value_p);
}

/**
 * mpl_param_element_create_uint8_array_tag
 *
 */
mpl_param_element_t*
    mpl_param_element_create_uint8_array_tag(mpl_param_element_id_t param_id,
                                             int tag,
                                             const uint8_t* value, size_t size)
{
    mpl_param_element_t* element_p;
    mpl_uint8_array_t *p;

    p = malloc(sizeof(mpl_uint8_array_t));
    if(NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return NULL;
    }

    p->arr_p = malloc(size * sizeof(uint8_t));
    if (NULL == p->arr_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(p);
        return NULL;
    }

    element_p = mpl_param_element_create_empty_tag(param_id, tag);
    if (NULL == element_p)
    {
        free(p->arr_p);
        free(p);
        return NULL;
    }

    element_p->value_p = p;
    p->len = size;
    memcpy(p->arr_p, value, size * sizeof(uint8_t));
    return (element_p);
}


/**
 * mpl_param_element_create_uint16_array_tag
 *
 */
mpl_param_element_t*
    mpl_param_element_create_uint16_array_tag(mpl_param_element_id_t param_id,
                                              int tag,
                                              const uint16_t* value, size_t size)
{
    mpl_param_element_t* element_p;
    mpl_uint16_array_t *p;

    p = malloc(sizeof(mpl_uint16_array_t));
    if(NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return NULL;
    }

    p->arr_p = malloc(size * sizeof(uint16_t));
    if (NULL == p->arr_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(p);
        return NULL;
    }

    element_p = mpl_param_element_create_empty_tag(param_id, tag);
    if (NULL == element_p)
    {
        free(p->arr_p);
        free(p);
        return NULL;
    }

    element_p->value_p = p;
    p->len = size;
    memcpy(p->arr_p, value, size * sizeof(uint16_t));
    return (element_p);
}


/**
 * mpl_param_element_create_uint32_array_tag
 *
 */
mpl_param_element_t*
    mpl_param_element_create_uint32_array_tag(mpl_param_element_id_t param_id,
                                              int tag,
                                              const uint32_t* value, size_t size)
{
    mpl_param_element_t* element_p;
    mpl_uint32_array_t *p;

    p = malloc(sizeof(mpl_uint32_array_t));
    if(NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return NULL;
    }

    p->arr_p = malloc(size * sizeof(uint32_t));
    if (NULL == p->arr_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(p);
        return NULL;
    }

    element_p = mpl_param_element_create_empty_tag(param_id, tag);
    if (NULL == element_p)
    {
        free(p->arr_p);
        free(p);
        return NULL;
    }

    element_p->value_p = p;
    p->len = size;
    memcpy(p->arr_p, value, size * sizeof(uint32_t));
    return (element_p);
}


/**
 * mpl_param_element_create_string_tuple_tag
 *
 */
mpl_param_element_t*
    mpl_param_element_create_string_tuple_tag(mpl_param_element_id_t param_id,
                                              int tag,
                                              const char* key, const char *val)
{
    mpl_param_element_t* element_p;
    mpl_string_tuple_t *p;

    p = malloc(sizeof(mpl_string_tuple_t));
    if(NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return NULL;
    }

    p->key_p = malloc(strlen(key) + 1);
    if (NULL == p->key_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(p);
        return NULL;
    }

    p->value_p = malloc(strlen(val) + 1);
    if (NULL == p->value_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(p->key_p);
        free(p);
        return NULL;
    }

    element_p = mpl_param_element_create_empty_tag(param_id, tag);
    if (NULL == element_p)
    {
        free(p->key_p);
        free(p->value_p);
        free(p);
        return NULL;
    }

    element_p->value_p = p;
    strcpy(p->key_p, key);
    strcpy(p->value_p, val);
    return (element_p);
}


/**
 * mpl_param_element_create_int_tuple_tag
 *
 */
mpl_param_element_t*
    mpl_param_element_create_int_tuple_tag(mpl_param_element_id_t param_id,
                                           int tag,
                                           int key, int val)
{
    mpl_param_element_t* element_p;
    mpl_int_tuple_t *p;

    p = malloc(sizeof(mpl_int_tuple_t));
    if(NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return NULL;
    }

    element_p = mpl_param_element_create_empty_tag(param_id, tag);
    if (NULL == element_p)
    {
        free(p);
        return NULL;
    }

    element_p->value_p = p;
    p->key = key;
    p->value = val;
    return (element_p);
}


/**
 * mpl_param_element_create_strint_tuple_tag
 *
 */
mpl_param_element_t*
    mpl_param_element_create_strint_tuple_tag(mpl_param_element_id_t param_id,
                                              int tag,
                                              const char* key, int val)
{
    mpl_param_element_t* element_p;
    mpl_strint_tuple_t *p;

    p = malloc(sizeof(mpl_strint_tuple_t));
    if(NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return NULL;
    }

    p->key_p = malloc(strlen(key) + 1);
    if (NULL == p->key_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(p);
        return NULL;
    }

    element_p = mpl_param_element_create_empty_tag(param_id, tag);
    if (NULL == element_p)
    {
        free(p->key_p);
        free(p);
        return NULL;
    }

    element_p->value_p = p;
    strcpy(p->key_p, key);
    p->value = val;
    return (element_p);
}


/**
 * mpl_param_element_create_struint8_tuple_tag
 *
 */
mpl_param_element_t*
    mpl_param_element_create_struint8_tuple_tag(mpl_param_element_id_t param_id,
                                                int tag,
                                                const char* key, uint8_t val)
{
    mpl_param_element_t* element_p;
    mpl_struint8_tuple_t *p;

    p = malloc(sizeof(mpl_struint8_tuple_t));
    if(NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return NULL;
    }

    p->key_p = malloc(strlen(key) + 1);
    if (NULL == p->key_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(p);
        return NULL;
    }

    element_p = mpl_param_element_create_empty_tag(param_id, tag);
    if (NULL == element_p)
    {
        free(p->key_p);
        free(p);
        return NULL;
    }

    element_p->value_p = p;
    strcpy(p->key_p, key);
    p->value = val;
    return (element_p);
}


#ifdef UNIT_TEST
typedef struct
{
    bool param_test_active;
    mpl_param_element_id_t param_id;
    MPL_ErrorCode_t errno_code;
} mpl_unittest_response_t;

static mpl_unittest_response_t mpl_param_unittest;


void unittest_force_mpl_init(void)
{
    mpl_param_unittest.param_test_active = FALSE;
}

void unittest_force_mpl_set(mpl_param_element_id_t param_id,
                            MPL_ErrorCode_t errno_code)
{
    mpl_param_unittest.param_test_active = TRUE;
    mpl_param_unittest.param_id = param_id;
    mpl_param_unittest.errno_code = errno_code;
}


bool unittest_force_mpl_errno_response(mpl_param_element_id_t param_id,
                                       MPL_ErrorCode_t *errno_code)
{
    if (mpl_param_unittest.param_test_active == FALSE)
        return FALSE;
    if (param_id != mpl_param_unittest.param_id)
        return FALSE;
    *errno_code = mpl_param_unittest.errno_code;
    return TRUE;
}
#endif

/**
 * mpl_param_element_create_tag
 */
mpl_param_element_t*
    mpl_param_element_create_tag(mpl_param_element_id_t param_id,
                                 int tag,
                                 const void* value_p)
{
    mpl_param_element_t* element_p;
    mpl_param_descr_set_t *param_descr_p;
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;

#ifdef UNIT_TEST
    {
        MPL_ErrorCode_t error_code;
        if (unittest_force_mpl_errno_response(param_id, &error_code))
        {
            set_errno(error_code);
            return NULL;
        }
    }
#endif


    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(param_id), NULL);

    if (NULL == param_descr_p)
    {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    element_p = mpl_param_element_create_empty_tag(param_id, tag);
    if (NULL == element_p)
    {
        return NULL;
    }

    if (NULL != value_p)
    {
        int res;
        char *scratch_string_p;
        int len;
        void *scratch_value_p = NULL;

        res = (*param_descr_p->array[PARAMID_TO_INDEX(param_id)].pack_func)
              (value_p,
               NULL,
               0,
               &param_descr_p->array2[PARAMID_TO_INDEX(param_id)],
               &options);

        if (res < 0)
        {
            MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                                ("Param value pack failed for %s\n",
                                 mpl_param_id_get_string(param_id)));
            mpl_param_element_destroy(element_p);
            return NULL;
        }

        len = res + 1;
        scratch_string_p = get_scratch_string(len);
        if (NULL == scratch_string_p)
        {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Could not get scratch string\n"));
            set_errno(E_MPL_FAILED_OPERATION);
            mpl_param_element_destroy(element_p);
            return NULL;
        }

        res = (*param_descr_p->array[PARAMID_TO_INDEX(param_id)].pack_func)
              (value_p,
               scratch_string_p,
               len,
               &param_descr_p->array2[PARAMID_TO_INDEX(param_id)],
               &options);

        if (res >= 0)
        {
            mpl_param_element_id_t ctxt;
            if (param_descr_p->array[PARAMID_TO_INDEX(param_id)].type == mpl_type_bag)
                ctxt = param_id;
            else
                ctxt = MPL_PARAM_ID_UNDEFINED;

            res = (*param_descr_p->array[PARAMID_TO_INDEX(param_id)].unpack_func)
                  (&scratch_string_p[1],
                   &scratch_value_p,
                   &param_descr_p->array2[PARAMID_TO_INDEX(param_id)],
                   &options,
                   ctxt);
            (*param_descr_p->array[PARAMID_TO_INDEX(param_id)].free_func)
                (scratch_value_p);
        }

        if (res < 0)
        {
            MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                                ("Param value check failed for %s\n",
                                 mpl_param_id_get_string(param_id)));
            mpl_param_element_destroy(element_p);
            return NULL;
        }

        res = (*param_descr_p->array[PARAMID_TO_INDEX(param_id)].clone_func)
              (&element_p->value_p,
               value_p,
               &param_descr_p->array2[PARAMID_TO_INDEX(param_id)]);
        if (res < 0)
        {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Param value clone failed for %s\n",
                                 mpl_param_id_get_string(param_id)));
            mpl_param_element_destroy(element_p);
            return NULL;
        }
    }

    return (element_p);
}


/**
 * mpl_param_element_set_tag
 */
int
    mpl_param_element_set_tag(mpl_param_element_t* element_p, int tag)
{
    if ((tag < 0) || (tag > 99))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Illegal tag value: %d\n", tag));
        set_errno(E_MPL_INVALID_PARAMETER);
        return -1;
    }
    element_p->tag = tag;
    return 0;
}

/**
 * mpl_param_element_clone
 */
mpl_param_element_t*
    mpl_param_element_clone(const mpl_param_element_t* element_p)
{
    mpl_param_element_t* new_element_p;
    int res;
    mpl_param_descr_set_t *param_descr_p;

    if (NULL == element_p)
        return NULL;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(element_p->id),
                                  NULL);

    if (NULL == param_descr_p)
    {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    new_element_p = mpl_param_element_create_empty_tag(element_p->id,
                                                       element_p->tag);
    if (NULL == new_element_p)
    {
        return NULL;
    }
    MPL_PARAM_ELEMENT_SET_FIELD_INFO(new_element_p,
                                     element_p->context,
                                     element_p->id_in_context);
    if (NULL != element_p->value_p)
    {
        res = (*param_descr_p->array[PARAMID_TO_INDEX(element_p->id)].clone_func)
              (&new_element_p->value_p,
               element_p->value_p,
               &param_descr_p->array2[PARAMID_TO_INDEX(element_p->id)]);
        if (res < 0)
        {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Param value clone failed for %s\n",
                                 mpl_param_id_get_string(element_p->id)));
            mpl_param_element_destroy(new_element_p);
            return NULL;
        }
    }

    return new_element_p;
}


/**
 * mpl_param_element_compare
 */
int
    mpl_param_element_compare(const mpl_param_element_t* element1_p,
                              const mpl_param_element_t* element2_p)
{
    mpl_param_descr_set_t *param_descr_p;

    if (NULL == element1_p)
        return -1;

    if (NULL == element2_p)
        return -1;

    if (element1_p->id != element2_p->id)
        return -1;

    if (element1_p->tag != element2_p->tag)
        return -1;

    if (element1_p->context != element2_p->context)
        return -1;

    if (element1_p->id_in_context != element2_p->id_in_context)
        return -1;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(element1_p->id),
                                  NULL);

    if (NULL == param_descr_p)
    {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return -1;
    }

    if ((element1_p->value_p == NULL) && (element2_p->value_p == NULL))
        return 0;

    if ((element1_p->value_p == NULL) || (element2_p->value_p == NULL))
        return -1;

    return (*param_descr_p->array[PARAMID_TO_INDEX(element1_p->id)].compare_func)
        (element1_p->value_p,
         element2_p->value_p,
         &param_descr_p->array2[PARAMID_TO_INDEX(element1_p->id)]);
}


/**
 * mpl_param_element_get_default_bl
 */
mpl_param_element_t*
    mpl_param_element_get_default_bl(mpl_param_element_id_t param_id,
                                     mpl_blacklist_t blacklist)
{
    mpl_param_element_t* element_p;
    int res;
    mpl_param_descr_set_t *param_descr_p;

    param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(param_id), NULL);

    if (NULL == param_descr_p)
    {
        // [DOC] The specified MPL parameter has an unrecognized parameter set
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Parameter set is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    if (!PARAMID_OK(param_id, param_descr_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Unknown parameter ID: %d\n", param_id));
        set_errno(E_MPL_INVALID_PARAMETER);
        return (NULL);
    }

    /* Is the parameter in the blacklist? */
    if (mpl_param_list_find(param_id, blacklist))
        return NULL;

    /* Do we have a default value? */
    if (param_descr_p->array[PARAMID_TO_INDEX(param_id)].default_value_p == NULL)
    {
        /* There is no default value for parameter */
        return NULL;
    }

    element_p = mpl_param_element_create_empty(param_id);
    if (NULL == element_p)
    {
        return NULL;
    }

    res = (*param_descr_p->array[PARAMID_TO_INDEX(param_id)].clone_func)
          (&element_p->value_p,
           param_descr_p->array[PARAMID_TO_INDEX(param_id)].default_value_p,
           &param_descr_p->array2[PARAMID_TO_INDEX(param_id)]);
    if (res < 0)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Param value clone failed for %s\n",
                             mpl_param_id_get_string(param_id)));
        mpl_param_element_destroy(element_p);
        return NULL;
    }

    return element_p;
}

/**
 * mpl_param_element_destroy
 */
void
    mpl_param_element_destroy(mpl_param_element_t* element_p)
{
    if (NULL == element_p)
        return;

    if (NULL != element_p->value_p)
    {
        mpl_param_descr_set_t *param_descr_p;

        param_descr_p = paramset_find(MPL_PARAMID_TO_PARAMSET(element_p->id),
                                      NULL);

        if (NULL != param_descr_p)
        {
            if (PARAMID_OK(element_p->id, param_descr_p))
            {
                (*param_descr_p->array[PARAMID_TO_INDEX(element_p->id)].free_func)
                    (element_p->value_p);
            }
            else
            {
                MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                                    ("param id not ok, element not freed\n"));
                set_errno(E_MPL_INVALID_PARAMETER);
            }
        }
        else
        {
            MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                                ("param descr set not found, element not freed\n"));
            set_errno(E_MPL_INVALID_PARAMETER);
        }
    }

    free(element_p);
}

int mpl_param_list_pack(mpl_list_t *param_list_p,
                        char *buf_p,
                        int buflen)
{
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    options.no_prefix = false;
    return mpl_param_list_pack_extended(param_list_p,
                                        buf_p,
                                        buflen,
                                        &options);
}

int mpl_param_list_pack_no_prefix(mpl_list_t *param_list_p,
                                  char *buf_p,
                                  int buflen)
{
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    options.no_prefix = true;
    return mpl_param_list_pack_extended(param_list_p,
                                        buf_p,
                                        buflen,
                                        &options);
}

/**
 * mpl_param_list_pack_extended
 */
int mpl_param_list_pack_extended(mpl_list_t *param_list_p,
                                 char *buf_p,
                                 int buflen,
                                 const mpl_pack_options_t *options_p)
{
    int total_len = 0;
    int tmplen;
    mpl_list_t *elem_p;
    mpl_param_element_t* param_elem_p;
    int res;
    mpl_pack_options_t *new_options_p = calloc(1, sizeof(mpl_pack_options_t));

    if (new_options_p == NULL) {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        res = -1;
        goto error_return;
    }

    *new_options_p = *options_p;
    if (!new_options_p->force_field_pack_mode)
        new_options_p->field_pack_mode = field_pack_mode_context;

    /* Make sure we always terminate the buffer */
    if ((NULL != buf_p) && (buflen > 0))
        buf_p[0] = '\0';

    /* Loop over all parameters in parameter list and pack them */
    /*lint -esym(613, buf_p) */
    MPL_LIST_FOR_EACH(param_list_p, elem_p)
    {
        /* Add a delimiter between all parameters */
        if (total_len > 0)
        {
            tmplen = snprintf((total_len < buflen)?buf_p+total_len:NULL,
                              (total_len < buflen)?buflen-total_len:0,
                              "%c",
                              new_options_p->message_delimiter);
            if (tmplen < 0)
            {
                res = tmplen;
                goto error_return;
            }

            total_len += tmplen;
        }

        param_elem_p = MPL_LIST_CONTAINER(elem_p, mpl_param_element_t, list_entry);
        tmplen = mpl_param_pack_internal(param_elem_p,
                                         (total_len < buflen)?buf_p+total_len:NULL,
                                         (total_len < buflen)?buflen-total_len:0,
                                         new_options_p);
        if (tmplen < 0)
        {
            res = tmplen;
            goto error_return;
        }

        total_len += tmplen;
    }
    free(new_options_p);

    return total_len;

error_return:
    if (new_options_p != NULL)
        free(new_options_p);
    return res;
}

/* for backward compatibility */
int mpl_param_list_pack_internal(mpl_list_t *param_list_p,
                                 char *buf_p,
                                 int buflen,
                                 char message_delimiter,
                                 bool no_prefix)
{
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    options.no_prefix = no_prefix;
    options.message_delimiter = message_delimiter;
    return mpl_param_list_pack_extended(param_list_p,
                                        buf_p,
                                        buflen,
                                        &options);
}

mpl_list_t *mpl_param_list_unpack(char *buf_p)
{
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    return mpl_param_list_unpack_internal(buf_p,
                                          &options,
                                          MPL_PARAM_ID_UNDEFINED,
                                          NULL);
}

mpl_list_t *mpl_param_list_unpack_error(char *buf_p, bool *has_error_p)
{
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    return mpl_param_list_unpack_internal(buf_p,
                                          &options,
                                          MPL_PARAM_ID_UNDEFINED,
                                          has_error_p);
}

mpl_list_t *mpl_param_list_unpack_param_set(char *buf_p, int param_set_id)
{
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    options.param_set_id = param_set_id;
    return mpl_param_list_unpack_internal(buf_p,
                                          &options,
                                          MPL_PARAM_ID_UNDEFINED,
                                          NULL);
}

mpl_list_t *mpl_param_list_unpack_param_set_error(char *buf_p,
                                                  int param_set_id,
                                                  bool *has_error_p)
{
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    options.param_set_id = param_set_id;
    return mpl_param_list_unpack_internal(buf_p,
                                          &options,
                                          MPL_PARAM_ID_UNDEFINED,
                                          has_error_p);
}

/**
 * mpl_param_list_unpack_internal - unpack packed parameter list
 *
 **/
mpl_list_t *mpl_param_list_unpack_internal(char *buf_p,
                                           const mpl_pack_options_t *options_p,
                                           mpl_param_element_id_t unpack_context,
                                           bool *has_error_p)
{
    int i;
    mpl_param_element_t *param_elem_p;
    mpl_list_t *param_list_p=NULL;
    int numargs;
    mpl_arg_t *args_p;
    char *tmp_buf_p;

    if (NULL == buf_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("buf_p is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        if (has_error_p != NULL)
            *has_error_p = true;
        return NULL;
    }

    args_p = malloc(sizeof(mpl_arg_t)*MPL_MAX_ARGS);
    if (NULL == args_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        if (has_error_p != NULL)
            *has_error_p = true;
        return NULL;
    }
    memset(args_p,0,sizeof(mpl_arg_t)*MPL_MAX_ARGS);

    tmp_buf_p = malloc(strlen(buf_p) + 1);
    if (NULL == tmp_buf_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        if (has_error_p != NULL)
            *has_error_p = true;
        free(args_p);
        return NULL;
    }
    strcpy(tmp_buf_p, buf_p);

    /* Split buffer into array of key, value string pointers */
    numargs = mpl_get_args(args_p,
                           MPL_MAX_ARGS,
                           tmp_buf_p,
                           '=',
                           options_p->message_delimiter,
                           '\\');

    /* Loop over arguments and make list of parameter elements */
    for (i = 0; i < numargs; i++)
    {
        assert(NULL != args_p[i].key_p);

        if (mpl_param_unpack_internal(args_p[i].key_p,
                                      args_p[i].value_p,
                                      &param_elem_p,
                                      options_p,
                                      unpack_context) < 0)
        {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Param unpack failed for param: %s=%s\n",
                                 args_p[i].key_p,
                                 args_p[i].value_p));
            mpl_param_list_destroy(&param_list_p);
            if (has_error_p != NULL)
                *has_error_p = true;
            free(args_p);
            free(tmp_buf_p);
            return NULL;
        }

        mpl_list_add(&param_list_p, &param_elem_p->list_entry);
    }

    if (has_error_p != NULL)
        *has_error_p = false;
    free(args_p);
    free(tmp_buf_p);
    return param_list_p;
}


/**
 * mpl_param_list_clone
 */
mpl_list_t *mpl_param_list_clone(mpl_list_t *param_list_p)
{
    mpl_list_t *elem_p;
    mpl_param_element_t *cloned_param_p;
    mpl_list_t *cloned_list_p=NULL;


    MPL_LIST_FOR_EACH(param_list_p, elem_p)
    {
        cloned_param_p =
            mpl_param_element_clone(MPL_LIST_CONTAINER(elem_p,
                                                       mpl_param_element_t,
                                                       list_entry));
        if(cloned_param_p == NULL)
        {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,("clone parameter\n"));
            goto error_return;
        }
        mpl_list_append(&cloned_list_p, &cloned_param_p->list_entry);
    }
    return cloned_list_p;

error_return:
    mpl_param_list_destroy(&cloned_list_p);
    return NULL;
}

/**
 * mpl_param_list_destroy
 *
 */
void mpl_param_list_destroy( mpl_list_t **param_list_pp )
{
    mpl_list_t *elem_p, *tmp_p;

    if (NULL == param_list_pp)
        return;

    MPL_LIST_FOR_EACH_SAFE(*param_list_pp, elem_p, tmp_p)
    {
        (void)mpl_list_remove(param_list_pp, elem_p);
        mpl_param_element_destroy(MPL_LIST_CONTAINER(elem_p,
                                                     mpl_param_element_t,
                                                     list_entry));
    }
    *param_list_pp = NULL;
}

/**
 * mpl_param_list_find
 *
 */
mpl_param_element_t*
    mpl_param_list_find( mpl_param_element_id_t param_id,
                         mpl_list_t *param_list_p )
{
    mpl_list_t *elem_p;
    mpl_param_element_t *res;

    MPL_LIST_FOR_EACH(param_list_p, elem_p)
    {
        res = MPL_LIST_CONTAINER(elem_p, mpl_param_element_t, list_entry);
        if (mpl_param_id_is_same_or_child(param_id, res->id))
            return res;
    }

    return NULL;
}

/**
 * mpl_param_list_find_tag
 *
 */
mpl_param_element_t*
    mpl_param_list_find_tag( mpl_param_element_id_t param_id,
                             int tag,
                             mpl_list_t *param_list_p )
{
    mpl_list_t *elem_p;
    mpl_param_element_t *res;

    MPL_LIST_FOR_EACH(param_list_p, elem_p)
    {
        res = MPL_LIST_CONTAINER(elem_p, mpl_param_element_t, list_entry);
        if (((param_id == MPL_PARAM_ID_UNDEFINED) || mpl_param_id_is_same_or_child(param_id, res->id)) &&
            (res->tag == tag))
            return res;
    }

    return NULL;
}

/**
 * mpl_param_list_find_all
 *
 */
mpl_list_t*
    mpl_param_list_find_all( mpl_param_element_id_t param_id,
                             mpl_list_t *param_list_p )
{
    mpl_param_element_t* param_elem_p;
    mpl_param_element_t* found_param_elem_p;
    mpl_list_t* result_list_p = NULL;

    param_elem_p = mpl_param_list_find(param_id, param_list_p);
    if (NULL != param_elem_p)
    {
        found_param_elem_p = mpl_param_element_clone(param_elem_p);
        if (NULL == found_param_elem_p)
            goto error_return;
        mpl_list_add(&result_list_p, &found_param_elem_p->list_entry);
    }

    while (NULL != param_elem_p)
    {
        param_elem_p = mpl_param_list_find_next(param_id, param_elem_p);
        if (NULL != param_elem_p)
        {
            found_param_elem_p = mpl_param_element_clone(param_elem_p);
            if (NULL == found_param_elem_p)
                goto error_return;
            mpl_list_add(&result_list_p, &found_param_elem_p->list_entry);
        }
    }
    return result_list_p;

error_return:
    mpl_param_list_destroy(&result_list_p);
    return NULL;
}

/**
 * mpl_param_list_find_all_tag
 *
 */
mpl_list_t*
    mpl_param_list_find_all_tag( mpl_param_element_id_t param_id,
                                 int tag,
                                 mpl_list_t *param_list_p )
{
    mpl_param_element_t* param_elem_p;
    mpl_param_element_t* found_param_elem_p;
    mpl_list_t* result_list_p = NULL;

    param_elem_p = mpl_param_list_find_tag(param_id, tag, param_list_p);
    if (NULL != param_elem_p)
    {
        found_param_elem_p = mpl_param_element_clone(param_elem_p);
        if (NULL == found_param_elem_p)
            goto error_return;
        mpl_list_add(&result_list_p, &found_param_elem_p->list_entry);
    }

    while (NULL != param_elem_p)
    {
        param_elem_p = mpl_param_list_find_next_tag(param_id, tag, param_elem_p);
        if (NULL != param_elem_p)
        {
            found_param_elem_p = mpl_param_element_clone(param_elem_p);
            if (NULL == found_param_elem_p)
                goto error_return;
            mpl_list_add(&result_list_p, &found_param_elem_p->list_entry);
        }
    }
    return result_list_p;

error_return:
    mpl_param_list_destroy(&result_list_p);
    return NULL;
}

int mpl_param_list_param_count( mpl_param_element_id_t param_id,
                                mpl_list_t *param_list_p )
{
    mpl_param_element_t* param_elem_p;
    int count = 0;

    param_elem_p = mpl_param_list_find(param_id, param_list_p);
    if (NULL != param_elem_p)
        count++;

    while (NULL != param_elem_p)
    {
        param_elem_p = mpl_param_list_find_next(param_id, param_elem_p);
        if (NULL != param_elem_p)
            count++;
    }
    return count;
}

int mpl_param_list_param_count_tag( mpl_param_element_id_t param_id,
                                    int tag,
                                    mpl_list_t *param_list_p )
{
    mpl_param_element_t* param_elem_p;
    int count = 0;

    param_elem_p = mpl_param_list_find_tag(param_id, tag, param_list_p);
    if (NULL != param_elem_p)
        count++;

    while (NULL != param_elem_p)
    {
        param_elem_p = mpl_param_list_find_next_tag(param_id, tag, param_elem_p);
        if (NULL != param_elem_p)
            count++;
    }
    return count;
}

mpl_param_element_t*
    mpl_param_list_find_field( mpl_param_element_id_t context,
                               int id_in_context,
                               mpl_list_t *param_list_p )
{
    mpl_list_t *elem_p;
    mpl_param_element_t *res;
    mpl_param_element_id_t bag_field_context =
        mpl_param_get_bag_field_context(context,id_in_context);

    MPL_LIST_FOR_EACH(param_list_p, elem_p)
    {
        res = MPL_LIST_CONTAINER(elem_p, mpl_param_element_t, list_entry);
        if ((res->context == bag_field_context) &&
            (res->id_in_context == id_in_context))
            return res;
    }

    return NULL;
}

mpl_param_element_t*
    mpl_param_list_find_field_tag( mpl_param_element_id_t context,
                                   int id_in_context,
                                   int tag,
                                   mpl_list_t *param_list_p )
{
    mpl_list_t *elem_p;
    mpl_param_element_t *res;

    MPL_LIST_FOR_EACH(param_list_p, elem_p)
    {
        res = MPL_LIST_CONTAINER(elem_p, mpl_param_element_t, list_entry);
        if ((res->context == mpl_param_get_bag_field_context(context,id_in_context)) &&
            (res->id_in_context == id_in_context) &&
            (res->tag == tag))
            return res;
    }

    return NULL;
}

mpl_list_t*
    mpl_param_list_find_field_all( mpl_param_element_id_t context,
                                   int id_in_context,
                                   mpl_list_t *param_list_p )
{
    mpl_param_element_t* param_elem_p;
    mpl_param_element_t* found_param_elem_p;
    mpl_list_t* result_list_p = NULL;

    param_elem_p = mpl_param_list_find_field(context,
                                             id_in_context,
                                             param_list_p);
    if (NULL != param_elem_p)
    {
        found_param_elem_p = mpl_param_element_clone(param_elem_p);
        if (NULL == found_param_elem_p)
            goto error_return;
        mpl_list_add(&result_list_p, &found_param_elem_p->list_entry);
    }

    while (NULL != param_elem_p)
    {
        param_elem_p = mpl_param_list_find_field_next(context,
                                                      id_in_context,
                                                      param_elem_p);
        if (NULL != param_elem_p)
        {
            found_param_elem_p = mpl_param_element_clone(param_elem_p);
            if (NULL == found_param_elem_p)
                goto error_return;
            mpl_list_add(&result_list_p, &found_param_elem_p->list_entry);
        }
    }
    return result_list_p;

error_return:
    mpl_param_list_destroy(&result_list_p);
    return NULL;
}

mpl_list_t*
    mpl_param_list_find_field_all_tag( mpl_param_element_id_t context,
                                       int id_in_context,
                                       int tag,
                                       mpl_list_t *param_list_p )
{
    mpl_param_element_t* param_elem_p;
    mpl_param_element_t* found_param_elem_p;
    mpl_list_t* result_list_p = NULL;

    param_elem_p = mpl_param_list_find_field_tag(context,
                                                 id_in_context,
                                                 tag,
                                                 param_list_p);
    if (NULL != param_elem_p)
    {
        found_param_elem_p = mpl_param_element_clone(param_elem_p);
        if (NULL == found_param_elem_p)
            goto error_return;
        mpl_list_add(&result_list_p, &found_param_elem_p->list_entry);
    }

    while (NULL != param_elem_p)
    {
        param_elem_p = mpl_param_list_find_field_next_tag(context,
                                                          id_in_context,
                                                          tag,
                                                          param_elem_p);
        if (NULL != param_elem_p)
        {
            found_param_elem_p = mpl_param_element_clone(param_elem_p);
            if (NULL == found_param_elem_p)
                goto error_return;
            mpl_list_add(&result_list_p, &found_param_elem_p->list_entry);
        }
    }
    return result_list_p;

error_return:
    mpl_param_list_destroy(&result_list_p);
    return NULL;
}

int mpl_param_list_field_count( mpl_param_element_id_t context,
                                int id_in_context,
                                mpl_list_t *param_list_p )
{
    mpl_param_element_t* param_elem_p;
    int count = 0;

    param_elem_p = mpl_param_list_find_field(context,
                                             id_in_context,
                                             param_list_p);
    if (NULL != param_elem_p)
        count++;

    while (NULL != param_elem_p)
    {
        param_elem_p = mpl_param_list_find_field_next(context,
                                                      id_in_context,
                                                      param_elem_p);
        if (NULL != param_elem_p)
            count++;
    }
    return count;
}

int mpl_param_list_field_count_tag( mpl_param_element_id_t context,
                                    int id_in_context,
                                    int tag,
                                    mpl_list_t *param_list_p )
{
    mpl_param_element_t* param_elem_p;
    int count = 0;

    param_elem_p = mpl_param_list_find_field_tag(context,
                                                 id_in_context,
                                                 tag,
                                                 param_list_p);
    if (NULL != param_elem_p)
        count++;

    while (NULL != param_elem_p)
    {
        param_elem_p = mpl_param_list_find_field_next_tag(context,
                                                          id_in_context,
                                                          tag,
                                                          param_elem_p);
        if (NULL != param_elem_p)
            count++;
    }
    return count;
}

/**
 * mpl_param_list_string_tuple_key_find
 *
 */
mpl_param_element_t*
    mpl_param_list_string_tuple_key_find( mpl_param_element_id_t param_id,
                                          char *key_p,
                                          mpl_list_t *param_list_p )
{
    mpl_param_element_t *param_elem_p;
    mpl_string_tuple_t *st_p;

    if (mpl_param_id_get_type(param_id) != mpl_type_string_tuple)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Invalid type '%s'\n",
                             mpl_param_id_get_type_string(param_id)));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    if ((NULL == key_p) || (0 == strlen(key_p)))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Invalid key_p\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    param_elem_p = mpl_param_list_find(param_id, param_list_p);
    while (NULL != param_elem_p)
    {
        st_p = MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_string_tuple_t*,
                                                    param_elem_p);
        if ((NULL != st_p) &&
            (NULL != st_p->key_p) &&
            (strlen(st_p->key_p) == strlen(key_p)) &&
            !strncmp(st_p->key_p, key_p, strlen(key_p)))
            return param_elem_p;

        param_elem_p = mpl_param_list_find_next(param_id, param_elem_p);
    }
    return NULL;
}


/**
 * mpl_param_list_strint_tuple_key_find
 *
 */
mpl_param_element_t*
    mpl_param_list_strint_tuple_key_find( mpl_param_element_id_t param_id,
                                          char *key_p,
                                          mpl_list_t *param_list_p )
{
    mpl_param_element_t *param_elem_p;
    mpl_strint_tuple_t *st_p;

    if (mpl_param_id_get_type(param_id) != mpl_type_strint_tuple)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Invalid type '%s'\n",
                             mpl_param_id_get_type_string(param_id)));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    if ((NULL == key_p) || (0 == strlen(key_p)))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Invalid key_p\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    param_elem_p = mpl_param_list_find(param_id, param_list_p);
    while (NULL != param_elem_p)
    {
        st_p = MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_strint_tuple_t*,
                                                    param_elem_p);
        if ((NULL != st_p) &&
            (NULL != st_p->key_p) &&
            (strlen(st_p->key_p) == strlen(key_p)) &&
            !strncmp(st_p->key_p, key_p, strlen(key_p)))
            return param_elem_p;

        param_elem_p = mpl_param_list_find_next(param_id, param_elem_p);
    }
    return NULL;
}

/**
 * mpl_param_list_struint8_tuple_key_find
 *
 */
mpl_param_element_t*
    mpl_param_list_struint8_tuple_key_find( mpl_param_element_id_t param_id,
                                            char *key_p,
                                            mpl_list_t *param_list_p )
{
    mpl_param_element_t *param_elem_p;
    mpl_struint8_tuple_t *st_p;

    if (mpl_param_id_get_type(param_id) != mpl_type_struint8_tuple)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Invalid type '%s'\n",
                             mpl_param_id_get_type_string(param_id)));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    if ((NULL == key_p) || (0 == strlen(key_p)))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Invalid key_p\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    param_elem_p = mpl_param_list_find(param_id, param_list_p);
    while (NULL != param_elem_p)
    {
        st_p = MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_struint8_tuple_t*,
                                                    param_elem_p);
        if ((NULL != st_p) &&
            (NULL != st_p->key_p) &&
            (strlen(st_p->key_p) == strlen(key_p)) &&
            !strncmp(st_p->key_p, key_p, strlen(key_p)))
            return param_elem_p;

        param_elem_p = mpl_param_list_find_next(param_id, param_elem_p);
    }
    return NULL;
}

/**
 * mpl_param_list_int_tuple_key_find
 *
 */
mpl_param_element_t*
    mpl_param_list_int_tuple_key_find( mpl_param_element_id_t param_id,
                                       int key,
                                       mpl_list_t *param_list_p )
{
    mpl_param_element_t *param_elem_p;
    mpl_int_tuple_t *it_p;

    if (mpl_param_id_get_type(param_id) != mpl_type_int_tuple)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Invalid type '%s'\n",
                             mpl_param_id_get_type_string(param_id)));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    param_elem_p = mpl_param_list_find(param_id, param_list_p);
    while (NULL != param_elem_p)
    {
        it_p = MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_int_tuple_t*,
                                                    param_elem_p);
        if ((NULL != it_p) && (it_p->key == key))
            return param_elem_p;

        param_elem_p = mpl_param_list_find_next(param_id, param_elem_p);
    }
    return NULL;
}


mpl_param_element_t*
    mpl_param_list_string_tuple_find( mpl_param_element_id_t param_id,
                                      char *key_p,
                                      char *value_p,
                                      mpl_list_t *param_list_p )
{
    mpl_param_element_t* param_elem_p;
    mpl_string_tuple_t *st_p;

    param_elem_p = mpl_param_list_string_tuple_key_find(param_id,
                                                        key_p,
                                                        param_list_p);
    while (NULL != param_elem_p)
    {
        st_p = MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_string_tuple_t*,
                                                    param_elem_p);
        if ((NULL != st_p) &&
            (NULL != st_p->value_p) &&
            (strlen(st_p->value_p) == strlen(value_p)) &&
            !strncmp(st_p->value_p, value_p, strlen(value_p)))
            return param_elem_p;

        param_elem_p =
            mpl_param_list_string_tuple_key_find(param_id,
                                                 key_p,
                                                 param_elem_p->list_entry.next_p);
    }
    return NULL;
}

mpl_param_element_t*
    mpl_param_list_strint_tuple_find( mpl_param_element_id_t param_id,
                                      char *key_p,
                                      int value,
                                      mpl_list_t *param_list_p )
{
    mpl_param_element_t* param_elem_p;
    mpl_strint_tuple_t *it_p;

    param_elem_p = mpl_param_list_strint_tuple_key_find(param_id,
                                                        key_p,
                                                        param_list_p);
    while (NULL != param_elem_p)
    {
        it_p = MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_strint_tuple_t*,
                                                    param_elem_p);
        if ((NULL != it_p) && (it_p->value == value))
            return param_elem_p;

        param_elem_p =
            mpl_param_list_strint_tuple_key_find(param_id,
                                                 key_p,
                                                 param_elem_p->list_entry.next_p);
    }
    return NULL;
}

mpl_param_element_t*
    mpl_param_list_struint8_tuple_find( mpl_param_element_id_t param_id,
                                        char *key_p,
                                        uint8_t value,
                                        mpl_list_t *param_list_p )
{
    mpl_param_element_t* param_elem_p;
    mpl_struint8_tuple_t *it_p;

    param_elem_p = mpl_param_list_struint8_tuple_key_find(param_id,
                                                          key_p,
                                                          param_list_p);
    while (NULL != param_elem_p)
    {
        it_p = MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_struint8_tuple_t*,
                                                    param_elem_p);
        if ((NULL != it_p) && (it_p->value == value))
            return param_elem_p;

        param_elem_p =
            mpl_param_list_strint_tuple_key_find(param_id,
                                                 key_p,
                                                 param_elem_p->list_entry.next_p);
    }
    return NULL;
}

mpl_param_element_t*
    mpl_param_list_int_tuple_find( mpl_param_element_id_t param_id,
                                   int key,
                                   int value,
                                   mpl_list_t *param_list_p )
{
    mpl_param_element_t* param_elem_p;
    mpl_int_tuple_t *it_p;

    param_elem_p = mpl_param_list_int_tuple_key_find(param_id,
                                                     key,
                                                     param_list_p);
    while (NULL != param_elem_p)
    {
        it_p = MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(mpl_int_tuple_t*,
                                                    param_elem_p);
        if ((NULL != it_p) && (it_p->value == value))
            return param_elem_p;

        param_elem_p =
            mpl_param_list_int_tuple_key_find(param_id,
                                              key,
                                              param_elem_p->list_entry.next_p);
    }
    return NULL;
}



/**
 * mpl_param_list_tuple_key_find_wildcard
 *
 */
mpl_param_element_t*
    mpl_param_list_tuple_key_find_wildcard( mpl_param_element_id_t param_id,
                                            char *key_p,
                                            char *wildcard_p,
                                            mpl_list_t *param_list_p )
{
    mpl_param_element_t *param_elem_p;

    if (mpl_param_id_get_type(param_id) != mpl_type_string_tuple)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Invalid type '%s'\n",
                             mpl_param_id_get_type_string(param_id)));
        set_errno(E_MPL_INVALID_PARAMETER);
        return NULL;
    }

    param_elem_p = mpl_param_list_tuple_key_find(param_id, key_p, param_list_p);
    if (NULL != param_elem_p)
        return param_elem_p;

    return mpl_param_list_tuple_key_find(param_id, wildcard_p, param_list_p);
}


/**
 * mpl_pack_param_value_string()
 **/
int mpl_pack_param_value_string(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p)
{
    assert(NULL != param_value_p);

    return snprintf_escape(options_p->message_delimiter, '\\',
                           buf, buflen, "=%s", (char*)param_value_p);
}

/**
 * mpl_unpack_param_value_string()
 **/
int
    mpl_unpack_param_value_string(const char* value_str, void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context)
{
    char* p;
    size_t size;
    char *temp_str;
    int temp_string_allocated = 0;
    const int *max_p = descr_p->max_p;
    const int *min_p = descr_p->min_p;

    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);


    if (strchr(value_str, '\\')) {
        temp_str = remove_escape(value_str, options_p->message_delimiter, '\\');
        if (temp_str == NULL) {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                                ("Failed allocating memory\n"));
            set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
            return (-1);
        }
        temp_string_allocated = 1;
    }
    else {
        temp_str = (char*) value_str;
    }

    if ((max_p != NULL) &&
        ((int)strlen(temp_str) > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack string failed on max length check: %zu > %d\n",
                             strlen(temp_str), *max_p));/*lint !e557 %zu is C99 */
        set_errno(E_MPL_FAILED_OPERATION);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    if ((min_p != NULL) &&
        ((int)strlen(temp_str) < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack string failed on min length check: %zu < %d\n",
                             strlen(temp_str), *min_p));/*lint !e557 %zu is C99 */
        set_errno(E_MPL_FAILED_OPERATION);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    size = strlen(temp_str)+1;
    p = malloc(size);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    strncpy(p, temp_str, size);
    if (temp_string_allocated)
        free(temp_str);
    *value_pp = p;

    return (0);
}

/**
 * mpl_clone_param_value_string()
 **/
int mpl_clone_param_value_string(void **new_value_pp,
                                 const void* old_value_p,
                                 const mpl_param_descr2_t *descr_p)
{
    size_t size = strlen((char*)old_value_p) + 1;
    char* p = malloc(size);
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    strncpy(p, (char*)old_value_p, size);
    *new_value_pp = p;

    return (0);
}

/**
 * mpl_copy_param_value_string()
 **/
int mpl_copy_param_value_string(void *to_value_p,
                                const void* from_value_p,
                                int size,
                                const mpl_param_descr2_t *descr_p)
{
    int real_size = strlen((char*)from_value_p) + 1;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    if (real_size <= size)
        strncpy(to_value_p, (char*)from_value_p, real_size);

    return real_size;
}

/**
 * mpl_compare_param_value_string()
 **/
int mpl_compare_param_value_string(const void *value1_p,
                                   const void* value2_p,
                                   const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return strcmp(value1_p, value2_p);
}

/**
 * mpl_sizeof_param_value_string()
 **/
size_t mpl_sizeof_param_value_string(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return 0;
}


/**
 * mpl_pack_param_value_wstring()
 **/
int mpl_pack_param_value_wstring(const void* param_value_p,
                                 char *buf,
                                 size_t buflen,
                                 const mpl_param_descr2_t *descr_p,
                                 const mpl_pack_options_t *options_p)
{
    mpl_uint32_array_t a;
    int res;

    assert(NULL != param_value_p);

    a.len = wcslen((wchar_t*)param_value_p) + 1;
    if (sizeof(wchar_t) == sizeof(uint32_t))
    {
        a.arr_p = (uint32_t*)param_value_p;
    }
    else if (sizeof(wchar_t) == sizeof(uint16_t))
    {
        unsigned int i;

        a.arr_p = malloc(a.len * sizeof(uint32_t));
        if (NULL == a.arr_p)
        {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                                ("Failed allocating memory\n"));
            set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
            return (-1);
        }

        for (i = 0; i < a.len; i++)
        {
            a.arr_p[i] = ((uint16_t*)param_value_p)[i];
        }
    }
    else
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unsupported wchar width: %zu\n",
                             sizeof(wchar_t)));/*lint !e557 %zu is C99 */
        set_errno(E_MPL_FAILED_OPERATION);
        return (-1);
    }

    res = mpl_pack_param_value_uint32_array(&a,
                                            buf,
                                            buflen,
                                            descr_p,
                                            options_p);

    if (sizeof(wchar_t) == sizeof(uint16_t))
    {
        free(a.arr_p);
    }

    return res;

}

/**
 * mpl_unpack_param_value_wstring()
 **/
int
    mpl_unpack_param_value_wstring(const char* value_str, void **value_pp,
                                   const mpl_param_descr2_t *descr_p,
                                   const mpl_pack_options_t *options_p,
                                   mpl_param_element_id_t unpack_context)
{
    mpl_uint32_array_t *a_p;
    int res;
    const int *max_p = descr_p->max_p;
    const int *min_p = descr_p->min_p;

    if ((res = mpl_unpack_param_value_uint32_array(value_str,
                                                   (void **)&a_p,
                                                   descr_p,
                                                   options_p,
                                                   unpack_context)) < 0)
    {
        return res;
    }

    if ((max_p != NULL) &&
        ((int)(a_p->len - 1) > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack wstring failed on max length check: %d > %d\n",
                             a_p->len - 1, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        mpl_free_param_value_uint32_array(a_p);
        return (-1);
    }

    if ((min_p != NULL) &&
        ((int)(a_p->len - 1) < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack wstring failed on min length check: %d < %d\n",
                             a_p->len - 1, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        mpl_free_param_value_uint32_array(a_p);
        return (-1);
    }

    if (sizeof(wchar_t) == sizeof(uint16_t))
    {
        unsigned int i;
        uint16_t *arr_p;

        arr_p = malloc(a_p->len * sizeof(wchar_t));
        if (NULL == arr_p)
        {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                                ("Failed allocating memory\n"));
            set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
            free(a_p);
            return (-1);
        }

        for (i = 0; i < a_p->len; i++)
        {
            arr_p[i] = a_p->arr_p[i];
        }
        *value_pp = arr_p;
        free(a_p->arr_p);
    }
    else if (sizeof(wchar_t) == sizeof(uint32_t))
    {
        *value_pp = a_p->arr_p;
    }
    else
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unsupported wchar width: %zu\n",
                             sizeof(wchar_t)));/*lint !e557 %zu is C99 */
        set_errno(E_MPL_FAILED_OPERATION);
        mpl_free_param_value_uint32_array(a_p);
        return (-1);
    }

    free(a_p);
    return 0;
}

/**
 * mpl_clone_param_value_wstring()
 **/
int mpl_clone_param_value_wstring(void **new_value_pp,
                                  const void* old_value_p,
                                  const mpl_param_descr2_t *descr_p)
{
    size_t size = wcslen((wchar_t*)old_value_p) + 1;
    wchar_t* p = malloc(size * sizeof(L'\0'));
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    (void)wcsncpy(p, (wchar_t*)old_value_p, size);
    *new_value_pp = p;

    return (0);
}

/**
 * mpl_copy_param_value_wstring()
 **/
int mpl_copy_param_value_wstring(void *to_value_p,
                                 const void* from_value_p,
                                 int size,
                                 const mpl_param_descr2_t *descr_p)
{
    int real_size = (wcslen((wchar_t*)from_value_p) + 1) * sizeof(L'\0');
    MPL_IDENTIFIER_NOT_USED(descr_p);

    if (real_size <= size)
        (void)wcsncpy(to_value_p, (wchar_t*)from_value_p, real_size / (int)sizeof(L'\0'));

    return real_size;
}

/**
 * mpl_compare_param_value_wstring()
 **/
int mpl_compare_param_value_wstring(const void *value1_p,
                                    const void* value2_p,
                                    const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return wcscmp(value1_p, value2_p);
}

/**
 * mpl_sizeof_param_value_wstring()
 **/
size_t mpl_sizeof_param_value_wstring(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return 0;
}



/**
 * mpl_pack_param_value_int()
 **/
int mpl_pack_param_value_int(const void* param_value_p,
                             char *buf,
                             size_t buflen,
                             const mpl_param_descr2_t *descr_p,
                             const mpl_pack_options_t *options_p)
{
    assert(NULL != param_value_p);
    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p)
        return snprintf(buf, buflen, "=%d", *(int*)param_value_p);
}

/**
 * mpl_unpack_param_value_int()
 **/
int
    mpl_unpack_param_value_int(const char* value_str,
                               void **value_pp,
                               const mpl_param_descr2_t *descr_p,
                               const mpl_pack_options_t *options_p,
                               mpl_param_element_id_t unpack_context)
{
    int* p;
    const int *max_p = descr_p->max_p;
    const int *min_p = descr_p->min_p;
    int range_id = 0;

    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    p = malloc(sizeof(int));
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    if (convert_int(value_str, p) < 0)
    {
        free(p);
        return (-1);
    }

    if ((max_p != NULL) &&
        (*p > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack int failed on max check: %d > %d\n",
                             *p, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if ((min_p != NULL) &&
        (*p < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack int failed on min check: %d < %d\n",
                             *p, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if (descr_p->integer_ranges_size > 0) {
        range_id = check_integer_ranges(*p,
                                        descr_p->integer_ranges,
                                        descr_p->integer_ranges_size);
        if (range_id < 0) {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Unpack int failed on range check: %d\n",
                                 *p));
            set_errno(E_MPL_FAILED_OPERATION);
            free(p);
            return (-1);
        }
    }
    assert(NULL != value_pp);
    *value_pp = p;

    return range_id;
}

/**
 * mpl_clone_param_value_int()
 **/
int
    mpl_clone_param_value_int(void **new_value_pp,
                              const void* old_value_p,
                              const mpl_param_descr2_t *descr_p)
{
    int* p = malloc(sizeof(int));
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    *p = *(int*)old_value_p;
    *new_value_pp = p;

    return (0);
}

/**
 * mpl_copy_param_value_int()
 **/
int
    mpl_copy_param_value_int(void *to_value_p,
                             const void* from_value_p,
                             int size,
                             const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (size >= (int)sizeof(int))
        *(int*)to_value_p = *(int*)from_value_p;

    return sizeof(int);
}

/**
 * mpl_compare_param_value_int()
 **/
int mpl_compare_param_value_int(const void *value1_p,
                                const void* value2_p,
                                const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return (*((int*) value1_p) != *((int*) value2_p));
}

/**
 * mpl_sizeof_param_value_int()
 **/
size_t mpl_sizeof_param_value_int(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return sizeof(int);
}



/**
 * mpl_pack_param_value_sint8()
 **/
int mpl_pack_param_value_sint8(const void* param_value_p,
                               char *buf,
                               size_t buflen,
                               const mpl_param_descr2_t *descr_p,
                               const mpl_pack_options_t *options_p)
{
    assert(NULL != param_value_p);
    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p)
        return snprintf(buf, buflen, "=%d", *(sint8_t*)param_value_p);
}

/**
 * mpl_unpack_param_value_sint8()
 **/
int
    mpl_unpack_param_value_sint8(const char* value_str, void **value_pp,
                                 const mpl_param_descr2_t *descr_p,
                                 const mpl_pack_options_t *options_p,
                                 mpl_param_element_id_t unpack_context)
{
    int temp;
    sint8_t* p;
    const sint8_t *max_p = descr_p->max_p;
    const sint8_t *min_p = descr_p->min_p;
    int range_id = 0;

    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    p = malloc(sizeof(sint8_t));
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    if (convert_int(value_str, &temp) < 0)
    {
        free(p);
        return (-1);
    }

    if ((temp < SCHAR_MIN) ||
        (temp > (int)SCHAR_MAX))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack sint8 failed on range check: %d\n", temp));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if ((max_p != NULL) &&
        ((sint8_t)temp > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack sint8 failed on max check: %d > %d\n",
                             temp, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if ((min_p != NULL) &&
        ((sint8_t)temp < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack sint8 failed on min check: %d < %d\n",
                             temp, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if (descr_p->integer_ranges_size > 0) {
        range_id = check_integer_ranges(temp,
                                        descr_p->integer_ranges,
                                        descr_p->integer_ranges_size);
        if (range_id < 0) {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Unpack sint8 failed on range check: %d\n",
                                 temp));
            set_errno(E_MPL_FAILED_OPERATION);
            free(p);
            return (-1);
        }
    }

    *p = (sint8_t)temp;

    assert(NULL != value_pp);
    *value_pp = p;

    return range_id;
}

/**
 * mpl_clone_param_value_sint8()
 **/
int
    mpl_clone_param_value_sint8(void **new_value_pp,
                                const void* old_value_p,
                                const mpl_param_descr2_t *descr_p)
{
    sint8_t* p = malloc(sizeof(sint8_t));
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    *p = *(sint8_t*)old_value_p;
    *new_value_pp = p;

    return (0);
}

/**
 * mpl_copy_param_value_sint8()
 **/
int
    mpl_copy_param_value_sint8(void *to_value_p,
                               const void* from_value_p,
                               int size,
                               const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (size >= (int)sizeof(sint8_t))
        *(sint8_t*)to_value_p = *(sint8_t*)from_value_p;

    return sizeof(sint8_t);
}

/**
 * mpl_compare_param_value_sint8()
 **/
int mpl_compare_param_value_sint8(const void *value1_p,
                                  const void* value2_p,
                                  const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return (*((sint8_t*) value1_p) != *((sint8_t*) value2_p));
}



/**
 * mpl_pack_param_value_sint16()
 **/
int mpl_pack_param_value_sint16(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p)
{
    assert(NULL != param_value_p);
    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p)
        return snprintf(buf, buflen, "=%d", *(sint16_t*)param_value_p);
}

/**
 * mpl_unpack_param_value_sint16()
 **/
int
    mpl_unpack_param_value_sint16(const char* value_str, void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context)
{
    int temp;
    sint16_t* p;
    const sint16_t *max_p = descr_p->max_p;
    const sint16_t *min_p = descr_p->min_p;
    int range_id = 0;

    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    p = malloc(sizeof(sint16_t));
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    if (convert_int(value_str, &temp) < 0)
    {
        free(p);
        return (-1);
    }

    if ((temp < SHRT_MIN) ||
        (temp > (int)SHRT_MAX))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack sint16 failed on range check: %d\n", temp));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if ((max_p != NULL) &&
        ((sint16_t)temp > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack sint16 failed on max check: %d > %d\n",
                             temp, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if ((min_p != NULL) &&
        ((sint16_t)temp < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack sint16 failed on min check: %d < %d\n",
                             temp, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if (descr_p->integer_ranges_size > 0) {
        range_id = check_integer_ranges(temp,
                                        descr_p->integer_ranges,
                                        descr_p->integer_ranges_size);
        if (range_id < 0) {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Unpack sint16 failed on range check: %d\n",
                                 temp));
            set_errno(E_MPL_FAILED_OPERATION);
            free(p);
            return (-1);
        }
    }

    *p = (sint16_t)temp;

    assert(NULL != value_pp);
    *value_pp = p;

    return range_id;
}

/**
 * mpl_clone_param_value_sint16()
 **/
int
    mpl_clone_param_value_sint16(void **new_value_pp,
                                 const void* old_value_p,
                                 const mpl_param_descr2_t *descr_p)
{
    sint16_t* p = malloc(sizeof(sint16_t));
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    *p = *(sint16_t*)old_value_p;
    *new_value_pp = p;

    return (0);
}

/**
 * mpl_copy_param_value_sint16()
 **/
int
    mpl_copy_param_value_sint16(void *to_value_p,
                                const void* from_value_p,
                                int size,
                                const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (size >= (int)sizeof(sint16_t))
        *(sint16_t*)to_value_p = *(sint16_t*)from_value_p;

    return sizeof(sint16_t);
}

/**
 * mpl_compare_param_value_sint16()
 **/
int mpl_compare_param_value_sint16(const void *value1_p,
                                   const void* value2_p,
                                   const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return (*((sint16_t*) value1_p) != *((sint16_t*) value2_p));
}



/**
 * mpl_pack_param_value_sint32()
 **/
int mpl_pack_param_value_sint32(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p)
{
    assert(NULL != param_value_p);
    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p)
        return snprintf(buf, buflen, "=%d", *(sint32_t*)param_value_p);
}

/**
 * mpl_unpack_param_value_sint32()
 **/
int
    mpl_unpack_param_value_sint32(const char* value_str, void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context)
{
    int temp;
    sint32_t* p;
    const sint32_t *max_p = descr_p->max_p;
    const sint32_t *min_p = descr_p->min_p;
    int range_id = 0;

    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    p = malloc(sizeof(sint32_t));
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    if (convert_int(value_str, &temp) < 0)
    {
        free(p);
        return (-1);
    }

    if ((max_p != NULL) &&
        ((sint32_t)temp > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack sint32 failed on max check: %d > %d\n",
                             temp, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if ((min_p != NULL) &&
        ((sint32_t)temp < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack sint32 failed on min check: %d < %d\n",
                             temp, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if (descr_p->integer_ranges_size > 0) {
        range_id = check_integer_ranges(temp,
                                        descr_p->integer_ranges,
                                        descr_p->integer_ranges_size);
        if (range_id < 0) {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Unpack sint32 failed on range check: %d\n",
                                 temp));
            set_errno(E_MPL_FAILED_OPERATION);
            free(p);
            return (-1);
        }
    }

    *p = (sint32_t)temp;

    assert(NULL != value_pp);
    *value_pp = p;

    return range_id;
}

/**
 * mpl_clone_param_value_sint32()
 **/
int
    mpl_clone_param_value_sint32(void **new_value_pp,
                                 const void* old_value_p,
                                 const mpl_param_descr2_t *descr_p)
{
    sint32_t* p = malloc(sizeof(sint32_t));
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    *p = *(sint32_t*)old_value_p;
    *new_value_pp = p;

    return (0);
}

/**
 * mpl_copy_param_value_sint32()
 **/
int
    mpl_copy_param_value_sint32(void *to_value_p,
                                const void* from_value_p,
                                int size,
                                const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (size >= (int)sizeof(sint32_t))
        *(sint32_t*)to_value_p = *(sint32_t*)from_value_p;

    return sizeof(sint32_t);
}

/**
 * mpl_compare_param_value_sint32()
 **/
int mpl_compare_param_value_sint32(const void *value1_p,
                                   const void* value2_p,
                                   const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return (*((sint32_t*) value1_p) != *((sint32_t*) value2_p));
}


/**
 * mpl_pack_param_value_sint64()
 **/
int mpl_pack_param_value_sint64(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p)
{
    assert(NULL != param_value_p);
    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p)
        return snprintf(buf, buflen, "=%" PRIi64, *(int64_t*)param_value_p);
}

/**
 * mpl_unpack_param_value_sint64()
 **/
int
    mpl_unpack_param_value_sint64(const char* value_str, void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context)
{
    int64_t temp;
    int64_t* p;
    const int64_t *max_p = descr_p->max_p;
    const int64_t *min_p = descr_p->min_p;
    int range_id = 0;
    char *endp;

    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    p = malloc(sizeof(int64_t));
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    temp = strtoll(value_str, &endp, 0);

    if (*endp != '\0')
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,("strtoll failed %s\n",
                                                    value_str));
        free(p);
        return (-1);
    }

    if ((max_p != NULL) &&
        (temp > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack sint64 failed on max check: %"
                             PRIi64 " > %" PRIi64 "\n", temp, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if ((min_p != NULL) &&
        (temp < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack sint64 failed on min check: %"
                             PRIi64 " < %" PRIi64 "\n", temp, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if (descr_p->integer_ranges_size > 0) {
        range_id = check_integer_ranges(temp,
                                        descr_p->integer_ranges,
                                        descr_p->integer_ranges_size);
        if (range_id < 0) {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Unpack sint64 failed on range check: %" PRIi64 "\n",
                                 temp));
            set_errno(E_MPL_FAILED_OPERATION);
            free(p);
            return (-1);
        }
    }

    *p = temp;

    assert(NULL != value_pp);
    *value_pp = p;

    return range_id;
}

/**
 * mpl_clone_param_value_sint64()
 **/
int
    mpl_clone_param_value_sint64(void **new_value_pp,
                                 const void* old_value_p,
                                 const mpl_param_descr2_t *descr_p)
{
    int64_t* p = malloc(sizeof(int64_t));
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    *p = *(int64_t*)old_value_p;
    *new_value_pp = p;

    return (0);
}

/**
 * mpl_copy_param_value_sint64()
 **/
int
    mpl_copy_param_value_sint64(void *to_value_p,
                                const void* from_value_p,
                                int size,
                                const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if ((size_t)size >= sizeof(int64_t))
        *(int64_t*)to_value_p = *(int64_t*)from_value_p;

    return sizeof(int64_t);
}

/**
 * mpl_compare_param_value_sint64()
 **/
int mpl_compare_param_value_sint64(const void *value1_p,
                                   const void* value2_p,
                                   const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return (*((int64_t*) value1_p) != *((int64_t*) value2_p));
}

/**
 * mpl_sizeof_param_value_sint64()
 **/
size_t mpl_sizeof_param_value_sint64(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return sizeof(int64_t);
}



/**
 * mpl_pack_param_value_uint8()
 **/
int mpl_pack_param_value_uint8(const void* param_value_p,
                               char *buf,
                               size_t buflen,
                               const mpl_param_descr2_t *descr_p,
                               const mpl_pack_options_t *options_p)
{
    assert(NULL != param_value_p);
    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p)
        return snprintf(buf, buflen, "=0x%x", *(uint8_t*)param_value_p);
}

/**
 * mpl_unpack_param_value_uint8()
 **/
int
    mpl_unpack_param_value_uint8(const char* value_str, void **value_pp,
                                 const mpl_param_descr2_t *descr_p,
                                 const mpl_pack_options_t *options_p,
                                 mpl_param_element_id_t unpack_context)
{
    int temp;
    uint8_t* p;
    const uint8_t *max_p = descr_p->max_p;
    const uint8_t *min_p = descr_p->min_p;
    int range_id = 0;

    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    p = malloc(sizeof(uint8_t));
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    if (convert_int(value_str, &temp) < 0)
    {
        free(p);
        return (-1);
    }

    if ((temp < 0) ||
        (temp > (int)UINT8_MAX))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint8 failed on range check: %d\n", temp));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if ((max_p != NULL) &&
        ((uint8_t)temp > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint8 failed on max check: %d > %d\n",
                             temp, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if ((min_p != NULL) &&
        ((uint8_t)temp < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint8 failed on min check: %d < %d\n",
                             temp, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if (descr_p->integer_ranges_size > 0) {
        range_id = check_integer_ranges(temp,
                                        descr_p->integer_ranges,
                                        descr_p->integer_ranges_size);
        if (range_id < 0) {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Unpack uint8 failed on range check: %d\n",
                                 temp));
            set_errno(E_MPL_FAILED_OPERATION);
            free(p);
            return (-1);
        }
    }

    *p = (uint8_t)temp;

    assert(NULL != value_pp);
    *value_pp = p;

    return range_id;
}

/**
 * mpl_clone_param_value_uint8()
 **/
int
    mpl_clone_param_value_uint8(void **new_value_pp,
                                const void* old_value_p,
                                const mpl_param_descr2_t *descr_p)
{
    uint8_t* p = malloc(sizeof(uint8_t));
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    *p = *(uint8_t*)old_value_p;
    *new_value_pp = p;

    return (0);
}

/**
 * mpl_copy_param_value_uint8()
 **/
int
    mpl_copy_param_value_uint8(void *to_value_p,
                               const void* from_value_p,
                               int size,
                               const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (size >= (int)sizeof(uint8_t))
        *(uint8_t*)to_value_p = *(uint8_t*)from_value_p;

    return sizeof(uint8_t);
}

/**
 * mpl_compare_param_value_uint8()
 **/
int mpl_compare_param_value_uint8(const void *value1_p,
                                  const void* value2_p,
                                  const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return (*((uint8_t*) value1_p) != *((uint8_t*) value2_p));
}

/**
 * mpl_sizeof_param_value_uint8()
 **/
size_t mpl_sizeof_param_value_uint8(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return sizeof(uint8_t);
}

/**
 * mpl_pack_param_value_uint16()
 **/
int mpl_pack_param_value_uint16(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p)
{
    assert(NULL != param_value_p);
    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p)
        return snprintf(buf, buflen, "=0x%" PRIx32, *(uint16_t*)param_value_p);
}

/**
 * mpl_unpack_param_value_uint16()
 **/
int
    mpl_unpack_param_value_uint16(const char* value_str, void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context)
{
    uint64_t temp;
    uint16_t* p;
    const uint16_t *max_p = descr_p->max_p;
    const uint16_t *min_p = descr_p->min_p;
    int range_id = 0;
    char *endp;
    int base = 10;

    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    p = malloc(sizeof(uint16_t));
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    if ((NULL != strstr(value_str, "0x")) ||
        (NULL != strstr(value_str, "0X")))
        base = 16;

    temp = strtoull(value_str, &endp, base);

    if (*endp != '\0')
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,("strtoull failed %s\n",
                                                    value_str));
        free(p);
        return (-1);
    }

    if (temp > UINT16_MAX)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint16 failed on range check: 0x%"
                             PRIu64 "\n", temp));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if ((max_p != NULL) &&
        ((uint16_t)temp > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint16 failed on max check: %"
                             PRIu64 " > %" PRIu16 "\n", temp, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if ((min_p != NULL) &&
        ((uint16_t)temp < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint16 failed on min check: %"
                             PRIu64 " < %" PRIu16 "\n", temp, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if (descr_p->integer_ranges_size > 0) {
        range_id = check_integer_ranges(temp,
                                        descr_p->integer_ranges,
                                        descr_p->integer_ranges_size);
        if (range_id < 0) {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Unpack uint16 failed on range check: %" PRIu64 "\n",
                                 temp));
            set_errno(E_MPL_FAILED_OPERATION);
            free(p);
            return (-1);
        }
    }

    *p = (uint16_t)temp;

    assert(NULL != value_pp);
    *value_pp = p;

    return range_id;
}

/**
 * mpl_clone_param_value_uint16()
 **/
int
    mpl_clone_param_value_uint16(void **new_value_pp,
                                 const void* old_value_p,
                                 const mpl_param_descr2_t *descr_p)
{
    uint16_t* p = malloc(sizeof(uint16_t));
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    *p = *(uint16_t*)old_value_p;
    *new_value_pp = p;

    return (0);
}

/**
 * mpl_copy_param_value_uint16()
 **/
int
    mpl_copy_param_value_uint16(void *to_value_p,
                                const void* from_value_p,
                                int size,
                                const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (size >= (int)sizeof(uint16_t))
        *(uint16_t*)to_value_p = *(uint16_t*)from_value_p;

    return sizeof(uint16_t);
}

/**
 * mpl_compare_param_value_uint16()
 **/
int mpl_compare_param_value_uint16(const void *value1_p,
                                   const void* value2_p,
                                   const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return (*((uint16_t*) value1_p) != *((uint16_t*) value2_p));
}

/**
 * mpl_sizeof_param_value_uint16()
 **/
size_t mpl_sizeof_param_value_uint16(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return sizeof(uint16_t);
}


/**
 * mpl_pack_param_value_uint32()
 **/
int mpl_pack_param_value_uint32(const void* param_value_p,
                                char *buf, size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p)
{
    assert(NULL != param_value_p);
    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p)
        return snprintf(buf, buflen, "=0x%" PRIx32, *(uint32_t*)param_value_p);
}

/**
 * mpl_unpack_param_value_uint32()
 **/
int
    mpl_unpack_param_value_uint32(const char* value_str, void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context)
{
    uint64_t temp;
    uint32_t* p;
    const uint32_t *max_p = descr_p->max_p;
    const uint32_t *min_p = descr_p->min_p;
    int range_id = 0;
    char *endp;
    int base = 10;

    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    p = malloc(sizeof(uint32_t));
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    if ((NULL != strstr(value_str, "0x")) ||
        (NULL != strstr(value_str, "0X")))
        base = 16;

    temp = strtoull(value_str, &endp, base);

    if (*endp != '\0')
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,("strtoull failed %s\n",
                                                    value_str));
        free(p);
        return (-1);
    }

    if (temp > UINT32_MAX)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint32 failed on range check: 0x%"
                             PRIu64 "\n", temp));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if ((max_p != NULL) &&
        ((uint32_t)temp > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint32 failed on max check: %"
                             PRIu64 " > %" PRIu32 "\n", temp, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if ((min_p != NULL) &&
        ((uint32_t)temp < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint32 failed on min check: %"
                             PRIu64 " < %" PRIu32 "\n", temp, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if (descr_p->integer_ranges_size > 0) {
        range_id = check_integer_ranges(temp,
                                        descr_p->integer_ranges,
                                        descr_p->integer_ranges_size);
        if (range_id < 0) {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Unpack uint32 failed on range check: %" PRIu64 "\n",
                                 temp));
            set_errno(E_MPL_FAILED_OPERATION);
            free(p);
            return (-1);
        }
    }

    *p = (uint32_t)temp;

    assert(NULL != value_pp);
    *value_pp = p;

    return range_id;
}

/**
 * mpl_clone_param_value_uint32()
 **/
int
    mpl_clone_param_value_uint32(void **new_value_pp,
                                 const void* old_value_p,
                                 const mpl_param_descr2_t *descr_p)
{
    uint32_t* p = malloc(sizeof(uint32_t));
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    *p = *(uint32_t*)old_value_p;
    *new_value_pp = p;

    return (0);
}

/**
 * mpl_copy_param_value_uint32()
 **/
int
    mpl_copy_param_value_uint32(void *to_value_p,
                                const void* from_value_p,
                                int size,
                                const mpl_param_descr2_t *descr_p)
{
    if (size >= (int)sizeof(uint32_t))
        *(uint32_t*)to_value_p = *(uint32_t*)from_value_p;

    return sizeof(uint32_t);
}

/**
 * mpl_compare_param_value_uint32()
 **/
int mpl_compare_param_value_uint32(const void *value1_p,
                                   const void* value2_p,
                                   const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return (*((uint32_t*) value1_p) != *((uint32_t*) value2_p));
}

/**
 * mpl_sizeof_param_value_uint32()
 **/
size_t mpl_sizeof_param_value_uint32(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return sizeof(uint32_t);
}


/**
 * mpl_pack_param_value_uint64()
 **/
int mpl_pack_param_value_uint64(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p)
{
    assert(NULL != param_value_p);
    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p);
    return snprintf(buf, buflen, "=0x%" PRIx64, *(uint64_t*)param_value_p);
}

/**
 * mpl_unpack_param_value_uint64()
 **/
int
    mpl_unpack_param_value_uint64(const char* value_str, void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context)
{
    uint64_t temp;
    uint64_t* p;
    const uint64_t *max_p = descr_p->max_p;
    const uint64_t *min_p = descr_p->min_p;
    int range_id = 0;
    int base = 10;
    char *endp = NULL;

    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    p = malloc(sizeof(uint64_t));
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    if ((NULL != strstr(value_str, "0x")) ||
        (NULL != strstr(value_str, "0X")))
        base = 16;

    temp = strtoull(value_str, &endp, base);

    if (*endp != '\0')
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,("strtoull failed %s\n",
                                                    value_str));
        free(p);
        return (-1);
    }

    if ((max_p != NULL) &&
        (temp > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint64 failed on max check: %"
                             PRIu64 " > %" PRIu64 "\n", temp, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if ((min_p != NULL) &&
        (temp < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint64 failed on min check: %"
                             PRIu64 " < %" PRIu64 "\n", temp, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(p);
        return (-1);
    }

    if (descr_p->integer_ranges_size > 0) {
        range_id = check_integer_ranges(temp,
                                        descr_p->integer_ranges,
                                        descr_p->integer_ranges_size);
        if (range_id < 0) {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Unpack uint64 failed on range check: %" PRIu64 "\n",
                                 temp));
            set_errno(E_MPL_FAILED_OPERATION);
            free(p);
            return (-1);
        }
    }

    *p = temp;

    assert(NULL != value_pp);
    *value_pp = p;

    return range_id;
}

/**
 * mpl_clone_param_value_uint64()
 **/
int
    mpl_clone_param_value_uint64(void **new_value_pp,
                                 const void* old_value_p,
                                 const mpl_param_descr2_t *descr_p)
{
    uint64_t* p = malloc(sizeof(uint64_t));
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    *p = *(uint64_t*)old_value_p;
    *new_value_pp = p;

    return (0);
}

/**
 * mpl_copy_param_value_uint64()
 **/
int
    mpl_copy_param_value_uint64(void *to_value_p,
                                const void* from_value_p,
                                int size,
                                const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if ((size_t)size >= sizeof(uint64_t))
        *(uint64_t*)to_value_p = *(uint64_t*)from_value_p;

    return sizeof(uint64_t);
}

/**
 * mpl_compare_param_value_uint64()
 **/
int mpl_compare_param_value_uint64(const void *value1_p,
                                   const void* value2_p,
                                   const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return (*((uint64_t*) value1_p) != *((uint64_t*) value2_p));
}

/**
 * mpl_sizeof_param_value_uint64()
 **/
size_t mpl_sizeof_param_value_uint64(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return sizeof(uint64_t);
}

/**
 * mpl_pack_param_value_enum()
 **/
int mpl_pack_param_value_enum (const void* param_value_p,
                               char *buf,
                               size_t buflen,
                               const mpl_param_descr2_t *descr_p,
                               const mpl_pack_options_t *options_p)
{
    int64_t value;
    const char *name_p;
    MPL_IDENTIFIER_NOT_USED(options_p)
        assert(NULL != param_value_p);
    assert(NULL != descr_p->enum_values);

    switch (descr_p->enum_representation_bytesize) {
        case 1:
            if (descr_p->enum_representation_signed)
                value = *((sint8_t*)param_value_p);
            else
                value = *((uint8_t*)param_value_p);
            break;
        case 2:
            if (descr_p->enum_representation_signed)
                value = *((sint16_t*)param_value_p);
            else
                value = *((uint16_t*)param_value_p);
            break;
        case 4:
            if (descr_p->enum_representation_signed)
                value = *((sint32_t*)param_value_p);
            else
                value = *((uint32_t*)param_value_p);
            break;
        default:
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Pack enum failed: no support for 8 byte enum representations\n"));
            set_errno(E_MPL_FAILED_OPERATION);
            return (-1);
    }

    name_p = convert_enum_value_to_name(value, descr_p->enum_values, descr_p->enum_values_size);
    if (name_p == NULL)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Pack enum failed: unknown value %" PRIi64 "\n",
                             value));
        set_errno(E_MPL_FAILED_OPERATION);
        return (-1);
    }
    return snprintf(buf, buflen, "=%s", name_p);
}

#define DEFINE_MPL_PACK_PARAM_VALUE_ENUM(enum_name, enum_type, format)         \
    int mpl_pack_param_value_##enum_name (const void* param_value_p,    \
                                          char *buf,                    \
                                          size_t buflen,                \
                                          const mpl_param_descr2_t *descr_p, \
                                          const mpl_pack_options_t *options_p) \
    {                                                                   \
        enum_type value;                                                \
        const char *name_p;                                             \
        MPL_IDENTIFIER_NOT_USED(options_p)                              \
            assert(NULL != param_value_p);                              \
        assert(NULL != descr_p->enum_values);                           \
                                                                        \
        value = *((enum_type*)param_value_p);                           \
        name_p = convert_enum_value_to_name(value, descr_p->enum_values, descr_p->enum_values_size); \
        if (name_p == NULL)                                             \
        {                                                               \
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,                 \
                                ("Pack enum failed: unknown value %" format "\n", \
                                 value));                               \
            set_errno(E_MPL_FAILED_OPERATION);                          \
            return (-1);                                                \
        }                                                               \
                                                                        \
        return snprintf(buf, buflen, "=%s", name_p);                    \
    }                                                                   \


/**
 * mpl_unpack_param_value_enum()
 **/
int mpl_unpack_param_value_enum(const char* value_str,
                                void **value_pp,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p,
                                mpl_param_element_id_t unpack_context)
{
    void* p = malloc(descr_p->enum_representation_bytesize);
    int64_t value;
    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }
    if (convert_enum_name_to_int64(value_str,
                                   &value,
                                   descr_p->enum_values,
                                   descr_p->enum_values_size) < 0)
    {
        int64_t i;
        if (convert_int64(value_str, &i) < 0)
        {
            free(p);
            return (-1);
        }
        value = i;
        if (convert_enum_value_to_name(value,
                                       descr_p->enum_values,
                                       descr_p->enum_values_size) == NULL)
        {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Unpack enum failed: unknown value %" PRIi64 "\n",
                                 i));
            set_errno(E_MPL_FAILED_OPERATION);
            free(p);
            return (-1);
        }
    }
    assert(NULL != value_pp);

    switch (descr_p->enum_representation_bytesize) {
        case 1:
            if (descr_p->enum_representation_signed)
                *((sint8_t*)p) = value;
            else
                *((uint8_t*)p) = value;
            break;
        case 2:
            if (descr_p->enum_representation_signed)
                *((sint16_t*)p) = value;
            else
                *((uint16_t*)p) = value;
            break;
        case 4:
            if (descr_p->enum_representation_signed)
                *((sint32_t*)p) = value;
            else
                *((uint32_t*)p) = value;
            break;
        default:
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                                ("Unpack enum failed: no support for 8 byte enum representations\n"));
            set_errno(E_MPL_FAILED_OPERATION);
            free(p);
            return (-1);
    }
    *value_pp = p;
    return (0);
}


#define DEFINE_MPL_UNPACK_PARAM_VALUE_ENUM(enum_name, enum_type)        \
    int mpl_unpack_param_value_##enum_name (const char* value_str,      \
                                            void **value_pp,            \
                                            const mpl_param_descr2_t *descr_p, \
                                            const mpl_pack_options_t *options_p, \
                                            mpl_param_element_id_t unpack_context) \
    {                                                                   \
        enum_type* p = malloc(sizeof(enum_type));                       \
        int64_t value;                                                  \
                                                                        \
        MPL_IDENTIFIER_NOT_USED(options_p);                             \
        MPL_IDENTIFIER_NOT_USED(unpack_context);                        \
                                                                        \
        if (NULL == p)                                                  \
        {                                                               \
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,         \
                                ("Failed allocating memory\n"));        \
            set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);                  \
            return (-1);                                                \
        }                                                               \
                                                                        \
        if (convert_enum_name_to_int64(value_str,                       \
                                       &value,                          \
                                       descr_p->enum_values,            \
                                       descr_p->enum_values_size) < 0)  \
        {                                                               \
            int64_t i;                                                      \
                                                                        \
            if (convert_int64(value_str, &i) < 0)                         \
            {                                                           \
                free(p);                                                \
                return (-1);                                            \
            }                                                           \
                                                                        \
            value = i;                                                  \
                                                                        \
            if (convert_enum_value_to_name(value,                       \
                                           descr_p->enum_values,        \
                                           descr_p->enum_values_size) == NULL) \
            {                                                           \
                MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,             \
                                    ("Unpack enum failed: unknown value %" PRIi64 "\n", \
                                     i));                               \
                set_errno(E_MPL_FAILED_OPERATION);                      \
                free(p);                                                \
                return (-1);                                            \
            }                                                           \
        }                                                               \
                                                                        \
        assert(NULL != value_pp);                                       \
        *p = (enum_type)value;                                          \
        *value_pp = p;                                                  \
                                                                        \
        return (0);                                                     \
    }


/**
 * mpl_clone_param_value_enum()
 **/
    int
    mpl_clone_param_value_enum(void **new_value_pp,
                               const void* old_value_p,
                               const mpl_param_descr2_t *descr_p)
{
    void* p = malloc(descr_p->enum_representation_bytesize);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    memcpy(p, old_value_p, descr_p->enum_representation_bytesize);
    *new_value_pp = p;

    return (0);
}

/**
 * mpl_copy_param_value_enum()
 **/
int
    mpl_copy_param_value_enum(void *to_value_p,
                              const void* from_value_p,
                              int size,
                              const mpl_param_descr2_t *descr_p)
{
    if (size >= (int)descr_p->enum_representation_bytesize)
        memcpy(to_value_p, from_value_p, descr_p->enum_representation_bytesize);
    return descr_p->enum_representation_bytesize;
}

/**
 * mpl_compare_param_value_enum()
 **/
int mpl_compare_param_value_enum(const void *value1_p,
                                 const void* value2_p,
                                 const mpl_param_descr2_t *descr_p)
{
    return memcmp(value1_p, value2_p, descr_p->enum_representation_bytesize);
}

/**
 * mpl_sizeof_param_value_enum()
 **/
size_t mpl_sizeof_param_value_enum(const mpl_param_descr2_t *descr_p)
{
    return descr_p->enum_representation_bytesize;
}

/**
 * mpl_pack_param_value_enum8()
 **/
DEFINE_MPL_PACK_PARAM_VALUE_ENUM(enum8, uint8_t, "d")


/**
 * mpl_unpack_param_value_enum8()
 **/
    DEFINE_MPL_UNPACK_PARAM_VALUE_ENUM(enum8, uint8_t)

/**
 * mpl_pack_param_value_enum16()
 **/
    DEFINE_MPL_PACK_PARAM_VALUE_ENUM(enum16, uint16_t, "d")

/**
 * mpl_unpack_param_value_enum16()
 **/
    DEFINE_MPL_UNPACK_PARAM_VALUE_ENUM(enum16, uint16_t)


/**
 * mpl_pack_param_value_enum32()
 **/
    DEFINE_MPL_PACK_PARAM_VALUE_ENUM(enum32, uint32_t, "d")

/**
 * mpl_unpack_param_value_enum32()
 **/
    DEFINE_MPL_UNPACK_PARAM_VALUE_ENUM(enum32, uint32_t)

/**
 * mpl_pack_param_value_signed_enum8()
 **/
    DEFINE_MPL_PACK_PARAM_VALUE_ENUM(signed_enum8, sint8_t, "d")

/**
 * mpl_unpack_param_value_signed_enum8()
 **/
    DEFINE_MPL_UNPACK_PARAM_VALUE_ENUM(signed_enum8, sint8_t)

/**
 * mpl_pack_param_value_signed_enum16()
 **/
    DEFINE_MPL_PACK_PARAM_VALUE_ENUM(signed_enum16, sint16_t, "d")

/**
 * mpl_unpack_param_value_signed_enum16()
 **/
    DEFINE_MPL_UNPACK_PARAM_VALUE_ENUM(signed_enum16, sint16_t)

/**
 * mpl_pack_param_value_signed_enum32()
 **/
    DEFINE_MPL_PACK_PARAM_VALUE_ENUM(signed_enum32, sint32_t, "d")

/**
 * mpl_unpack_param_value_signed_enum32()
 **/
    DEFINE_MPL_UNPACK_PARAM_VALUE_ENUM(signed_enum32, sint32_t)

/**
 * mpl_pack_param_value_bool()
 **/
    int mpl_pack_param_value_bool(const void* param_value_p,
                                  char *buf,
                                  size_t buflen,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p)
{
    int index;

    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p)

        assert(NULL != param_value_p);

    index = *(bool*)param_value_p;
    if (index >= (int)ARRAY_SIZE(mpl_names_bool))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Pack bool failed on range check: %d >= %zu\n",
                             index, ARRAY_SIZE(mpl_names_bool)));/*lint !e557 %zu is C99 */
        set_errno(E_MPL_FAILED_OPERATION);
        return -1;
    }

    return snprintf(buf, buflen, "=%s", mpl_names_bool[index]);
}

/**
 * mpl_unpack_param_value_bool()
 **/
int mpl_unpack_param_value_bool(const char* value_str, void **value_pp,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p,
                                mpl_param_element_id_t unpack_context)
{
    int temp;
    bool* p = malloc(sizeof(bool));

    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    if (convert_stringarr_to_int(value_str, &temp,
                                 mpl_names_bool, ARRAY_SIZE(mpl_names_bool)) < 0)
    {
        if (convert_int(value_str, &temp) < 0)
        {
            free(p);
            return (-1);
        }
    }

    *p = (bool)temp;

    assert(NULL != value_pp);
    *value_pp = p;

    return (0);
}

int mpl_clone_param_value_bool(void **new_value_pp,
                               const void* old_value_p,
                               const mpl_param_descr2_t *descr_p)
{
    bool* p = malloc(sizeof(bool));
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    *p = *(bool*)old_value_p;
    *new_value_pp = p;

    return (0);
}

/**
 * mpl_copy_param_value_bool()
 **/
int
    mpl_copy_param_value_bool(void *to_value_p,
                              const void* from_value_p,
                              int size,
                              const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (size >= (int)sizeof(bool))
        *(bool*)to_value_p = *(bool*)from_value_p;

    return sizeof(bool);
}

/**
 * mpl_compare_param_value_bool()
 **/
int mpl_compare_param_value_bool(const void *value1_p,
                                 const void* value2_p,
                                 const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return (*((bool*) value1_p) != *((bool*) value2_p));
}

/**
 * mpl_sizeof_param_value_bool()
 **/
size_t mpl_sizeof_param_value_bool(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return sizeof(bool);
}

/**
 * mpl_pack_param_value_bool8()
 **/
int mpl_pack_param_value_bool8(const void* param_value_p,
                               char *buf,
                               size_t buflen,
                               const mpl_param_descr2_t *descr_p,
                               const mpl_pack_options_t *options_p)
{
    int index;

    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p)

        assert(NULL != param_value_p);

    index = *(uint8_t*)param_value_p;
    if (index >= (int)ARRAY_SIZE(mpl_names_bool8))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Pack bool8 failed on range check: %d >= %zu\n",
                             index, ARRAY_SIZE(mpl_names_bool8)));/*lint !e557 %zu is C99 */
        set_errno(E_MPL_FAILED_OPERATION);
        return -1;
    }

    return snprintf(buf, buflen, "=%s", mpl_names_bool8[index]);
}

/**
 * mpl_unpack_param_value_bool8()
 **/
int mpl_unpack_param_value_bool8(const char* value_str,
                                 void **value_pp,
                                 const mpl_param_descr2_t *descr_p,
                                 const mpl_pack_options_t *options_p,
                                 mpl_param_element_id_t unpack_context)
{
    int temp;
    uint8_t* p = malloc(sizeof(uint8_t));

    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    if (convert_stringarr_to_int(value_str, &temp,
                                 mpl_names_bool8,
                                 ARRAY_SIZE(mpl_names_bool8)) < 0)
    {
        if (convert_int(value_str, &temp) < 0)
        {
            free(p);
            return (-1);
        }
    }

    *p = (uint8_t)temp;

    assert(NULL != value_pp);
    *value_pp = p;

    return (0);
}


/**
 * mpl_pack_param_value_uint8_array()
 **/
int mpl_pack_param_value_uint8_array(const void* param_value_p,
                                     char *buf,
                                     size_t buflen,
                                     const mpl_param_descr2_t *descr_p,
                                     const mpl_pack_options_t *options_p)
{
    int i;
    const mpl_uint8_array_t *a_p;
    int len;
    int total_len;

    assert(NULL != param_value_p);
    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p)

        a_p = param_value_p;

    len = snprintf(buf, buflen, "=%08x", a_p->len);
    if ((int)buflen > len)
        buf += len;
    total_len = len;

    for (i = 0; i < a_p->len; i++)
    {
        int left;

        left = (int)buflen > total_len ? (int)buflen - total_len : 0;

        len = snprintf(buf, left, "%02x", a_p->arr_p[i]);
        if (left > len)
            buf += len;
        total_len += len;
    }

    return total_len;
}


/**
 * mpl_unpack_param_value_uint8_array()
 **/
int
    mpl_unpack_param_value_uint8_array(const char* value_str, void **value_pp,
                                       const mpl_param_descr2_t *descr_p,
                                       const mpl_pack_options_t *options_p,
                                       mpl_param_element_id_t unpack_context)
{
    unsigned int len;
    unsigned int i;
    unsigned int val;
    mpl_uint8_array_t *a_p;
    uint8_t* p;
    const int *max_p = descr_p->max_p;
    const int *min_p = descr_p->min_p;

    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    a_p = malloc(sizeof(mpl_uint8_array_t));
    if (NULL == a_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    if (sscanf(value_str, "%8x", &len) <= 0)
    {
        free(a_p);
        return (-1);
    }

    if ((max_p != NULL) &&
        ((int)len > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint8_array failed on max length check: %d > %d\n",
                             len, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(a_p);
        return (-1);
    }

    if ((min_p != NULL) &&
        ((int)len < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint8_array failed on min length check: %d < %d\n",
                             len, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(a_p);
        return (-1);
    }

    a_p->arr_p = malloc(len * sizeof(uint8_t));

    if (NULL == a_p->arr_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(a_p);
        return (-1);
    }

    p = (uint8_t *) value_str + 8;
    for (i = 0; i < len; i++)
    {
        if (sscanf((char*) p, "%2x", &val) <= 0)
        {
            free(a_p->arr_p);
            free(a_p);
            return -1;
        }
        a_p->arr_p[i] = (uint8_t) val;
        p += 2;
    }

    a_p->len = len;

    assert(NULL != value_pp);
    *value_pp = a_p;

    return (0);
}

/**
 * mpl_clone_param_value_uint8_array()
 **/
int
    mpl_clone_param_value_uint8_array(void **new_value_pp,
                                      const void* old_value_p,
                                      const mpl_param_descr2_t *descr_p)
{
    mpl_uint8_array_t *a_p;
    const mpl_uint8_array_t *old_a_p;

    MPL_IDENTIFIER_NOT_USED(descr_p);
    a_p = malloc(sizeof(mpl_uint8_array_t));
    if (NULL == a_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    old_a_p = old_value_p;

    a_p->arr_p = malloc(old_a_p->len * sizeof(uint8_t));
    if (NULL == a_p->arr_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(a_p);
        return (-1);
    }

    a_p->len = old_a_p->len;
    memcpy(a_p->arr_p, old_a_p->arr_p, a_p->len * sizeof(uint8_t));

    *new_value_pp = a_p;

    return (0);
}

/**
 * mpl_copy_param_value_uint8_array()
 **/
int
    mpl_copy_param_value_uint8_array(void *to_value_p,
                                     const void* from_value_p,
                                     int size,
                                     const mpl_param_descr2_t *descr_p)
{
    const mpl_uint8_array_t *arr_p = from_value_p;
    int real_size = arr_p->len;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    if (real_size <= size )
        memcpy(to_value_p, arr_p->arr_p, real_size);

    return real_size;
}

/**
 * mpl_compare_param_value_uint8_array()
 **/
int mpl_compare_param_value_uint8_array(const void *value1_p,
                                        const void* value2_p,
                                        const mpl_param_descr2_t *descr_p)
{
    const mpl_uint8_array_t *a1_p;
    const mpl_uint8_array_t *a2_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    a1_p = value1_p;
    a2_p = value2_p;

    if (a1_p->len != a2_p->len)
        return -1;

    return memcmp(a1_p->arr_p, a2_p->arr_p, a1_p->len * sizeof(uint8_t));
}

/**
 * mpl_sizeof_param_value_uint8_array()
 **/
size_t mpl_sizeof_param_value_uint8_array(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return 0;
}

/**
 * mpl_free_param_value_uint8_array()
 **/
void mpl_free_param_value_uint8_array(void *value_p)
{
    mpl_uint8_array_t *a_p;

    a_p = value_p;
    if (NULL != a_p)
    {
        if (NULL != a_p->arr_p)
        {
            free(a_p->arr_p);
        }
        free(a_p);
    }
}


/**
 * mpl_pack_param_value_uint16_array()
 **/
int mpl_pack_param_value_uint16_array(const void* param_value_p,
                                      char *buf,
                                      size_t buflen,
                                      const mpl_param_descr2_t *descr_p,
                                      const mpl_pack_options_t *options_p)
{
    int i;
    const mpl_uint16_array_t *a_p;
    int len;
    int total_len;

    assert(NULL != param_value_p);
    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p)

        a_p = param_value_p;

    len = snprintf(buf, buflen, "=%08x", a_p->len);
    if ((int)buflen > len)
        buf += len;
    total_len = len;

    for (i = 0; i < a_p->len; i++)
    {
        int left;

        left = (int)buflen > total_len ? (int)buflen - total_len : 0;

        len = snprintf(buf, left, "%04x", a_p->arr_p[i]);
        if (left > len)
            buf += len;
        total_len += len;
    }

    return total_len;
}


/**
 * mpl_unpack_param_value_uint16_array()
 **/
int
    mpl_unpack_param_value_uint16_array(const char* value_str, void **value_pp,
                                        const mpl_param_descr2_t *descr_p,
                                        const mpl_pack_options_t *options_p,
                                        mpl_param_element_id_t unpack_context)
{
    unsigned int len;
    unsigned int i;
    unsigned int val;
    mpl_uint16_array_t *a_p;
    uint8_t* p;
    const int *max_p = descr_p->max_p;
    const int *min_p = descr_p->min_p;

    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    a_p = malloc(sizeof(mpl_uint16_array_t));
    if (NULL == a_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    if (sscanf(value_str, "%8x", &len) <= 0)
    {
        free(a_p);
        return (-1);
    }

    if ((max_p != NULL) &&
        ((int)len > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint16_array failed on "
                             "max length check: %d > %d\n", len, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(a_p);
        return (-1);
    }

    if ((min_p != NULL) &&
        ((int)len < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint16_array failed on "
                             "min length check: %d < %d\n", len, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(a_p);
        return (-1);
    }

    a_p->arr_p = malloc(len * sizeof(uint16_t));

    if (NULL == a_p->arr_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(a_p);
        return (-1);
    }

    p = (uint8_t *) value_str + 8;
    for (i = 0; i < len; i++)
    {
        if (sscanf((char*) p, "%4x", &val) <= 0)
        {
            free(a_p->arr_p);
            free(a_p);
            return -1;
        }
        a_p->arr_p[i] = (uint16_t) val;
        p += 4;
    }

    a_p->len = len;

    assert(NULL != value_pp);
    *value_pp = a_p;

    return (0);
}

/**
 * mpl_clone_param_value_uint16_array()
 **/
int
    mpl_clone_param_value_uint16_array(void **new_value_pp,
                                       const void* old_value_p,
                                       const mpl_param_descr2_t *descr_p)
{
    mpl_uint16_array_t *a_p;
    const mpl_uint16_array_t *old_a_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    a_p = malloc(sizeof(mpl_uint16_array_t));
    if (NULL == a_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    old_a_p = old_value_p;

    a_p->arr_p = malloc(old_a_p->len * sizeof(uint16_t));
    if (NULL == a_p->arr_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(a_p);
        return (-1);
    }

    a_p->len = old_a_p->len;
    memcpy(a_p->arr_p, old_a_p->arr_p, a_p->len * sizeof(uint16_t));

    *new_value_pp = a_p;

    return (0);
}

/**
 * mpl_copy_param_value_uint16_array()
 **/
int
    mpl_copy_param_value_uint16_array(void *to_value_p,
                                      const void* from_value_p,
                                      int size,
                                      const mpl_param_descr2_t *descr_p)
{
    const mpl_uint16_array_t *arr_p = from_value_p;
    int real_size = arr_p->len * sizeof(uint16_t);
    MPL_IDENTIFIER_NOT_USED(descr_p);

    if (real_size <= size )
        memcpy(to_value_p, arr_p->arr_p, real_size);

    return real_size;
}

/**
 * mpl_compare_param_value_uint16_array()
 **/
int mpl_compare_param_value_uint16_array(const void *value1_p,
                                         const void* value2_p,
                                         const mpl_param_descr2_t *descr_p)
{
    const mpl_uint16_array_t *a1_p;
    const mpl_uint16_array_t *a2_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    a1_p = value1_p;
    a2_p = value2_p;

    if (a1_p->len != a2_p->len)
        return -1;

    return memcmp(a1_p->arr_p, a2_p->arr_p, a1_p->len * sizeof(uint16_t));
}

/**
 * mpl_sizeof_param_value_uint16_array()
 **/
size_t mpl_sizeof_param_value_uint16_array(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return 0;
}

/**
 * mpl_free_param_value_uint16_array()
 **/
void mpl_free_param_value_uint16_array(void *value_p)
{
    mpl_uint16_array_t *a_p;

    a_p = value_p;
    if (NULL != a_p)
    {
        if (NULL != a_p->arr_p)
        {
            free(a_p->arr_p);
        }
        free(a_p);
    }
}


/**
 * mpl_pack_param_value_uint32_array()
 **/
int mpl_pack_param_value_uint32_array(const void* param_value_p,
                                      char *buf,
                                      size_t buflen,
                                      const mpl_param_descr2_t *descr_p,
                                      const mpl_pack_options_t *options_p)
{
    unsigned int i;
    const mpl_uint32_array_t *a_p;
    int len;
    int total_len;

    assert(NULL != param_value_p);
    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p)

        a_p = param_value_p;

    len = snprintf(buf, buflen, "=%08x", a_p->len);
    if ((int)buflen > len)
        buf += len;
    total_len = len;

    for (i = 0; i < a_p->len; i++)
    {
        int left;

        left = (int)buflen > total_len ? (int)buflen - total_len : 0;

        len = snprintf(buf, left, "%08x", a_p->arr_p[i]);
        if (left > len)
            buf += len;
        total_len += len;
    }

    return total_len;
}


/**
 * mpl_unpack_param_value_uint32_array()
 **/
int
    mpl_unpack_param_value_uint32_array(const char* value_str,
                                        void **value_pp,
                                        const mpl_param_descr2_t *descr_p,
                                        const mpl_pack_options_t *options_p,
                                        mpl_param_element_id_t unpack_context)
{
    unsigned int len;
    unsigned int i;
    unsigned int val;
    mpl_uint32_array_t *a_p;
    uint8_t* p;
    const int *max_p = descr_p->max_p;
    const int *min_p = descr_p->min_p;

    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    a_p = malloc(sizeof(mpl_uint32_array_t));
    if (NULL == a_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    if (sscanf(value_str, "%8x", &len) <= 0)
    {
        free(a_p);
        return (-1);
    }

    if ((max_p != NULL) &&
        ((int)len > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint32_array failed on "
                             "max length check: %d > %d\n", len, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(a_p);
        return (-1);
    }

    if ((min_p != NULL) &&
        ((int)len < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack uint32_array failed on "
                             "min length check: %d < %d\n", len, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(a_p);
        return (-1);
    }

    a_p->arr_p = malloc(len * sizeof(uint32_t));

    if (NULL == a_p->arr_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(a_p);
        return (-1);
    }

    p = (uint8_t *) value_str + 8;
    for (i = 0; i < len; i++)
    {
        if (sscanf((char*) p, "%8x", &val) <= 0)
        {
            free(a_p->arr_p);
            free(a_p);
            return -1;
        }
        a_p->arr_p[i] = (uint32_t) val;
        p += 8;
    }

    a_p->len = len;

    assert(NULL != value_pp);
    *value_pp = a_p;

    return (0);
}

/**
 * mpl_clone_param_value_uint32_array()
 **/
int
    mpl_clone_param_value_uint32_array(void **new_value_pp,
                                       const void* old_value_p,
                                       const mpl_param_descr2_t *descr_p)
{
    mpl_uint32_array_t *a_p;
    const mpl_uint32_array_t *old_a_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    a_p = malloc(sizeof(mpl_uint32_array_t));
    if (NULL == a_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    old_a_p = old_value_p;

    a_p->arr_p = malloc(old_a_p->len * sizeof(uint32_t));
    if (NULL == a_p->arr_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(a_p);
        return (-1);
    }

    a_p->len = old_a_p->len;
    memcpy(a_p->arr_p, old_a_p->arr_p, a_p->len * sizeof(uint32_t));

    *new_value_pp = a_p;

    return (0);
}

/**
 * mpl_copy_param_value_uint32_array()
 **/
int
    mpl_copy_param_value_uint32_array(void *to_value_p,
                                      const void* from_value_p,
                                      int size,
                                      const mpl_param_descr2_t *descr_p)
{
    const mpl_uint32_array_t *arr_p = from_value_p;
    int real_size = arr_p->len * sizeof(uint32_t);
    MPL_IDENTIFIER_NOT_USED(descr_p);

    if (real_size <= size )
        memcpy(to_value_p, arr_p->arr_p, real_size);

    return real_size;
}

/**
 * mpl_compare_param_value_uint32_array()
 **/
int mpl_compare_param_value_uint32_array(const void *value1_p,
                                         const void* value2_p,
                                         const mpl_param_descr2_t *descr_p)
{
    const mpl_uint32_array_t *a1_p;
    const mpl_uint32_array_t *a2_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    a1_p = value1_p;
    a2_p = value2_p;

    if (a1_p->len != a2_p->len)
        return -1;

    return memcmp(a1_p->arr_p, a2_p->arr_p, a1_p->len * sizeof(uint32_t));
}

/**
 * mpl_sizeof_param_value_uint32_array()
 **/
size_t mpl_sizeof_param_value_uint32_array(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return 0;
}

/**
 * mpl_free_param_value_uint32_array()
 **/
void mpl_free_param_value_uint32_array(void *value_p)
{
    mpl_uint32_array_t *a_p;

    a_p = value_p;
    if (NULL != a_p)
    {
        if (NULL != a_p->arr_p)
        {
            free(a_p->arr_p);
        }
        free(a_p);
    }
}


/**
 * mpl_pack_param_value_string_tuple()
 **/
int mpl_pack_param_value_string_tuple(const void* param_value_p,
                                      char *buf,
                                      size_t buflen,
                                      const mpl_param_descr2_t *descr_p,
                                      const mpl_pack_options_t *options_p)
{
    const mpl_string_tuple_t *st_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    assert(NULL != param_value_p);

    st_p = param_value_p;
    assert(NULL != st_p->key_p);

    if (NULL != st_p->value_p)
        return snprintf_escape(options_p->message_delimiter, '\\',
                               buf, buflen, "=%s:%s", st_p->key_p, st_p->value_p);
    else
        return snprintf_escape(options_p->message_delimiter, '\\',
                               buf, buflen, "=%s:", st_p->key_p);
}


/**
 * mpl_unpack_param_value_string_tuple()
 **/
int
    mpl_unpack_param_value_string_tuple(const char* value_str, void **value_pp,
                                        const mpl_param_descr2_t *descr_p,
                                        const mpl_pack_options_t *options_p,
                                        mpl_param_element_id_t unpack_context)
{
    mpl_string_tuple_t *st_p;
    char* p;
    size_t slen;
    char *temp_str;
    int temp_string_allocated = 0;
    const int *max_p = descr_p->max_p;
    const int *min_p = descr_p->min_p;
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    if (strchr(value_str, '\\')) {
        temp_str = remove_escape(value_str, options_p->message_delimiter, '\\');
        if (temp_str == NULL) {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                                ("Failed allocating memory\n"));
            set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
            return (-1);
        }
        temp_string_allocated = 1;
    }
    else {
        temp_str = (char*) value_str;
    }

    p = strchr(temp_str, ':');
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack string_tuple failed, no delimiter: %s\n",
                             temp_str));
        set_errno(E_MPL_FAILED_OPERATION);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    st_p = malloc(sizeof(mpl_string_tuple_t));
    if (NULL == st_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    slen = p - temp_str;

    if ((max_p != NULL) &&
        ((int)slen > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack string_tuple key failed on "
                             "max length check: %zu > %d\n",
                             slen,
                             *max_p));/*lint !e557 %zu is C99 */
        set_errno(E_MPL_FAILED_OPERATION);
        free(st_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    if ((min_p != NULL) &&
        ((int)slen < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack string_tuple key failed on "
                             "min length check: %zu < %d\n",
                             slen,
                             *min_p));/*lint !e557 %zu is C99 */
        set_errno(E_MPL_FAILED_OPERATION);
        free(st_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    st_p->key_p = malloc(slen + 1);
    if (NULL == st_p->key_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(st_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }
    strncpy(st_p->key_p, temp_str, slen);
    st_p->key_p[slen] = 0;

    p++;
    slen = strlen(p);

    if ((max_p != NULL) &&
        ((int)slen > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack string_tuple value failed on "
                             "max length check: %zu > %d\n",
                             slen,
                             *max_p));/*lint !e557 %zu is C99 */
        set_errno(E_MPL_FAILED_OPERATION);
        free(st_p->key_p);
        free(st_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    if ((min_p != NULL) &&
        ((int)slen < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack string_tuple value failed on "
                             "min length check: %zu < %d\n",
                             slen,
                             *min_p));/*lint !e557 %zu is C99 */
        set_errno(E_MPL_FAILED_OPERATION);
        free(st_p->key_p);
        free(st_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    st_p->value_p = malloc(slen + 1);
    if (NULL == st_p->value_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(st_p->key_p);
        free(st_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    strncpy(st_p->value_p, p, slen);
    st_p->value_p[slen] = 0;
    *value_pp = st_p;

    if (temp_string_allocated)
        free(temp_str);

    return (0);
}

/**
 * mpl_clone_param_value_string_tuple()
 **/
int
    mpl_clone_param_value_string_tuple(void **new_value_pp,
                                       const void* old_value_p,
                                       const mpl_param_descr2_t *descr_p)
{
    mpl_string_tuple_t *st_p;
    const mpl_string_tuple_t *old_st_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    st_p = malloc(sizeof(mpl_string_tuple_t));
    if (NULL == st_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    old_st_p = old_value_p;

    st_p->key_p = malloc(strlen(old_st_p->key_p) + 1);
    if (NULL == st_p->key_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(st_p);
        return (-1);
    }

    st_p->value_p = malloc(strlen(old_st_p->value_p) + 1);
    if (NULL == st_p->value_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(st_p->key_p);
        free(st_p);
        return (-1);
    }

    strcpy(st_p->key_p, old_st_p->key_p);
    strcpy(st_p->value_p, old_st_p->value_p);

    *new_value_pp = st_p;

    return (0);
}

/**
 * mpl_copy_param_value_string_tuple()
 **/
/* NOTE: Copy on a tuple means copy of the value part of the tuple */
int
    mpl_copy_param_value_string_tuple(void *to_value_p,
                                      const void* from_value_p,
                                      int size,
                                      const mpl_param_descr2_t *descr_p)
{
    const mpl_string_tuple_t *tuple_p = from_value_p;
    int real_size = strlen(tuple_p->value_p) + 1;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    if (real_size <= size)
        strncpy(to_value_p, tuple_p->value_p, real_size);

    return real_size;
}

/**
 * mpl_compare_param_value_string_tuple()
 **/
int mpl_compare_param_value_string_tuple(const void *value1_p,
                                         const void* value2_p,
                                         const mpl_param_descr2_t *descr_p)
{
    const mpl_string_tuple_t *st1_p;
    const mpl_string_tuple_t *st2_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    st1_p = value1_p;
    st2_p = value2_p;

    if ((NULL == st1_p->key_p) || (NULL == st2_p->key_p))
    {
        if (st1_p->key_p != st2_p->key_p)
            return -1;
    }
    else if (strcmp(st1_p->key_p, st2_p->key_p))
        return -1;

    if ((NULL == st1_p->value_p) || (NULL == st2_p->value_p))
    {
        if (st1_p->value_p != st2_p->value_p)
            return -1;
    }
    else if (strcmp(st1_p->value_p, st2_p->value_p))
        return -1;

    return 0;
}

/**
 * mpl_sizeof_param_value_string_tuple()
 **/
size_t mpl_sizeof_param_value_string_tuple(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return 0;
}

/**
 * mpl_free_param_value_string_tuple()
 **/
void mpl_free_param_value_string_tuple(void *value_p)
{
    mpl_string_tuple_t *st_p;

    st_p = value_p;
    if (NULL != st_p)
    {
        if (NULL != st_p->key_p)
        {
            free(st_p->key_p);
        }
        if (NULL != st_p->value_p)
        {
            free(st_p->value_p);
        }
        free(st_p);
    }
}



/**
 * mpl_pack_param_value_int_tuple()
 **/
int mpl_pack_param_value_int_tuple(const void* param_value_p,
                                   char *buf,
                                   size_t buflen,
                                   const mpl_param_descr2_t *descr_p,
                                   const mpl_pack_options_t *options_p)
{
    const mpl_int_tuple_t *it_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    assert(NULL != param_value_p);
    MPL_IDENTIFIER_NOT_USED(options_p)

        it_p = param_value_p;

    return snprintf(buf, buflen, "=%d:%d", it_p->key, it_p->value);
}


/**
 * mpl_unpack_param_value_int_tuple()
 **/
int
    mpl_unpack_param_value_int_tuple(const char* value_str, void **value_pp,
                                     const mpl_param_descr2_t *descr_p,
                                     const mpl_pack_options_t *options_p,
                                     mpl_param_element_id_t unpack_context)
{
    mpl_int_tuple_t *it_p;
    char* p;
    char *tmp_buf_p;
    const int *max_p = descr_p->max_p;
    const int *min_p = descr_p->min_p;

    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    tmp_buf_p = malloc(strlen(value_str) + 1);
    if (NULL == tmp_buf_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return -1;
    }
    strcpy(tmp_buf_p, value_str);

    p = strchr(tmp_buf_p, ':');
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack int_tuple failed, no delimiter: %s\n",
                             value_str));
        set_errno(E_MPL_FAILED_OPERATION);
        free(tmp_buf_p);
        return (-1);
    }

    *p = '\0';

    it_p = malloc(sizeof(mpl_int_tuple_t));
    if (NULL == it_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(tmp_buf_p);
        return (-1);
    }

    if (convert_int(tmp_buf_p, &it_p->key) < 0)
    {
        free(it_p);
        free(tmp_buf_p);
        return (-1);
    }

    if ((max_p != NULL) &&
        (it_p->key > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack int_tuple key failed on "
                             "max check: %d > %d\n",
                             it_p->key,
                             *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(it_p);
        free(tmp_buf_p);
        return (-1);
    }

    if ((min_p != NULL) &&
        (it_p->key < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack int_tuple key failed on "
                             "min check: %d < %d\n",
                             it_p->key,
                             *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(it_p);
        free(tmp_buf_p);
        return (-1);
    }

    p++;

    if (convert_int(p, &it_p->value) < 0)
    {
        free(it_p);
        free(tmp_buf_p);
        return (-1);
    }

    if ((max_p != NULL) &&
        (it_p->value > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack int_tuple value failed on "
                             "max check: %d > %d\n",
                             it_p->value,
                             *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(it_p);
        free(tmp_buf_p);
        return (-1);
    }

    if ((min_p != NULL) &&
        (it_p->value < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack int_tuple value failed on "
                             "min check: %d < %d\n",
                             it_p->value,
                             *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(it_p);
        free(tmp_buf_p);
        return (-1);
    }

    *value_pp = it_p;

    free(tmp_buf_p);
    return (0);
}

/**
 * mpl_clone_param_value_int_tuple()
 **/
int
    mpl_clone_param_value_int_tuple(void **new_value_pp,
                                    const void* old_value_p,
                                    const mpl_param_descr2_t *descr_p)
{
    mpl_int_tuple_t *it_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    it_p = malloc(sizeof(mpl_int_tuple_t));
    if (NULL == it_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    *it_p = *(mpl_int_tuple_t*)old_value_p;
    *new_value_pp = it_p;

    return (0);
}

/**
 * mpl_copy_param_value_int_tuple()
 **/
/* NOTE: Copy on a tuple means copy of the value part of the tuple */
int
    mpl_copy_param_value_int_tuple(void *to_value_p,
                                   const void* from_value_p,
                                   int size,
                                   const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (size >= (int)sizeof(int))
        *(int*)to_value_p = ((mpl_int_tuple_t*)from_value_p)->value;

    return sizeof(int);
}

/**
 * mpl_compare_param_value_int_tuple()
 **/
int mpl_compare_param_value_int_tuple(const void *value1_p,
                                      const void* value2_p,
                                      const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return (memcmp(value1_p, value2_p, sizeof(mpl_int_tuple_t)) != 0);
}

/**
 * mpl_sizeof_param_value_int_tuple()
 **/
size_t mpl_sizeof_param_value_int_tuple(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return sizeof(mpl_int_tuple_t);
}


/**
 * mpl_pack_param_value_strint_tuple()
 **/
int mpl_pack_param_value_strint_tuple(const void* param_value_p,
                                      char *buf,
                                      size_t buflen,
                                      const mpl_param_descr2_t *descr_p,
                                      const mpl_pack_options_t *options_p)
{
    const mpl_strint_tuple_t *t_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    assert(NULL != param_value_p);

    t_p = param_value_p;
    assert(NULL != t_p->key_p);

    return snprintf_escape(options_p->message_delimiter, '\\',
                           buf, buflen, "=%s:%d", t_p->key_p, t_p->value);
}


/**
 * mpl_unpack_param_value_strint_tuple()
 **/
int
    mpl_unpack_param_value_strint_tuple(const char* value_str, void **value_pp,
                                        const mpl_param_descr2_t *descr_p,
                                        const mpl_pack_options_t *options_p,
                                        mpl_param_element_id_t unpack_context)
{
    mpl_strint_tuple_t *t_p;
    char* p;
    int slen;
    char *temp_str;
    int temp_string_allocated = 0;
    const int *max_p = descr_p->max_p;
    const int *min_p = descr_p->min_p;
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    if (strchr(value_str, '\\')) {
        temp_str = remove_escape(value_str, options_p->message_delimiter, '\\');
        if (temp_str == NULL) {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                                ("Failed allocating memory\n"));
            set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
            return (-1);
        }
        temp_string_allocated = 1;
    }
    else {
        temp_str = (char*) value_str;
    }

    p = strchr(temp_str, ':');
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack strint_tuple failed, no delimiter: %s\n",
                             temp_str));
        set_errno(E_MPL_FAILED_OPERATION);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    t_p = malloc(sizeof(mpl_strint_tuple_t));
    if (NULL == t_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    slen = p - temp_str;

    t_p->key_p = malloc(slen + 1);
    if (NULL == t_p->key_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(t_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }
    strncpy(t_p->key_p, temp_str, slen);
    t_p->key_p[slen] = 0;


    p++;

    if (convert_int(p, &t_p->value) < 0)
    {
        free(t_p->key_p);
        free(t_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    if ((max_p != NULL) &&
        (t_p->value > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack strint_tuple value failed on "
                             "max check: %d > %d\n", t_p->value, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(t_p->key_p);
        free(t_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    if ((min_p != NULL) &&
        (t_p->value < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack strint_tuple value failed on "
                             "min check: %d < %d\n", t_p->value, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(t_p->key_p);
        free(t_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    *value_pp = t_p;

    if (temp_string_allocated)
        free(temp_str);
    return (0);
}

/**
 * mpl_clone_param_value_strint_tuple()
 **/
int
    mpl_clone_param_value_strint_tuple(void **new_value_pp,
                                       const void* old_value_p,
                                       const mpl_param_descr2_t *descr_p)
{
    mpl_strint_tuple_t *t_p;
    const mpl_strint_tuple_t *old_t_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    t_p = malloc(sizeof(mpl_strint_tuple_t));
    if (NULL == t_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    old_t_p = old_value_p;

    t_p->key_p = malloc(strlen(old_t_p->key_p) + 1);
    if (NULL == t_p->key_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(t_p);
        return (-1);
    }

    strcpy(t_p->key_p, old_t_p->key_p);
    t_p->value = old_t_p->value;

    *new_value_pp = t_p;

    return (0);
}

/**
 * mpl_copy_param_value_strint_tuple()
 **/
/* NOTE: Copy on a tuple means copy of the value part of the tuple */
int
    mpl_copy_param_value_strint_tuple(void *to_value_p,
                                      const void* from_value_p,
                                      int size,
                                      const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (size >= (int)sizeof(int))
        *(int*)to_value_p = ((mpl_strint_tuple_t*)from_value_p)->value;

    return sizeof(int);
}

/**
 * mpl_compare_param_value_strint_tuple()
 **/
int mpl_compare_param_value_strint_tuple(const void *value1_p,
                                         const void* value2_p,
                                         const mpl_param_descr2_t *descr_p)
{
    const mpl_strint_tuple_t *t1_p;
    const mpl_strint_tuple_t *t2_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    t1_p = value1_p;
    t2_p = value2_p;

    if ((NULL == t1_p->key_p) || (NULL == t2_p->key_p))
    {
        if (t1_p->key_p != t2_p->key_p)
            return -1;
    }
    else if (strcmp(t1_p->key_p, t2_p->key_p))
        return -1;

    return (t1_p->value != t2_p->value);
}

/**
 * mpl_sizeof_param_value_strint_tuple()
 **/
size_t mpl_sizeof_param_value_strint_tuple(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return 0;
}


/**
 * mpl_free_param_value_strint_tuple()
 **/
void mpl_free_param_value_strint_tuple(void *value_p)
{
    mpl_strint_tuple_t *t_p;

    t_p = value_p;
    if (NULL != t_p)
    {
        if (NULL != t_p->key_p)
        {
            free(t_p->key_p);
        }

        free(t_p);
    }
}

/**
 * mpl_pack_param_value_struint8_tuple()
 **/
int mpl_pack_param_value_struint8_tuple(const void* param_value_p,
                                        char *buf,
                                        size_t buflen,
                                        const mpl_param_descr2_t *descr_p,
                                        const mpl_pack_options_t *options_p)
{
    const mpl_struint8_tuple_t *t_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    assert(NULL != param_value_p);

    t_p = param_value_p;
    assert(NULL != t_p->key_p);

    return snprintf_escape(options_p->message_delimiter, '\\',
                           buf, buflen, "=%s/%d", t_p->key_p, t_p->value);
}


/**
 * mpl_unpack_param_value_struint8_tuple()
 **/
int
    mpl_unpack_param_value_struint8_tuple(const char* value_str,
                                          void **value_pp,
                                          const mpl_param_descr2_t *descr_p,
                                          const mpl_pack_options_t *options_p,
                                          mpl_param_element_id_t unpack_context)
{
    mpl_struint8_tuple_t *t_p;
    char* p;
    int slen;
    int temp;
    char *temp_str;
    int temp_string_allocated = 0;
    const uint8_t *max_p = descr_p->max_p;
    const uint8_t *min_p = descr_p->min_p;
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    if (strchr(value_str, '\\')) {
        temp_str = remove_escape(value_str, options_p->message_delimiter, '\\');
        if (temp_str == NULL) {
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                                ("Failed allocating memory\n"));
            set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
            return (-1);
        }
        temp_string_allocated = 1;
    }
    else {
        temp_str = (char*) value_str;
    }

    p = strchr(temp_str, '/');
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack struint8_tuple failed, no delimiter: %s\n",
                             temp_str));
        set_errno(E_MPL_FAILED_OPERATION);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    t_p = malloc(sizeof(mpl_struint8_tuple_t));
    if (NULL == t_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    slen = p - temp_str;

    t_p->key_p = malloc(slen + 1);
    if (NULL == t_p->key_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(t_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }
    strncpy(t_p->key_p, temp_str, slen);
    t_p->key_p[slen] = 0;


    p++;

    if (convert_int(p, &temp) < 0)
    {
        free(t_p->key_p);
        free(t_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    if ((temp < 0) ||
        (temp > (int)UINT8_MAX))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack struint8_tuple failed on range check: %d\n", temp));
        set_errno(E_MPL_FAILED_OPERATION);
        free(t_p->key_p);
        free(t_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    t_p->value = (uint8_t)temp;

    if ((max_p != NULL) &&
        (t_p->value > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack struint8_tuple value failed on "
                             "max check: %d > %d\n", t_p->value, *max_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(t_p->key_p);
        free(t_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    if ((min_p != NULL) &&
        (t_p->value < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack struint8_tuple value failed on "
                             "min check: %d < %d\n", t_p->value, *min_p));
        set_errno(E_MPL_FAILED_OPERATION);
        free(t_p->key_p);
        free(t_p);
        if (temp_string_allocated)
            free(temp_str);
        return (-1);
    }

    *value_pp = t_p;

    if (temp_string_allocated)
        free(temp_str);
    return (0);
}

/**
 * mpl_clone_param_value_struint8_tuple()
 **/
int
    mpl_clone_param_value_struint8_tuple(void **new_value_pp,
                                         const void* old_value_p,
                                         const mpl_param_descr2_t *descr_p)
{
    mpl_struint8_tuple_t *t_p;
    const mpl_struint8_tuple_t *old_t_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    t_p = malloc(sizeof(mpl_struint8_tuple_t));
    if (NULL == t_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    old_t_p = old_value_p;

    t_p->key_p = malloc(strlen(old_t_p->key_p) + 1);
    if (NULL == t_p->key_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        free(t_p);
        return (-1);
    }

    strcpy(t_p->key_p, old_t_p->key_p);
    t_p->value = old_t_p->value;

    *new_value_pp = t_p;

    return (0);
}

/**
 * mpl_copy_param_value_struint8_tuple()
 **/
/* NOTE: Copy on a tuple means copy of the value part of the tuple */
int
    mpl_copy_param_value_struint8_tuple(void *to_value_p,
                                        const void* from_value_p,
                                        int size,
                                        const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (size >= (int)sizeof(int))
        *(uint8_t*)to_value_p = ((mpl_struint8_tuple_t*)from_value_p)->value;

    return sizeof(uint8_t);
}

/**
 * mpl_compare_param_value_struint8_tuple()
 **/
int mpl_compare_param_value_struint8_tuple(const void *value1_p,
                                           const void* value2_p,
                                           const mpl_param_descr2_t *descr_p)
{
    const mpl_struint8_tuple_t *t1_p;
    const mpl_struint8_tuple_t *t2_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    t1_p = value1_p;
    t2_p = value2_p;

    if ((NULL == t1_p->key_p) || (NULL == t2_p->key_p))
    {
        if (t1_p->key_p != t2_p->key_p)
            return -1;
    }
    else if (strcmp(t1_p->key_p, t2_p->key_p))
        return -1;

    return (t1_p->value != t2_p->value);
}

/**
 * mpl_sizeof_param_value_struint8_tuple()
 **/
size_t mpl_sizeof_param_value_struint8_tuple(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return 0;
}


/**
 * mpl_free_param_value_struint8_tuple()
 **/
void mpl_free_param_value_struint8_tuple(void *value_p)
{
    mpl_struint8_tuple_t *t_p;

    t_p = value_p;
    if (NULL != t_p)
    {
        if (NULL != t_p->key_p)
        {
            free(t_p->key_p);
        }

        free(t_p);
    }
}




/**
 * mpl_pack_param_value_bag()
 **/
int mpl_pack_param_value_bag(const void* param_value_p,
                             char *buf,
                             size_t buflen,
                             const mpl_param_descr2_t *descr_p,
                             const  mpl_pack_options_t *options_p)
{
    mpl_list_t* l_p = (mpl_list_t*)param_value_p;
    int tmplen;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    assert(NULL != param_value_p);

    tmplen = mpl_param_list_pack_extended(l_p,
                                          NULL,
                                          0,
                                          options_p);
    if (tmplen < 0)
        return tmplen;

    if ((tmplen+3) > (int)buflen)
        return (tmplen+3);


    if (mpl_param_list_pack_extended(l_p,
                                     buf+2,
                                     tmplen+1,
                                     options_p) != (int)tmplen)
        return -1;

    buf[0] = '=';
    buf[1] = '{';
    buf[tmplen+2] = '}';
    buf[tmplen+3] = '\0';
    return (tmplen+3);
}


/**
 * mpl_unpack_param_value_bag()
 **/
int
    mpl_unpack_param_value_bag(const char* value_str,
                               void **value_pp,
                               const mpl_param_descr2_t *descr_p,
                               const mpl_pack_options_t *options_p,
                               mpl_param_element_id_t unpack_context)
{
    mpl_list_t *l_p;
    char* start_p = NULL;
    char* end_p = NULL;
    char* copy_p;
    const int *max_p = descr_p->max_p;
    const int *min_p = descr_p->min_p;
    bool err = false;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    start_p = strchr((char*)value_str, '{');
    if (start_p != NULL)
        end_p = get_matching_close_bracket('{', '}', start_p, '\\');

    if ((start_p == NULL) || (end_p == NULL))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack list failed, no delimiter: %s\n",
                             value_str));
        set_errno(E_MPL_FAILED_OPERATION);
        return (-1);
    }

    start_p++;
    if ((start_p > end_p) || (*start_p == '\0'))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack list failed, format error: %s\n",
                             value_str));
        set_errno(E_MPL_FAILED_OPERATION);
        return (-1);
    }

    copy_p = calloc(1, (end_p - start_p) + 1);
    if (copy_p == NULL)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("copy_p\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    strncpy(copy_p, start_p, (end_p - start_p));

    l_p = mpl_param_list_unpack_internal(copy_p,
                                         options_p,
                                         unpack_context,
                                         &err);

    if (err) {
        free(copy_p);
        return -1;
    }

    if ((max_p != NULL) &&
        ((int)mpl_list_len(l_p) > *max_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack list value failed on "
                             "max check: %zu > %d\n", mpl_list_len(l_p),
                             *max_p));/*lint !e557 %zu is C99 */
        set_errno(E_MPL_FAILED_OPERATION);
        mpl_param_list_destroy(&l_p);
        free(copy_p);
        return (-1);
    }

    if ((min_p != NULL) &&
        ((int)mpl_list_len(l_p) < *min_p))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Unpack list value failed on "
                             "min check: %zu < %d\n", mpl_list_len(l_p),
                             *min_p));/*lint !e557 %zu is C99 */
        set_errno(E_MPL_FAILED_OPERATION);
        mpl_param_list_destroy(&l_p);
        free(copy_p);
        return (-1);
    }

    free(copy_p);
    *value_pp = l_p;

    return (0);
}

/**
 * mpl_clone_param_value_bag()
 **/
int
    mpl_clone_param_value_bag(void **new_value_pp,
                              const void* old_value_p,
                              const mpl_param_descr2_t *descr_p)
{
    mpl_list_t *l_p;
    mpl_list_t *ol_p = (mpl_list_t *)old_value_p;
    MPL_IDENTIFIER_NOT_USED(descr_p);

    l_p = mpl_param_list_clone(ol_p);
    if (l_p == NULL)
        return (-1);

    *new_value_pp = l_p;
    return 0;
}


/**
 * mpl_copy_param_value_bag()
 **/
/* NOTE: Copy on a bag means copy of the list pointer
   (rather pointless unless cloning a list from an element) */
int
    mpl_copy_param_value_bag(void *to_value_p,
                             const void* from_value_p,
                             int size,
                             const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (size >= (int)sizeof(mpl_list_t*))
        *(mpl_list_t**)to_value_p = (mpl_list_t*)from_value_p;

    return sizeof(mpl_list_t*);
}

/**
 * mpl_compare_param_value_bag()
 **/
int mpl_compare_param_value_bag(const void *value1_p,
                                const void* value2_p,
                                const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return mpl_compare_param_lists((mpl_list_t *)value1_p,
                                   (mpl_list_t *)value2_p);
}

/**
 * mpl_sizeof_param_value_bag()
 **/
size_t mpl_sizeof_param_value_bag(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return 0;
}


/**
 * mpl_free_param_value_bag()
 **/
void mpl_free_param_value_bag(void *value_p)
{
    mpl_list_t *l_p = value_p;
    mpl_param_list_destroy(&l_p);
}



/**
 * mpl_pack_param_value_addr()
 **/
int mpl_pack_param_value_addr(const void* param_value_p,
                              char *buf, size_t buflen,
                              const mpl_param_descr2_t *descr_p,
                              const mpl_pack_options_t *options_p)
{
    assert(NULL != param_value_p);
    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p)
        return snprintf(buf, buflen, "=%p", *(void**)param_value_p);
}

/**
 * mpl_unpack_param_value_addr()
 **/
int
    mpl_unpack_param_value_addr(const char* value_str, void **value_pp,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p,
                                mpl_param_element_id_t unpack_context)
{
    void** p;
    int res;

    MPL_IDENTIFIER_NOT_USED(descr_p);
    MPL_IDENTIFIER_NOT_USED(options_p);
    MPL_IDENTIFIER_NOT_USED(unpack_context);

    p = malloc(sizeof(void*));
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    res = sscanf(value_str, "%p", p);

    if (res <= 0)
    {
        free(p);
        return (-1);
    }

    assert(NULL != value_pp);
    *value_pp = p;

    return (0);
}

/**
 * mpl_clone_param_value_addr()
 **/
int
    mpl_clone_param_value_addr(void **new_value_pp,
                               const void* old_value_p,
                               const mpl_param_descr2_t *descr_p)
{
    void** p = malloc(sizeof(void*));
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (NULL == p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return (-1);
    }

    *p = *(void**)old_value_p;
    *new_value_pp = p;

    return (0);
}

/**
 * mpl_copy_param_value_addr()
 **/
int
    mpl_copy_param_value_addr(void *to_value_p,
                              const void* from_value_p,
                              int size,
                              const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    if (size >= (int)sizeof(void*))
        *(void**)to_value_p = *(void**)from_value_p;

    return sizeof(void*);
}

/**
 * mpl_compare_param_value_addr()
 **/
int mpl_compare_param_value_addr(const void *value1_p,
                                 const void* value2_p,
                                 const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return (*((void**) value1_p) != *((void**) value2_p));
}

/**
 * mpl_sizeof_param_value_addr()
 **/
size_t mpl_sizeof_param_value_addr(const mpl_param_descr2_t *descr_p)
{
    MPL_IDENTIFIER_NOT_USED(descr_p);
    return sizeof(void*);
}







/**
 * mpl_get_args()
 **/
int mpl_get_args(mpl_arg_t *args,
                 int args_len,
                 char *buf,
                 char equal,
                 char delimiter,
                 char escape)
{
    char *p;
    char *kp;
    char *vp;
    int i = 0;
    int buflen;

    assert(NULL != args);

    p = mpl_trimstring(buf, escape);

    buflen = strlen(p);

    while (((p - buf) < buflen) && (i < args_len))
    {
        char *mid;
        char *end;
        char end_copy;

        end = strchr_escape( p, delimiter, escape);

        if (NULL == end)
        {
            end = p + strlen(p);
            end_copy = *end;
        }
        else
        {
            end_copy = *end;
            *end = '\0';
        }

        mid = strchr_escape( p, equal, escape );
        if (NULL != mid)
        {
            *mid = '\0';
            kp = mpl_trimstring(p, escape);
            args[i].key_p = kp;
            vp = mpl_trimstring(mid + 1, escape);
            if (*vp == '{')
            {
                *end = end_copy;
                end = get_matching_close_bracket('{', '}', vp, escape);
                if (NULL == end)
                    end = vp + strlen(vp);
                else
                    end++;
                *end = '\0';
            }
            args[i].value_p = vp;
        }
        else
        {
            args[i].key_p = mpl_trimstring(p, escape);
            args[i].value_p = NULL;
        }

        while ((((end+1) - buf) < buflen) &&
               *(end+1) == delimiter)
            end++;

        p = end + 1;
        i++;
    }

    if ((p - buf) < buflen)
        return -1;
    else
        return i;
}


/**
 * mpl_add_param_to_list_n_tag - add parameter to param list
 *                               with value length check
 *
 */
int mpl_add_param_to_list_n_tag(mpl_list_t **param_list_pp,
                                mpl_param_element_id_t param_id,
                                int tag,
                                const void *value_p,
                                size_t len)
{
    mpl_param_element_t* param_elem_p;

    /* Add msgtype to the parameter list */
    param_elem_p = mpl_param_element_create_n_tag(param_id, tag, value_p, len);
    if (NULL == param_elem_p)
    {
        return -1;
    }

    mpl_list_add(param_list_pp, &param_elem_p->list_entry);
    return 0;
}

/**
 * mpl_add_param_to_list_tag - add parameter to param list
 *
 */
int mpl_add_param_to_list_tag(mpl_list_t **param_list_pp,
                              mpl_param_element_id_t param_id,
                              int tag,
                              const void *value_p)
{
    mpl_param_element_t* param_elem_p;

    /* Add msgtype to the parameter list */
    param_elem_p = mpl_param_element_create_tag(param_id, tag, value_p);
    if (NULL == param_elem_p)
    {
        return -1;
    }

    mpl_list_add(param_list_pp, &param_elem_p->list_entry);
    return 0;
}

/**
 * mpl_param_list_add_int_tag - add integer parameter to param list
 *
 */
int mpl_param_list_add_int_tag(mpl_list_t **param_list_pp,
                               mpl_param_element_id_t param_id,
                               int tag,
                               int value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_uint8_tag - add bool8 parameter to param list
 *
 */
int mpl_param_list_add_uint8_tag(mpl_list_t **param_list_pp,
                                 mpl_param_element_id_t param_id,
                                 int tag,
                                 uint8_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_sint8_tag - add sint8 parameter to param list
 *
 */
int mpl_param_list_add_sint8_tag(mpl_list_t **param_list_pp,
                                 mpl_param_element_id_t param_id,
                                 int tag,
                                 sint8_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_uint16_tag - add bool8 parameter to param list
 *
 */
int mpl_param_list_add_uint16_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  uint16_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_sint16_tag - add sint16 parameter to param list
 *
 */
int mpl_param_list_add_sint16_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  sint16_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_uint32_tag - add uint32 parameter to param list
 *
 */
int mpl_param_list_add_uint32_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  uint32_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_uint64_tag - add uint64 parameter to param list
 *
 */
int mpl_param_list_add_uint64_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  uint64_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_sint32_tag - add sint32 parameter to param list
 *
 */
int mpl_param_list_add_sint32_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  sint32_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_sint64_tag - add sint64 parameter to param list
 *
 */
int mpl_param_list_add_sint64_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  int64_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_bool8_tag - add bool8 parameter to param list
 *
 */
int mpl_param_list_add_bool8_tag(mpl_list_t **param_list_pp,
                                 mpl_param_element_id_t param_id,
                                 int tag,
                                 uint8_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_bool_tag - add bool8 parameter to param list
 *
 */
int mpl_param_list_add_bool_tag(mpl_list_t **param_list_pp,
                                mpl_param_element_id_t param_id,
                                int tag,
                                bool value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_enum_tag - add enum parameter to param list
 *
 */
int mpl_param_list_add_enum_tag(mpl_list_t **param_list_pp,
                                mpl_param_element_id_t param_id,
                                int tag,
                                int64_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       mpl_param_id_sizeof_param_value(param_id));
}

/**
 * mpl_param_list_add_enum8_tag - add enum parameter to param list
 *
 */
int mpl_param_list_add_enum8_tag(mpl_list_t **param_list_pp,
                                 mpl_param_element_id_t param_id,
                                 int tag,
                                 uint8_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_enum16_tag - add enum parameter to param list
 *
 */
int mpl_param_list_add_enum16_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  uint16_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_enum32_tag - add enum parameter to param list
 *
 */
int mpl_param_list_add_enum32_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  uint32_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_signed_enum8_tag - add enum parameter to param list
 *
 */
int mpl_param_list_add_signed_enum8_tag(mpl_list_t **param_list_pp,
                                        mpl_param_element_id_t param_id,
                                        int tag,
                                        sint8_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_signed_enum16_tag - add enum parameter to param list
 *
 */
int mpl_param_list_add_signed_enum16_tag(mpl_list_t **param_list_pp,
                                         mpl_param_element_id_t param_id,
                                         int tag,
                                         sint16_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_param_list_add_signed_enum32_tag - add enum parameter to param list
 *
 */
int mpl_param_list_add_signed_enum32_tag(mpl_list_t **param_list_pp,
                                         mpl_param_element_id_t param_id,
                                         int tag,
                                         sint32_t value)
{
    return mpl_add_param_to_list_n_tag(param_list_pp,
                                       param_id,
                                       tag,
                                       &value,
                                       sizeof(value));
}

/**
 * mpl_remove_from_blacklist - remove parameter id from blacklist
 *
 */
int mpl_remove_from_blacklist(mpl_blacklist_t *blacklist_p,
                              mpl_param_element_id_t param_id)
{
    mpl_param_element_t *elem_p;

    elem_p = mpl_param_list_find(param_id, *blacklist_p);

    if (elem_p == NULL)
        return -1;

    (void) mpl_list_remove(blacklist_p, &elem_p->list_entry);

    mpl_param_element_destroy(elem_p);

    return 0;
}

/**
 * mpl_trimstring()
 **/
char *mpl_trimstring(char *s, char escape)
{
    char *p = s;
    char *e;

    // Trim start
    while (strlen(s) && isspace(s[0]))
        s++;

    // Trim end, taking escape into account
    p = s + (strlen(s) - 1);

    while ((p > s) && isspace(*p)) {
        // did we find an escaped character?
        e = p;
        while (e && (e > s) && (*(e-1) == escape))
            e--;

        // An odd number of escapes in a row? (then it is escaped)
        if (1 == ((p - e) % 2))
            break;

        *p = '\0';
        p--;
    }

    return s;
}

/**
 * mpl_get_errno
 *
 */
int mpl_get_errno()
{
    mpl_pc_t* pc_p = get_pc();
    if (pc_p)
    {
        return pc_p->error;
    }
    else
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Could not find process context\n"));
        return -1;
    }
}


/**
 * mpl_compare_param_lists
 *
 */
int mpl_compare_param_lists(mpl_list_t *list1_p, mpl_list_t *list2_p)
{
    mpl_list_t *list_p;
    mpl_param_element_t* param_elem1_p;
    mpl_param_element_t* param_elem2_p;

    if ((NULL == list1_p) && (NULL != list2_p)) {
        printf("list 1 has no params, but list 2 has\n");
        return -1;
    }

    if ((NULL == list2_p) && (NULL != list1_p)) {
        printf("list 2 has no params, but list 1 has\n");
        return -1;
    }

    MPL_LIST_FOR_EACH(list1_p, list_p)
    {
        param_elem1_p = MPL_LIST_CONTAINER(list_p, mpl_param_element_t, list_entry);

        if (MPL_PARAM_ELEMENT_IS_FIELD(param_elem1_p)) {
            if (!MPL_FIELD_PRESENT_IN_LIST_TAG(param_elem1_p->context,
                                               param_elem1_p->id_in_context,
                                               param_elem1_p->tag,
                                               list2_p)) {
                printf("param field %s present in list 1 but missing in list 2\n",
                       mpl_param_get_bag_field_name(param_elem1_p->context,
                                                    param_elem1_p->id_in_context));
                return -1;
            }
            param_elem2_p = mpl_param_list_find_field_tag(param_elem1_p->context,
                                                          param_elem1_p->id_in_context,
                                                          param_elem1_p->tag,
                                                          list2_p);
        }
        else {
            if (!MPL_PARAM_PRESENT_IN_LIST(param_elem1_p->id, list2_p)) {
                printf("param %d present in list 1 but missing in list 2\n", param_elem1_p->id);
                return -1;
            }
            param_elem2_p = mpl_param_list_find_tag(param_elem1_p->id, param_elem1_p->tag, list2_p);
        }

        if (0 != mpl_param_element_compare(param_elem1_p, param_elem2_p))
        {
            printf("1: param %s[%d] (%x) <-> %s[%d] (%x) value mismatch: %s <-> %s\n",
                   mpl_param_id_get_string(param_elem1_p->id), param_elem1_p->id,
                   param_elem1_p->tag,
                   mpl_param_id_get_string(param_elem2_p->id), param_elem2_p->id,
                   param_elem2_p->tag,
                   mpl_param_value_get_string(param_elem1_p->id, param_elem1_p->value_p),
                   mpl_param_value_get_string(param_elem2_p->id, param_elem2_p->value_p));
            return -1;
        }
    }

    MPL_LIST_FOR_EACH(list2_p, list_p)
    {
        param_elem2_p = MPL_LIST_CONTAINER(list_p, mpl_param_element_t, list_entry);

        if (MPL_PARAM_ELEMENT_IS_FIELD(param_elem2_p)) {
            if (!MPL_FIELD_PRESENT_IN_LIST_TAG(param_elem2_p->context,
                                               param_elem2_p->id_in_context,
                                               param_elem2_p->tag,
                                               list1_p)) {
                printf("param field %s present in list 2 but missing in list 1\n",
                       mpl_param_get_bag_field_name(param_elem2_p->context,
                                                    param_elem2_p->id_in_context));
                return -1;
            }
            param_elem1_p = mpl_param_list_find_field_tag(param_elem2_p->context,
                                                          param_elem2_p->id_in_context,
                                                          param_elem2_p->tag,
                                                          list1_p);
        }
        else {
            if (!MPL_PARAM_PRESENT_IN_LIST(param_elem2_p->id, list1_p)) {
                printf("param %d present in list 2 but missing in message 1\n", param_elem2_p->id);
                return -1;
            }
            param_elem1_p = mpl_param_list_find_tag(param_elem2_p->id, param_elem2_p->tag, list1_p);
        }

        if (0 != mpl_param_element_compare(param_elem2_p, param_elem1_p))
        {
            printf("2: param %s[%d] (%x) <-> %s[%d] (%x) value mismatch: %s <-> %s\n",
                   mpl_param_id_get_string(param_elem2_p->id), param_elem2_p->id,
                   param_elem2_p->tag,
                   mpl_param_id_get_string(param_elem1_p->id), param_elem1_p->id,
                   param_elem1_p->tag,
                   mpl_param_value_get_string(param_elem2_p->id, param_elem2_p->value_p),
                   mpl_param_value_get_string(param_elem1_p->id, param_elem1_p->value_p));
            return -1;
        }
    }
    return 0;
}

int mpl_convert_int(const char* value_str, int *value_p)
{
    return convert_int(value_str, value_p);
}


/**
 * mpl_set_errno
 *
 */
void mpl_set_errno(int error_value)
{
    set_errno(error_value);
}

/****************************************************************************
 *
 * Private Functions
 *
 ****************************************************************************/

static void *allocate_and_copy_max(mpl_type_t mpl_type, uint64_t max)
{
    void *ret_p = NULL;

    switch (mpl_type) {
        case mpl_type_int:
        case mpl_type_string:
        case mpl_type_wstring:
        case mpl_type_uint8_array:
        case mpl_type_uint16_array:
        case mpl_type_uint32_array:
        case mpl_type_string_tuple:
        case mpl_type_int_tuple:
        case mpl_type_strint_tuple:
        case mpl_type_bag:
            ret_p = malloc(sizeof(int));
            if (ret_p == NULL)
                break;
            *((int*)ret_p) = (int) max;
            break;
        case mpl_type_sint8:
            ret_p = malloc(sizeof(sint8_t));
            if (ret_p == NULL)
                break;
            *((sint8_t*)ret_p) = (sint8_t) max;
            break;
        case mpl_type_sint16:
            ret_p = malloc(sizeof(sint16_t));
            if (ret_p == NULL)
                break;
            *((sint16_t*)ret_p) = (sint16_t) max;
            break;
        case mpl_type_sint32:
            ret_p = malloc(sizeof(sint32_t));
            if (ret_p == NULL)
                break;
            *((sint32_t*)ret_p) = (sint32_t) max;
            break;
        case mpl_type_sint64:
            ret_p = malloc(sizeof(int64_t));
            if (ret_p == NULL)
                break;
            *((int64_t*)ret_p) = (int64_t) max;
            break;
        case mpl_type_uint8:
        case mpl_type_struint8_tuple:
            ret_p = malloc(sizeof(uint8_t));
            if (ret_p == NULL)
                break;
            *((uint8_t*)ret_p) = (uint8_t) max;
            break;
        case mpl_type_uint16:
            ret_p = malloc(sizeof(uint16_t));
            if (ret_p == NULL)
                break;
            *((uint16_t*)ret_p) = (uint16_t) max;
            break;
        case mpl_type_uint32:
            ret_p = malloc(sizeof(uint32_t));
            if (ret_p == NULL)
                break;
            *((uint32_t*)ret_p) = (uint32_t) max;
            break;
        case mpl_type_uint64:
            ret_p = malloc(sizeof(uint64_t));
            if (ret_p == NULL)
                break;
            *((uint64_t*)ret_p) = max;
            break;
        default:
            assert(0);
    }

    if (ret_p == NULL)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
    }
    return ret_p;
}


static int upgrade_param_descr_set(mpl_param_descr_set_t *param_descr_p)
{
    int size;
    int i;
    int upgrade = 0;
    mpl_param_descr_t *pd_p;
    mpl_param_descr2_t *pd2_p;

    size = PARAM_SET_SIZE(param_descr_p);
    if (MPL_PARAMID_TO_PARAMSET(param_descr_p->paramid_enum_size) == param_descr_p->param_set_id) {
        /* the enum size indicates that this is an old
           parameter set and must be re-calculated */
        int base = MPL_PARAM_SET_ID_TO_PARAMID_BASE(param_descr_p->param_set_id);
        size = param_descr_p->paramid_enum_size - 1 - base;
        param_descr_p->paramid_enum_size = size + 1;
    }

    for (i = 0; i < size; i++) {
        pd_p = (mpl_param_descr_t *) &param_descr_p->array[i];
        if ((pd_p->max_p != NULL) || (pd_p->stringarr != NULL)) {
            upgrade = 1;
            break;
        }
    }

    if (!upgrade)
        return 0;

    param_descr_p->array2 = calloc(size, sizeof(mpl_param_descr2_t));
    if (param_descr_p->array2 == NULL) {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        return -1;
    }
    param_descr_p->is_dynamic_array2 = true;
    for (i = 0; i < size; i++) {
        int j;
        /* Only positive values in old enums, and no holes */
        enum {
            dummy_small_min = 0,
            dummy_small_max = 127
        } dummy_small_enum;

        enum {
            dummy_medium_min = 0,
            dummy_medium_max = 255
        } dummy_medium_enum;

        enum {
            dummy_big_min = 0,
            dummy_big_max = INT_MAX
        } dummy_big_enum;

        pd_p = (mpl_param_descr_t *) &param_descr_p->array[i];
        pd2_p = (mpl_param_descr2_t *) &param_descr_p->array2[i];
        if (pd_p->max_p != NULL) {
            pd2_p = (mpl_param_descr2_t *) &param_descr_p->array2[i];
            pd2_p->max_p = allocate_and_copy_max(pd_p->type, *pd_p->max_p);
        }

        if (pd_p->stringarr != NULL) {
            mpl_enum_value_t *enum_values;
            pd2_p->enum_values = calloc(pd_p->stringarr_size, sizeof(mpl_enum_value_t));
            if (pd2_p->enum_values == NULL) {
                MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                                    ("Failed allocating memory\n"));
                return -1;
            }
            enum_values = (mpl_enum_value_t *) pd2_p->enum_values;
            for (j = 0; j < pd_p->stringarr_size; j++) {
                enum_values[j].name_p = pd_p->stringarr[j];
                enum_values[j].value = j;
            }
            pd2_p->enum_values_size = pd_p->stringarr_size;
            if (j < 128)
                pd2_p->enum_representation_bytesize =
                    sizeof(dummy_small_enum);
            else if (j < 256)
                pd2_p->enum_representation_bytesize =
                    sizeof(dummy_medium_enum);
            else
                pd2_p->enum_representation_bytesize =
                    sizeof(dummy_big_enum);
            pd2_p->enum_representation_signed = false;
        }
    } /*lint !e550 */

    return 1;
}


/**
 * get_pc
 *
 */
static mpl_pc_t* get_pc(void)
{
    mpl_pc_t* pc_p;
    mpl_thread_t pid;
    mpl_list_t *elem_p;
    int i;

    pid = mpl_get_current_thread_id();

    (void)mpl_mutex_lock(mutex);
    MPL_LIST_FOR_EACH(mpl_pc_list_p, elem_p)
    {
        pc_p = MPL_LIST_CONTAINER(elem_p, mpl_pc_t, list_entry);
        if (pc_p->pid == pid)
        {
            (void)mpl_mutex_unlock(mutex);
            goto found;
        }
    }
    (void)mpl_mutex_unlock(mutex);

    pc_p = malloc(sizeof(mpl_pc_t));
    if (NULL == pc_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        return NULL;
    }
    memset(pc_p, 0, sizeof(mpl_pc_t));
    for (i = 0; i < num_scratch_strings; i++) {
        pc_p->scratch_string[i] = malloc(initial_scratch_string_len);
        if (pc_p->scratch_string[i] == NULL)
        {
            i--;
            MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                                ("Failed allocating memory\n"));
            /* free previously allocated scratch strings */
            for(;i>=0;i--)
                free(pc_p->scratch_string[i]);
            free(pc_p);
            return NULL;
        }
        pc_p->scratch_string_len[i] = initial_scratch_string_len;
    }
    pc_p->pid = pid;

    (void)mpl_mutex_lock(mutex);
    mpl_list_add(&mpl_pc_list_p, &pc_p->list_entry);
    (void)mpl_mutex_unlock(mutex);

found:
    return pc_p;
}

/**
 * set_errno
 *
 */
static void set_errno(int error_value)
{
    mpl_pc_t* pc_p = get_pc();
    if (pc_p)
    {
        pc_p->error = error_value;
    }
    else
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Could not find process context\n"));
    }
}

/**
 * get_scratch_string
 *
 */
static char *get_scratch_string(int len)
{
    mpl_pc_t* pc_p = get_pc();
    if (pc_p)
    {
        if (pc_p->scratch_string_current >= num_scratch_strings)
            pc_p->scratch_string_current = 0;
        if (len > pc_p->scratch_string_len[pc_p->scratch_string_current])
        {
            pc_p->scratch_string[pc_p->scratch_string_current] =
                realloc(pc_p->scratch_string[pc_p->scratch_string_current], len);
            if (pc_p->scratch_string[pc_p->scratch_string_current] == NULL)
            {
                MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                                    ("Failed allocating memory\n"));
                pc_p->scratch_string_len[pc_p->scratch_string_current] = 0;
                return NULL;
            }

            pc_p->scratch_string_len[pc_p->scratch_string_current] = len;
        }

        return pc_p->scratch_string[pc_p->scratch_string_current++];
    }

    MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                        ("Could not find process context\n"));
    return NULL;
}

/**
 * paramset_find
 *
 */
static mpl_param_descr_set_t* paramset_find(int param_set_id,
                                            char *paramid_prefix)
{
    mpl_list_t *elem_p;
    mpl_param_descr_set_t *paramset_p = NULL;
    bool id_match = false;
    bool prefix_match = false;
    mpl_pc_t* pc_p = get_pc();

    if (NULL == pc_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_OPERATION,
                            ("Could not find process context\n"));
        return NULL;
    }

    (void)mpl_mutex_lock(mutex);
    MPL_LIST_FOR_EACH(paramset_list_p, elem_p)
    {
        paramset_p = MPL_LIST_CONTAINER(elem_p,
                                        mpl_paramset_container_t,
                                        list_entry)->paramset_p;

        if ((param_set_id > 0) && (paramset_p->param_set_id == param_set_id))
        {
            id_match = true;
            if (NULL == paramid_prefix)
            {
                (void)mpl_mutex_unlock(mutex);
                return paramset_p;
            }
        }

        if ((NULL != paramid_prefix) &&
            !strncmp(paramset_p->paramid_prefix,
                     paramid_prefix,
                     MPL_PARAMID_PREFIX_MAXLEN))
        {
            prefix_match = true;
            if (param_set_id <= 0)
            {
                (void)mpl_mutex_unlock(mutex);
                return paramset_p;
            }
        }

        if (id_match && prefix_match)
        {
            (void)mpl_mutex_unlock(mutex);
            return paramset_p;
        }

        id_match = false;
        prefix_match = false;
    }
    (void)mpl_mutex_unlock(mutex);

    return NULL;
}

/**
 * paramset_add
 *
 */
static int paramset_add(mpl_param_descr_set_t* paramset_p)
{
    /*lint -esym(429, paramset_container_p) freed elswhere */
    mpl_paramset_container_t *paramset_container_p;

    if (NULL == paramset_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("paramset_p is NULL\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return -1;
    }

    paramset_container_p = malloc(sizeof(mpl_paramset_container_t));
    if (NULL == paramset_container_p)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return -1;
    }

    memset(paramset_container_p, 0, sizeof(mpl_paramset_container_t));

    paramset_container_p->paramset_p = paramset_p;

    (void)mpl_mutex_lock(mutex);
    mpl_list_add(&paramset_list_p, &paramset_container_p->list_entry);
    (void)mpl_mutex_unlock(mutex);

    return 0;
}

/**
 * strchr_escape()
 **/
static char *strchr_escape(char *s, char c, char escape)
{
    char *p = s;
    char *e;

    while (p && strlen(p)) {
        p = strchr(p, c);
        // did we find an escaped character?
        if (p==NULL)
            continue;
        e = p;
        while (e && (e > s) && (*(e-1) == escape))
            e--;
        if (1 == ((p - e) % 2)) {
            // An odd number of escapes in a row: yes
            p++;
            continue;
        }
        else
            break;
    }
    return p;
}


/**
 * snprintf_escape()
 **/
static int snprintf_escape(char delimiter, char escape,
                           char *str, size_t size, const char *format, ...)
{
    int n;
    int size_needed;
    char *tmpstr;
    int num_escapes_needed;
    char *p;
    va_list ap;

    va_start(ap, format);
    n = vsnprintf(NULL, 0, format, ap);
    if (n < 0)
        return n;
    va_end(ap);

    tmpstr = malloc(n + 1);
    if (tmpstr == NULL)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return -1;
    }

    va_start(ap, format);
    n = vsnprintf(tmpstr, n + 1, format, ap);
    assert(n > 0);
    va_end(ap);

    p = tmpstr;
    num_escapes_needed = 0;
    while (p && strlen(p)) {
        p = strchr(p, delimiter);
        if (!p)
            break;
        num_escapes_needed++;
        p++;
    }
    size_needed = n + num_escapes_needed;

    free(tmpstr);

    if (size > 0) {
        va_start(ap, format);
        n = vsnprintf(str, size - num_escapes_needed, format, ap);
        assert(n > 0);
        va_end(ap);
        if (num_escapes_needed)
            n = fill_escape(str, size, delimiter, escape);
        return n;
    }
    else {
        return size_needed;
    }
}

static int fill_escape(char *str, int size, char delimiter, char escape)
{
    char *s;
    char *d;
    char *src = strdup(str);

    if (src == NULL) {
        MPL_DBG_TRACE_ERROR(E_MPL_FAILED_ALLOCATING_MEMORY,
                            ("Failed allocating memory\n"));
        set_errno(E_MPL_FAILED_ALLOCATING_MEMORY);
        return 1;
    }

    s = src;
    d = str;
    while (*s) {
        if (*s == delimiter) {
            *d = escape;
            d++;
        }
        *d = *s;
        d++;
        s++;
        if ((d - str) > size)
            break;
    }
    *d = '\0';
    free(src);
    return (d - str);
}

static char *remove_escape(const char *src, char delimiter, char escape)
{
    char *s;
    char *d;
    char *dst = strdup(src);

    if (dst == NULL) {
        return NULL;
    }

    s = (char*) src;
    d = dst;
    while (*s) {
        if (*s == escape) {
            s++;
        }
        *d = *s;
        d++;
        s++;
    }
    *d = '\0';
    return (dst);
}

static char *get_matching_close_bracket(char open_bracket, char close_bracket, char *str_p, char escape)
{
    int num_open = 0;
    char *p;

    p = str_p;

    if (*p != open_bracket)
        return NULL;

    while (*p) {
        if (*p == escape) {
            p++;
            continue;
        }

        if (*p == open_bracket)
            num_open++;
        if (*p == close_bracket)
            num_open--;
        if (num_open == 0)
            return p;
        p++;
    }

    return NULL;
}

static char *get_bracket_contents(const char *str,
                                  char open_bracket,
                                  char close_bracket,
                                  size_t *len_p)
{
    char *e;
    char *s = strchr_escape((char*)str, open_bracket, '\\');
    if (s == NULL)
        return NULL;
    e = get_matching_close_bracket(open_bracket,
                                   close_bracket,
                                   s,
                                   '\\');
    if (e == NULL)
        return NULL;
    s++;
    assert(len_p);
    *len_p = e - s;
    return s;
}


/**
 * convert_int()
 **/
static int
    convert_int(const char* value_str, int *value_p)
{
    char* endp;

    assert(NULL != value_p);
    assert(NULL != value_str);

    /* Empty strings are not allowed */
    if (0 == strlen(value_str))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("empty string is not an acceptable integer value\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return -1;
    }

    *value_p = strtol( value_str, &endp, 0 );

    /* The whole string should be a number */
    if (*endp != '\0')
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Not an acceptable integer value: %s\n", value_str));
        set_errno(E_MPL_INVALID_PARAMETER);
        return (-1);
    }

    return (0);
}

/**
 * convert_int()
 **/
static int
    convert_int64(const char* value_str, int64_t *value_p)
{
    char* endp;

    assert(NULL != value_p);
    assert(NULL != value_str);

    /* Empty strings are not allowed */
    if (0 == strlen(value_str))
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("empty string is not an acceptable integer value\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return -1;
    }

    *value_p = strtoll( value_str, &endp, 0 );

    /* The whole string should be a number */
    if (*endp != '\0')
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Not an acceptable integer value: %s\n", value_str));
        set_errno(E_MPL_INVALID_PARAMETER);
        return (-1);
    }

    return (0);
}

/**
 * convert_stringarr_to_int()
 **/
static int
    convert_stringarr_to_int(const char* value_str,
                             int *value_p,
                             const char* stringarr[],
                             int stringarr_size)
{
    int index;

    assert(NULL != value_p);
    assert(NULL != value_str);
    assert(NULL != stringarr);

    for (index=0; index<stringarr_size; index++)
    {
        assert(NULL != stringarr[index]);
        if (0 == strcmp( value_str, stringarr[index]))
        {
            *value_p = index;
            return (0);
        }
    }

    return (-1);
}

#if !defined(__linux__) && !defined(WIN32)
static char *mpl_strdup(const char *s)
{
    char *tmp;
    int len;
    len = strlen(s)+1;
    tmp = malloc(len);
    if (tmp == NULL)
        return NULL;
    memset(tmp,0,len);
    strcpy(tmp,s);
    return tmp;
}
#endif


static int
    convert_enum_name_to_int64(const char* name_p,
                               int64_t *value_p,
                               const mpl_enum_value_t enum_values[],
                               int enum_values_size)
{
    int index;

    assert(NULL != value_p);
    assert(NULL != name_p);
    assert(NULL != enum_values);

    for (index = 0; index < enum_values_size; index++)
    {
        assert(NULL != enum_values[index].name_p);
        if (!strcmp(name_p, enum_values[index].name_p))
        {
            *value_p = enum_values[index].value;
            return (0);
        }
    }

    return (-1);
}

static const char *convert_enum_value_to_name(int64_t value,
                                              const mpl_enum_value_t enum_values[],
                                              int enum_values_size)
{
    int index;

    assert(NULL != enum_values);

    for (index = 0; index < enum_values_size; index++) {
        if (value == enum_values[index].value)
        {
            assert(NULL != enum_values[index].name_p);
            return enum_values[index].name_p;
        }
    }

    return NULL;
}

static int check_integer_ranges(int64_t value,
                                const mpl_integer_range_t integer_ranges[],
                                int integer_ranges_size)
{
    int i;

    for (i = 0; i < integer_ranges_size; i++) {
        assert(integer_ranges[i].first <= integer_ranges[i].last);
        if ((value >= integer_ranges[i].first) &&
            (value <= integer_ranges[i].last)) {
            return ((int) integer_ranges[i].id);
        }
    }
    return -1;
}

static const mpl_field_value_t *get_field_from_id(int field_id,
                                                  const mpl_field_value_t field_values[],
                                                  int field_values_size)
{
    int i;

    if (field_values_size == 0)
        return NULL;

    for (i = 0; i < field_values_size; i++)
    {
        if (field_values[i].field_id == field_id)
            return &field_values[i];
    }

    return NULL;
}

static const mpl_field_value_t *get_field_from_name(const char *field_str,
                                                    size_t field_strlen,
                                                    const mpl_field_value_t field_values[],
                                                    int field_values_size)
{
    int i;

    if (field_values_size == 0 || field_str == NULL)
        return NULL;

    for (i = 0; i < field_values_size; i++)
    {
        if ((strlen(field_values[i].name_p) == field_strlen) &&
            (0 == strncmp(field_values[i].name_p, field_str, field_strlen)))
            return &field_values[i];
    }

    return NULL;
}

static const char *get_field_name(int param_id,
                                  int field_id,
                                  mpl_param_descr_set_t *param_descr_p)
{
    const mpl_field_value_t *field_value_p;

    field_value_p = get_field_from_id(field_id,
                                      param_descr_p->array2[PARAMID_TO_INDEX(param_id)].field_values,
                                      param_descr_p->array2[PARAMID_TO_INDEX(param_id)].field_values_size
                                     );
    if (field_value_p == NULL)
        return NULL;

    return field_value_p->name_p;
}

static mpl_param_element_id_t get_field_paramid(int param_id,
                                                int field_id,
                                                mpl_param_descr_set_t *param_descr_p)
{
    const mpl_field_value_t *field_value_p;

    field_value_p = get_field_from_id(field_id,
                                      param_descr_p->array2[PARAMID_TO_INDEX(param_id)].field_values,
                                      param_descr_p->array2[PARAMID_TO_INDEX(param_id)].field_values_size
                                     );
    if (field_value_p == NULL)
        return MPL_PARAM_ID_UNDEFINED;

    return field_value_p->param_id;
}

static mpl_param_element_id_t get_field_context_paramid(int param_id,
                                                        int field_id,
                                                        mpl_param_descr_set_t *param_descr_p)
{
    const mpl_field_value_t *field_value_p;

    field_value_p = get_field_from_id(field_id,
                                      param_descr_p->array2[PARAMID_TO_INDEX(param_id)].field_values,
                                      param_descr_p->array2[PARAMID_TO_INDEX(param_id)].field_values_size
                                     );
    if (field_value_p == NULL)
        return MPL_PARAM_ID_UNDEFINED;

    return field_value_p->context_id;
}

static int get_child_paramid(int param_id,
                             int child_idx,
                             mpl_param_descr_set_t *param_descr_p)
{
    int num_children;

    num_children = param_descr_p->array2[PARAMID_TO_INDEX(param_id)].children_size;
    if ((child_idx < 0) || (child_idx >= num_children))  {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,("Child index out of bounds\n"));
        set_errno(E_MPL_INVALID_PARAMETER);
        return MPL_PARAM_ID_UNDEFINED;
    }
    return param_descr_p->array2[PARAMID_TO_INDEX(param_id)].children[child_idx];
}

static int get_child_index(mpl_param_element_id_t param_id,
                           mpl_param_element_id_t child_id,
                           mpl_param_descr_set_t *param_descr_p)
{
    int num_children;
    int i;

    num_children = param_descr_p->array2[PARAMID_TO_INDEX(param_id)].children_size;
    for (i = 0; i < num_children; i++) {
        if (param_descr_p->array2[PARAMID_TO_INDEX(param_id)].children[i] == child_id)
            return i;
    }
    return -1;
}

#if 1
void mpl_dbg_param_list_print(mpl_list_t *list_p)
{
    size_t buflen;
    char *buf_p;

    buflen = mpl_param_list_pack(list_p, NULL, 0);
    buf_p = malloc(buflen+1);
    (void) mpl_param_list_pack(list_p, buf_p, buflen+1);
    printf("List: %s\n", buf_p);
    free(buf_p);
}
#endif

bool _mpl_assert_typesize(size_t typesize, mpl_param_element_id_t param_id)
{
    bool typesize_equal;

    if (mpl_param_id_sizeof_param_value(param_id) == 0)
        return true;

    typesize_equal = (typesize == mpl_param_id_sizeof_param_value(param_id));
    if (!typesize_equal)
    {
        MPL_DBG_TRACE_ERROR(E_MPL_INVALID_PARAMETER,
                            ("Parameter size mismatch: "
                             "sizeof(%s) is %zu (not %zu)\n",
                             mpl_param_id_get_string(param_id),
                             mpl_param_id_sizeof_param_value(param_id),
                             typesize)); /*lint !e557 %zu is C99 */
    }

    assert(typesize_equal);

    return typesize_equal;
}
