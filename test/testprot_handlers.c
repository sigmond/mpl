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

#include "testprotocol.h"
#include <assert.h>
#include <stdlib.h>


static mpl_list_t *handle_TestEchoInt(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoEnum(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoBool(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoString(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoAddr(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoArray(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoTuple(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoBag(mpl_bag_t *reqParams);

mpl_list_t *handle_testprot(mpl_list_t *reqMsg)
{
    mpl_bag_t *reqParams;

    if (!TESTPROT_IS_COMMAND(reqMsg)) {
        goto req_failure;
    }

    reqParams = TESTPROT_GET_COMMAND_PARAMS_PTR(reqMsg);

    switch (TESTPROT_GET_COMMAND_ID(reqMsg)) {
        case TESTP_PARAM_ID(TestEchoInt_Req):
            return handle_TestEchoInt(reqParams);
        case TESTP_PARAM_ID(TestEchoEnum_Req):
            return handle_TestEchoEnum(reqParams);
        case TESTP_PARAM_ID(TestEchoBool_Req):
            return handle_TestEchoBool(reqParams);
        case TESTP_PARAM_ID(TestEchoString_Req):
            return handle_TestEchoString(reqParams);
        case TESTP_PARAM_ID(TestEchoAddr_Req):
            return handle_TestEchoAddr(reqParams);
        case TESTP_PARAM_ID(TestEchoArray_Req):
            return handle_TestEchoArray(reqParams);
        case TESTP_PARAM_ID(TestEchoTuple_Req):
            return handle_TestEchoTuple(reqParams);
        case TESTP_PARAM_ID(TestEchoBag_Req):
            return handle_TestEchoBag(reqParams);
        default:
            break;
    }

req_failure:
    {
        mpl_list_t *respMsg = NULL;
        mpl_bag_t *respParams = NULL;

        mpl_add_param_to_list(&respMsg,
                              TESTPROT_COMMAND_ID_TO_RESPONSE_ID(TESTPROT_GET_COMMAND_ID(reqMsg)),
                              respParams);
        mpl_param_list_destroy(&respParams);
        return respMsg;
    }
}

static mpl_list_t *handle_TestEchoInt(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_TestEchoInt_Req_i_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_i(&respParams, TESTP_GET_TestEchoInt_Req_i(reqParams));
    for (i = 1; i <= TESTP_TestEchoInt_Req_i_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoInt_Resp_i_arr_TAG(&respParams, TESTP_GET_TestEchoInt_Req_i_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoInt_Req_s8_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_s8(&respParams, TESTP_GET_TestEchoInt_Req_s8(reqParams));
    for (i = 1; i <= TESTP_TestEchoInt_Req_s8_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoInt_Resp_s8_arr_TAG(&respParams, TESTP_GET_TestEchoInt_Req_s8_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoInt_Req_s16_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_s16(&respParams, TESTP_GET_TestEchoInt_Req_s16(reqParams));
    for (i = 1; i <= TESTP_TestEchoInt_Req_s16_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoInt_Resp_s16_arr_TAG(&respParams, TESTP_GET_TestEchoInt_Req_s16_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoInt_Req_s32_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_s32(&respParams, TESTP_GET_TestEchoInt_Req_s32(reqParams));
    for (i = 1; i <= TESTP_TestEchoInt_Req_s32_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoInt_Resp_s32_arr_TAG(&respParams, TESTP_GET_TestEchoInt_Req_s32_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoInt_Req_s64_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_s64(&respParams, TESTP_GET_TestEchoInt_Req_s64(reqParams));
    for (i = 1; i <= TESTP_TestEchoInt_Req_s64_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoInt_Resp_s64_arr_TAG(&respParams, TESTP_GET_TestEchoInt_Req_s64_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoInt_Req_u8_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_u8(&respParams, TESTP_GET_TestEchoInt_Req_u8(reqParams));
    for (i = 1; i <= TESTP_TestEchoInt_Req_u8_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoInt_Resp_u8_arr_TAG(&respParams, TESTP_GET_TestEchoInt_Req_u8_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoInt_Req_u16_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_u16(&respParams, TESTP_GET_TestEchoInt_Req_u16(reqParams));
    for (i = 1; i <= TESTP_TestEchoInt_Req_u16_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoInt_Resp_u16_arr_TAG(&respParams, TESTP_GET_TestEchoInt_Req_u16_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoInt_Req_u32_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_u32(&respParams, TESTP_GET_TestEchoInt_Req_u32(reqParams));
    for (i = 1; i <= TESTP_TestEchoInt_Req_u32_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoInt_Resp_u32_arr_TAG(&respParams, TESTP_GET_TestEchoInt_Req_u32_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoInt_Req_u64_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_u64(&respParams, TESTP_GET_TestEchoInt_Req_u64(reqParams));
    for (i = 1; i <= TESTP_TestEchoInt_Req_u64_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoInt_Resp_u64_arr_TAG(&respParams, TESTP_GET_TestEchoInt_Req_u64_arr_TAG(reqParams, i), i);

    TESTP_ADD_BAG(&respMsg,TestEchoInt_Resp,respParams);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

static mpl_list_t *handle_TestEchoEnum(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_TestEchoEnum_Req_se_EXISTS(reqParams))
        TESTP_ADD_TestEchoEnum_Resp_se(&respParams, TESTP_GET_TestEchoEnum_Req_se(reqParams));
    for (i = 1; i <= TESTP_TestEchoEnum_Req_se_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoEnum_Resp_se_arr_TAG(&respParams, TESTP_GET_TestEchoEnum_Req_se_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoEnum_Req_se8_EXISTS(reqParams))
        TESTP_ADD_TestEchoEnum_Resp_se8(&respParams, TESTP_GET_TestEchoEnum_Req_se8(reqParams));
    for (i = 1; i <= TESTP_TestEchoEnum_Req_se8_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoEnum_Resp_se8_arr_TAG(&respParams, TESTP_GET_TestEchoEnum_Req_se8_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoEnum_Req_se16_EXISTS(reqParams))
        TESTP_ADD_TestEchoEnum_Resp_se16(&respParams, TESTP_GET_TestEchoEnum_Req_se16(reqParams));
    for (i = 1; i <= TESTP_TestEchoEnum_Req_se16_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoEnum_Resp_se16_arr_TAG(&respParams, TESTP_GET_TestEchoEnum_Req_se16_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoEnum_Req_se32_EXISTS(reqParams))
        TESTP_ADD_TestEchoEnum_Resp_se32(&respParams, TESTP_GET_TestEchoEnum_Req_se32(reqParams));
    for (i = 1; i <= TESTP_TestEchoEnum_Req_se32_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoEnum_Resp_se32_arr_TAG(&respParams, TESTP_GET_TestEchoEnum_Req_se32_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoEnum_Req_ue8_EXISTS(reqParams))
        TESTP_ADD_TestEchoEnum_Resp_ue8(&respParams, TESTP_GET_TestEchoEnum_Req_ue8(reqParams));
    for (i = 1; i <= TESTP_TestEchoEnum_Req_ue8_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoEnum_Resp_ue8_arr_TAG(&respParams, TESTP_GET_TestEchoEnum_Req_ue8_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoEnum_Req_ue16_EXISTS(reqParams))
        TESTP_ADD_TestEchoEnum_Resp_ue16(&respParams, TESTP_GET_TestEchoEnum_Req_ue16(reqParams));
    for (i = 1; i <= TESTP_TestEchoEnum_Req_ue16_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoEnum_Resp_ue16_arr_TAG(&respParams, TESTP_GET_TestEchoEnum_Req_ue16_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoEnum_Req_ue32_EXISTS(reqParams))
        TESTP_ADD_TestEchoEnum_Resp_ue32(&respParams, TESTP_GET_TestEchoEnum_Req_ue32(reqParams));
    for (i = 1; i <= TESTP_TestEchoEnum_Req_ue32_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoEnum_Resp_ue32_arr_TAG(&respParams, TESTP_GET_TestEchoEnum_Req_ue32_arr_TAG(reqParams, i), i);

    TESTP_ADD_BAG(&respMsg,TestEchoEnum_Resp,respParams);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

static mpl_list_t *handle_TestEchoBool(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_TestEchoBool_Req_b_EXISTS(reqParams))
        TESTP_ADD_TestEchoBool_Resp_b(&respParams, TESTP_GET_TestEchoBool_Req_b(reqParams));
    for (i = 1; i <= TESTP_TestEchoBool_Req_b_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoBool_Resp_b_arr_TAG(&respParams, TESTP_GET_TestEchoBool_Req_b_arr_TAG(reqParams, i), i);
    if (TESTP_TestEchoBool_Req_b8_EXISTS(reqParams))
        TESTP_ADD_TestEchoBool_Resp_b8(&respParams, TESTP_GET_TestEchoBool_Req_b8(reqParams));
    for (i = 1; i <= TESTP_TestEchoBool_Req_b8_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoBool_Resp_b8_arr_TAG(&respParams, TESTP_GET_TestEchoBool_Req_b8_arr_TAG(reqParams, i), i);

    TESTP_ADD_BAG(&respMsg,TestEchoBool_Resp,respParams);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

static mpl_list_t *handle_TestEchoString(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_TestEchoString_Req_s_EXISTS(reqParams))
        TESTP_ADD_TestEchoString_Resp_s(&respParams, TESTP_GET_TestEchoString_Req_s_PTR(reqParams));
    for (i = 1; i <= TESTP_TestEchoString_Req_s_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoString_Resp_s_arr_TAG(&respParams, TESTP_GET_TestEchoString_Req_s_arr_PTR_TAG(reqParams, i), i);
    if (TESTP_TestEchoString_Req_ws_EXISTS(reqParams))
        TESTP_ADD_TestEchoString_Resp_ws(&respParams, TESTP_GET_TestEchoString_Req_ws_PTR(reqParams));
    for (i = 1; i <= TESTP_TestEchoString_Req_ws_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoString_Resp_ws_arr_TAG(&respParams, TESTP_GET_TestEchoString_Req_ws_arr_PTR_TAG(reqParams, i), i);

    TESTP_ADD_BAG(&respMsg,TestEchoString_Resp,respParams);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

static mpl_list_t *handle_TestEchoAddr(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_TestEchoAddr_Req_a_EXISTS(reqParams))
        TESTP_ADD_TestEchoAddr_Resp_a(&respParams, TESTP_GET_TestEchoAddr_Req_a(reqParams));
    for (i = 1; i <= TESTP_TestEchoAddr_Req_a_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoAddr_Resp_a_arr_TAG(&respParams, TESTP_GET_TestEchoAddr_Req_a_arr_TAG(reqParams, i), i);

    TESTP_ADD_BAG(&respMsg,TestEchoAddr_Resp,respParams);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

static mpl_list_t *handle_TestEchoArray(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_TestEchoArray_Req_u8a_EXISTS(reqParams))
        TESTP_ADD_TestEchoArray_Resp_u8a(&respParams, TESTP_GET_TestEchoArray_Req_u8a_PTR(reqParams));
    for (i = 1; i <= TESTP_TestEchoArray_Req_u8a_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoArray_Resp_u8a_arr_TAG(&respParams, TESTP_GET_TestEchoArray_Req_u8a_arr_PTR_TAG(reqParams, i), i);
    if (TESTP_TestEchoArray_Req_u16a_EXISTS(reqParams))
        TESTP_ADD_TestEchoArray_Resp_u16a(&respParams, TESTP_GET_TestEchoArray_Req_u16a_PTR(reqParams));
    for (i = 1; i <= TESTP_TestEchoArray_Req_u16a_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoArray_Resp_u16a_arr_TAG(&respParams, TESTP_GET_TestEchoArray_Req_u16a_arr_PTR_TAG(reqParams, i), i);
    if (TESTP_TestEchoArray_Req_u32a_EXISTS(reqParams))
        TESTP_ADD_TestEchoArray_Resp_u32a(&respParams, TESTP_GET_TestEchoArray_Req_u32a_PTR(reqParams));
    for (i = 1; i <= TESTP_TestEchoArray_Req_u32a_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoArray_Resp_u32a_arr_TAG(&respParams, TESTP_GET_TestEchoArray_Req_u32a_arr_PTR_TAG(reqParams, i), i);

    TESTP_ADD_BAG(&respMsg,TestEchoArray_Resp,respParams);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

static mpl_list_t *handle_TestEchoTuple(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_TestEchoTuple_Req_stringtup_EXISTS(reqParams))
        TESTP_ADD_TestEchoTuple_Resp_stringtup(&respParams, TESTP_GET_TestEchoTuple_Req_stringtup_PTR(reqParams));
    for (i = 1; i <= TESTP_TestEchoTuple_Req_stringtup_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoTuple_Resp_stringtup_arr_TAG(&respParams, TESTP_GET_TestEchoTuple_Req_stringtup_arr_PTR_TAG(reqParams, i), i);
    if (TESTP_TestEchoTuple_Req_inttup_EXISTS(reqParams))
        TESTP_ADD_TestEchoTuple_Resp_inttup(&respParams, TESTP_GET_TestEchoTuple_Req_inttup_PTR(reqParams));
    for (i = 1; i <= TESTP_TestEchoTuple_Req_inttup_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoTuple_Resp_inttup_arr_TAG(&respParams, TESTP_GET_TestEchoTuple_Req_inttup_arr_PTR_TAG(reqParams, i), i);
    if (TESTP_TestEchoTuple_Req_strinttup_EXISTS(reqParams))
        TESTP_ADD_TestEchoTuple_Resp_strinttup(&respParams, TESTP_GET_TestEchoTuple_Req_strinttup_PTR(reqParams));
    for (i = 1; i <= TESTP_TestEchoTuple_Req_strinttup_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoTuple_Resp_strinttup_arr_TAG(&respParams, TESTP_GET_TestEchoTuple_Req_strinttup_arr_PTR_TAG(reqParams, i), i);
    if (TESTP_TestEchoTuple_Req_struint8tup_EXISTS(reqParams))
        TESTP_ADD_TestEchoTuple_Resp_struint8tup(&respParams, TESTP_GET_TestEchoTuple_Req_struint8tup_PTR(reqParams));
    for (i = 1; i <= TESTP_TestEchoTuple_Req_struint8tup_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoTuple_Resp_struint8tup_arr_TAG(&respParams, TESTP_GET_TestEchoTuple_Req_struint8tup_arr_PTR_TAG(reqParams, i), i);

    TESTP_ADD_BAG(&respMsg,TestEchoTuple_Resp,respParams);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

static mpl_list_t *handle_TestEchoBag(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_TestEchoBag_Req_b_EXISTS(reqParams))
        TESTP_ADD_TestEchoBag_Resp_b(&respParams, TESTP_GET_TestEchoBag_Req_b_PTR(reqParams));
    for (i = 1; i <= TESTP_TestEchoBag_Req_b_arr_FIELD_COUNT(reqParams); i++)
        TESTP_ADD_TestEchoBag_Resp_b_arr_TAG(&respParams, TESTP_GET_TestEchoBag_Req_b_arr_PTR_TAG(reqParams, i), i);

    TESTP_ADD_BAG(&respMsg,TestEchoBag_Resp,respParams);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

