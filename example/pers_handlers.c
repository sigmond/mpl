/*
 *   Copyright 2013 Per Sigmond <per@sigmond.no>
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
 *
 */

#include "personnel.h"
#include "pers_db.h"
#include <assert.h>
#include <stdlib.h>


static mpl_list_t *handle_Add(mpl_bag_t *reqParams);
static mpl_list_t *handle_Get(mpl_bag_t *reqParams);
static mpl_list_t *handle_Delete(mpl_bag_t *reqParams);
static mpl_list_t *handle_Find(mpl_bag_t *reqParams);
static char *get_error_info(mpl_list_t *check_result_list_p);

mpl_list_t *handle_persfile(mpl_list_t *reqMsg)
{
    mpl_bag_t *reqParams;
    int check_ret;
    mpl_list_t *check_result_list_p = NULL;
    char *errorinfo = NULL;
    mpl_param_element_t *elem_p;

    if (!PERSFILE_IS_COMMAND(reqMsg)) {
        goto req_failure;
    }

    reqParams = PERSFILE_GET_COMMAND_PARAMS_PTR(reqMsg);
    elem_p = mpl_param_list_find(PERS_PARAM_ID(Req), reqMsg);
    assert(elem_p != NULL);
    check_ret = personnel_checkBag_Req(elem_p,
                                       &check_result_list_p);
    if (check_ret) {
        errorinfo = get_error_info(check_result_list_p);
        mpl_param_list_destroy(&check_result_list_p);
        goto req_failure;
    }

    switch (PERSFILE_GET_COMMAND_ID(reqMsg)) {
        case PERS_PARAM_ID(Add_Req):
            return handle_Add(reqParams);
        case PERS_PARAM_ID(Get_Req):
            return handle_Get(reqParams);
        case PERS_PARAM_ID(Delete_Req):
            return handle_Delete(reqParams);
        case PERS_PARAM_ID(Find_Req):
            return handle_Find(reqParams);
        default:
            break;
    }

req_failure:
    {
        mpl_list_t *respMsg = NULL;
        mpl_bag_t *respParams = NULL;
        PERS_ENUM_VAR_DECLARE_INIT(Error, error, parameter);
        PERS_ADD_Resp_error(&respParams, error);
        if (errorinfo) {
            PERS_ADD_Resp_errorinfo(&respParams, errorinfo);
            free(errorinfo);
        }
        if (PERS_Req_userdata_EXISTS(reqParams))
            PERS_ADD_Resp_userdata(&respParams, PERS_GET_Req_userdata(reqParams));
        mpl_add_param_to_list(&respMsg,
                              PERSFILE_COMMAND_ID_TO_RESPONSE_ID(PERSFILE_GET_COMMAND_ID(reqMsg)),
                              respParams);
        mpl_param_list_destroy(&respParams);
        return respMsg;
    }
}

static mpl_list_t *handle_Add(mpl_bag_t *reqParams)
{
    mpl_bag_t *employee;
    uint16_t number;
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    PERS_ENUM_VAR_DECLARE_INIT(Error, error, success);

    assert(PERS_Add_Req_employee_EXISTS(reqParams));
    employee = PERS_GET_Add_Req_employee_PTR(reqParams);
    error = persdb_Add(employee, &number);

    PERS_ADD_Add_Resp_number(&respParams, number);
    PERS_ADD_Resp_error(&respParams, error);
    if (PERS_Req_userdata_EXISTS(reqParams))
        PERS_ADD_Resp_userdata(&respParams, PERS_GET_Req_userdata(reqParams));
    PERS_ADD_BAG(&respMsg,Add_Resp,respParams);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

static mpl_list_t *handle_Get(mpl_bag_t *reqParams)
{
    uint16_t number;
    mpl_bag_t *employee = NULL;
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    PERS_ENUM_VAR_DECLARE_INIT(Error, error, success);

    assert(PERS_Get_Req_number_EXISTS(reqParams));
    number = PERS_GET_Get_Req_number(reqParams);
    error = persdb_Get(number, &employee);

    if (employee)
        PERS_ADD_Get_Resp_employee(&respParams, employee);
    PERS_ADD_Resp_error(&respParams, error);
    if (PERS_Req_userdata_EXISTS(reqParams))
        PERS_ADD_Resp_userdata(&respParams, PERS_GET_Req_userdata(reqParams));
    PERS_ADD_BAG(&respMsg,Get_Resp,respParams);
    mpl_param_list_destroy(&employee);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

static mpl_list_t *handle_Delete(mpl_bag_t *reqParams)
{
    uint16_t number;
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    PERS_ENUM_VAR_DECLARE_INIT(Error, error, success);

    assert(PERS_Delete_Req_number_EXISTS(reqParams));
    number = PERS_GET_Delete_Req_number(reqParams);
    error = persdb_Delete(number);

    PERS_ADD_Resp_error(&respParams, error);
    if (PERS_Req_userdata_EXISTS(reqParams))
        PERS_ADD_Resp_userdata(&respParams, PERS_GET_Req_userdata(reqParams));
    PERS_ADD_BAG(&respMsg,Delete_Resp,respParams);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

static mpl_list_t *handle_Find(mpl_bag_t *reqParams)
{
    char *first = NULL;
    char *middle = NULL;
    char *last = NULL;
    mpl_list_t *employees = NULL;
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    PERS_ENUM_VAR_DECLARE_INIT(Error, error, success);

    if (PERS_Find_Req_first_EXISTS(reqParams))
        first = PERS_GET_Find_Req_first_PTR(reqParams);
    if (PERS_Find_Req_middle_EXISTS(reqParams))
        middle = PERS_GET_Find_Req_middle_PTR(reqParams);
    if (PERS_Find_Req_last_EXISTS(reqParams))
        last = PERS_GET_Find_Req_last_PTR(reqParams);
    error = persdb_Find(first, middle, last, &employees);

    if (employees) {
        int tag = 1;
        mpl_list_t *tmp;

        MPL_LIST_FOR_EACH(employees, tmp) {
            mpl_bag_t *tmpempl =
                MPL_LIST_CONTAINER(tmp, mpl_param_element_t, list_entry)->value_p;
            PERS_ADD_Find_Resp_employees_TAG(&respParams, tmpempl, tag++);
        }
    }

    PERS_ADD_Resp_error(&respParams, error);
    if (PERS_Req_userdata_EXISTS(reqParams))
        PERS_ADD_Resp_userdata(&respParams, PERS_GET_Req_userdata(reqParams));
    PERS_ADD_BAG(&respMsg,Find_Resp,respParams);
    mpl_param_list_destroy(&employees);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

static char *get_error_info(mpl_list_t *check_result_list_p)
{
    char *buf = NULL;
    int len;
    const char *prompt = "Protocol error: ";
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    options.force_field_pack_mode = true;

    len = mpl_param_list_pack_extended(check_result_list_p,
                                       NULL,
                                       0,
                                       &options);
    buf = calloc(1, strlen(prompt) + len + 1);
    strcat(buf, prompt);
    if (buf != NULL) {
        (void)mpl_param_list_pack_extended(check_result_list_p,
                                           buf + strlen(prompt),
                                           len+1,
                                           &options);
    }
    return buf;
}
