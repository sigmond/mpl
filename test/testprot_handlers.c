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
static mpl_list_t *handle_TestEchoIntPN(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoEnum(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoEnumPN(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoBool(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoBoolPN(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoString(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoStringPN(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoAddr(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoAddrPN(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoArray(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoArrayPN(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoTuple(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoTuplePN(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoBag(mpl_bag_t *reqParams);
static mpl_list_t *handle_TestEchoBagPN(mpl_bag_t *reqParams);

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
        case TESTP_PARAM_ID(TestEchoIntPN_Req):
            return handle_TestEchoIntPN(reqParams);
        case TESTP_PARAM_ID(TestEchoEnum_Req):
            return handle_TestEchoEnum(reqParams);
        case TESTP_PARAM_ID(TestEchoEnumPN_Req):
            return handle_TestEchoEnumPN(reqParams);
        case TESTP_PARAM_ID(TestEchoBool_Req):
            return handle_TestEchoBool(reqParams);
        case TESTP_PARAM_ID(TestEchoBoolPN_Req):
            return handle_TestEchoBoolPN(reqParams);
        case TESTP_PARAM_ID(TestEchoString_Req):
            return handle_TestEchoString(reqParams);
        case TESTP_PARAM_ID(TestEchoStringPN_Req):
            return handle_TestEchoStringPN(reqParams);
        case TESTP_PARAM_ID(TestEchoAddr_Req):
            return handle_TestEchoAddr(reqParams);
        case TESTP_PARAM_ID(TestEchoAddrPN_Req):
            return handle_TestEchoAddrPN(reqParams);
        case TESTP_PARAM_ID(TestEchoArray_Req):
            return handle_TestEchoArray(reqParams);
        case TESTP_PARAM_ID(TestEchoArrayPN_Req):
            return handle_TestEchoArrayPN(reqParams);
        case TESTP_PARAM_ID(TestEchoTuple_Req):
            return handle_TestEchoTuple(reqParams);
        case TESTP_PARAM_ID(TestEchoTuplePN_Req):
            return handle_TestEchoTuplePN(reqParams);
        case TESTP_PARAM_ID(TestEchoBag_Req):
            return handle_TestEchoBag(reqParams);
        case TESTP_PARAM_ID(TestEchoBagPN_Req):
            return handle_TestEchoBagPN(reqParams);
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

static mpl_list_t *handle_TestEchoIntPN(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_EXISTS(reqParams, Int))
        TESTP_ADD_INT(&respParams, Int, TESTP_GET_INT(reqParams, Int));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, IntArr); i++)
        TESTP_ADD_INT_TAG(&respParams, IntArr, TESTP_GET_INT_TAG(reqParams, IntArr, i), i);
    if (TESTP_EXISTS(reqParams, Sint8))
        TESTP_ADD_SINT8(&respParams, Sint8, TESTP_GET_SINT8(reqParams, Sint8));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Sint8Arr); i++)
        TESTP_ADD_SINT8_TAG(&respParams, Sint8Arr, TESTP_GET_SINT8_TAG(reqParams, Sint8Arr, i), i);
    if (TESTP_EXISTS(reqParams, Sint16))
        TESTP_ADD_SINT16(&respParams, Sint16, TESTP_GET_SINT16(reqParams, Sint16));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Sint16Arr); i++)
        TESTP_ADD_SINT16_TAG(&respParams, Sint16Arr, TESTP_GET_SINT16_TAG(reqParams, Sint16Arr, i), i);
    if (TESTP_EXISTS(reqParams, Sint32))
        TESTP_ADD_SINT32(&respParams, Sint32, TESTP_GET_SINT32(reqParams, Sint32));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Sint32Arr); i++)
        TESTP_ADD_SINT32_TAG(&respParams, Sint32Arr, TESTP_GET_SINT32_TAG(reqParams, Sint32Arr, i), i);
    if (TESTP_EXISTS(reqParams, Sint64))
        TESTP_ADD_SINT64(&respParams, Sint64, TESTP_GET_SINT64(reqParams, Sint64));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Sint64Arr); i++)
        TESTP_ADD_SINT64_TAG(&respParams, Sint64Arr, TESTP_GET_SINT64_TAG(reqParams, Sint64Arr, i), i);
    if (TESTP_EXISTS(reqParams, Uint8))
        TESTP_ADD_UINT8(&respParams, Uint8, TESTP_GET_UINT8(reqParams, Uint8));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Uint8Arr); i++)
        TESTP_ADD_UINT8_TAG(&respParams, Uint8Arr, TESTP_GET_UINT8_TAG(reqParams, Uint8Arr, i), i);
    if (TESTP_EXISTS(reqParams, Uint16))
        TESTP_ADD_UINT16(&respParams, Uint16, TESTP_GET_UINT16(reqParams, Uint16));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Uint16Arr); i++)
        TESTP_ADD_UINT16_TAG(&respParams, Uint16Arr, TESTP_GET_UINT16_TAG(reqParams, Uint16Arr, i), i);
    if (TESTP_EXISTS(reqParams, Uint32))
        TESTP_ADD_UINT32(&respParams, Uint32, TESTP_GET_UINT32(reqParams, Uint32));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Uint32Arr); i++)
        TESTP_ADD_UINT32_TAG(&respParams, Uint32Arr, TESTP_GET_UINT32_TAG(reqParams, Uint32Arr, i), i);
    if (TESTP_EXISTS(reqParams, Uint64))
        TESTP_ADD_UINT64(&respParams, Uint64, TESTP_GET_UINT64(reqParams, Uint64));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Uint64Arr); i++)
        TESTP_ADD_UINT64_TAG(&respParams, Uint64Arr, TESTP_GET_UINT64_TAG(reqParams, Uint64Arr, i), i);
    
    TESTP_ADD_BAG(&respMsg,TestEchoIntPN_Resp,respParams);
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

static mpl_list_t *handle_TestEchoEnumPN(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_EXISTS(reqParams, Se))
        TESTP_ADD_ENUM_FROM_VAR(&respParams, Se, TESTP_GET_ENUM(reqParams, Se));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, SeArr); i++)
        TESTP_ADD_ENUM_FROM_VAR_TAG(&respParams, SeArr, TESTP_GET_ENUM_TAG(reqParams, SeArr, i), i);
    if (TESTP_EXISTS(reqParams, Se8))
        TESTP_ADD_SIGNED_ENUM8_FROM_VAR(&respParams, Se8, TESTP_GET_SIGNED_ENUM8(reqParams, Se8));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Se8Arr); i++)
        TESTP_ADD_SIGNED_ENUM8_FROM_VAR_TAG(&respParams, Se8Arr, TESTP_GET_SIGNED_ENUM8_TAG(reqParams, Se8Arr, i), i);
    if (TESTP_EXISTS(reqParams, Se16))
        TESTP_ADD_SIGNED_ENUM16_FROM_VAR(&respParams, Se16, TESTP_GET_SIGNED_ENUM16(reqParams, Se16));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Se16Arr); i++)
        TESTP_ADD_SIGNED_ENUM16_FROM_VAR_TAG(&respParams, Se16Arr, TESTP_GET_SIGNED_ENUM16_TAG(reqParams, Se16Arr, i), i);
    if (TESTP_EXISTS(reqParams, Se32))
        TESTP_ADD_SIGNED_ENUM32_FROM_VAR(&respParams, Se32, TESTP_GET_SIGNED_ENUM32(reqParams, Se32));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Se32Arr); i++)
        TESTP_ADD_SIGNED_ENUM32_FROM_VAR_TAG(&respParams, Se32Arr, TESTP_GET_SIGNED_ENUM32_TAG(reqParams, Se32Arr, i), i);
    if (TESTP_EXISTS(reqParams, Ue8))
        TESTP_ADD_ENUM8_FROM_VAR(&respParams, Ue8, TESTP_GET_ENUM8(reqParams, Ue8));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Ue8Arr); i++)
        TESTP_ADD_ENUM8_FROM_VAR_TAG(&respParams, Ue8Arr, TESTP_GET_ENUM8_TAG(reqParams, Ue8Arr, i), i);
    if (TESTP_EXISTS(reqParams, Ue16))
        TESTP_ADD_ENUM16_FROM_VAR(&respParams, Ue16, TESTP_GET_ENUM16(reqParams, Ue16));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Ue16Arr); i++)
        TESTP_ADD_ENUM16_FROM_VAR_TAG(&respParams, Ue16Arr, TESTP_GET_ENUM16_TAG(reqParams, Ue16Arr, i), i);
    if (TESTP_EXISTS(reqParams, Ue32))
        TESTP_ADD_ENUM32_FROM_VAR(&respParams, Ue32, TESTP_GET_ENUM32(reqParams, Ue32));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Ue32Arr); i++)
        TESTP_ADD_ENUM32_FROM_VAR_TAG(&respParams, Ue32Arr, TESTP_GET_ENUM32_TAG(reqParams, Ue32Arr, i), i);

    TESTP_ADD_BAG(&respMsg,TestEchoEnumPN_Resp,respParams);
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

static mpl_list_t *handle_TestEchoBoolPN(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_EXISTS(reqParams, MyBool))
        TESTP_ADD_BOOL(&respParams, MyBool, TESTP_GET_BOOL(reqParams, MyBool));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, BoolArr); i++)
        TESTP_ADD_BOOL_TAG(&respParams, BoolArr, TESTP_GET_BOOL_TAG(reqParams, BoolArr, i), i);
    if (TESTP_EXISTS(reqParams, Bool8))
        TESTP_ADD_BOOL8(&respParams, Bool8, TESTP_GET_BOOL8(reqParams, Bool8));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Bool8Arr); i++)
        TESTP_ADD_BOOL8_TAG(&respParams, Bool8Arr, TESTP_GET_BOOL8_TAG(reqParams, Bool8Arr, i), i);
    
    TESTP_ADD_BAG(&respMsg,TestEchoBoolPN_Resp,respParams);
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

static mpl_list_t *handle_TestEchoStringPN(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_EXISTS(reqParams, String))
        TESTP_ADD_STRING(&respParams, String, TESTP_GET_STRING_PTR(reqParams, String));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, StringArr); i++)
        TESTP_ADD_STRING_TAG(&respParams, StringArr, TESTP_GET_STRING_PTR_TAG(reqParams, StringArr, i), i);
    if (TESTP_EXISTS(reqParams, WString))
        TESTP_ADD_WSTRING(&respParams, WString, TESTP_GET_WSTRING_PTR(reqParams, WString));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, WStringArr); i++)
        TESTP_ADD_WSTRING_TAG(&respParams, WStringArr, TESTP_GET_WSTRING_PTR_TAG(reqParams, WStringArr, i), i);
    
    TESTP_ADD_BAG(&respMsg,TestEchoStringPN_Resp,respParams);
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

static mpl_list_t *handle_TestEchoAddrPN(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_EXISTS(reqParams, Addr))
        TESTP_ADD_ADDR(&respParams, Addr, TESTP_GET_ADDR(reqParams, Addr));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, AddrArr); i++)
        TESTP_ADD_ADDR_TAG(&respParams, AddrArr, TESTP_GET_ADDR_TAG(reqParams, AddrArr, i), i);
    
    TESTP_ADD_BAG(&respMsg,TestEchoAddrPN_Resp,respParams);
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

static mpl_list_t *handle_TestEchoArrayPN(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_EXISTS(reqParams, U8a))
        TESTP_ADD_UINT8_ARRAY(&respParams, U8a, TESTP_GET_UINT8_ARRAY_PTR(reqParams, U8a));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, U8aArr); i++)
        TESTP_ADD_UINT8_ARRAY_TAG(&respParams, U8aArr, TESTP_GET_UINT8_ARRAY_PTR_TAG(reqParams, U8aArr, i), i);
    if (TESTP_EXISTS(reqParams, U16a))
        TESTP_ADD_UINT16_ARRAY(&respParams, U16a, TESTP_GET_UINT16_ARRAY_PTR(reqParams, U16a));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, U16aArr); i++)
        TESTP_ADD_UINT16_ARRAY_TAG(&respParams, U16aArr, TESTP_GET_UINT16_ARRAY_PTR_TAG(reqParams, U16aArr, i), i);
    if (TESTP_EXISTS(reqParams, U32a))
        TESTP_ADD_UINT32_ARRAY(&respParams, U32a, TESTP_GET_UINT32_ARRAY_PTR(reqParams, U32a));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, U32aArr); i++)
        TESTP_ADD_UINT32_ARRAY_TAG(&respParams, U32aArr, TESTP_GET_UINT32_ARRAY_PTR_TAG(reqParams, U32aArr, i), i);
    
    TESTP_ADD_BAG(&respMsg,TestEchoArrayPN_Resp,respParams);
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

static mpl_list_t *handle_TestEchoTuplePN(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_EXISTS(reqParams, StringTup))
        TESTP_ADD_STRING_TUPLE(&respParams, StringTup, TESTP_GET_STRING_TUPLE_PTR(reqParams, StringTup));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, StringTupArr); i++)
        TESTP_ADD_STRING_TUPLE_TAG(&respParams, StringTupArr, TESTP_GET_STRING_TUPLE_PTR_TAG(reqParams, StringTupArr, i), i);
    if (TESTP_EXISTS(reqParams, IntTup))
        TESTP_ADD_INT_TUPLE(&respParams, IntTup, TESTP_GET_INT_TUPLE_PTR(reqParams, IntTup));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, IntTupArr); i++)
        TESTP_ADD_INT_TUPLE_TAG(&respParams, IntTupArr, TESTP_GET_INT_TUPLE_PTR_TAG(reqParams, IntTupArr, i), i);
    if (TESTP_EXISTS(reqParams, StrintTup))
        TESTP_ADD_STRINT_TUPLE(&respParams, StrintTup, TESTP_GET_STRINT_TUPLE_PTR(reqParams, StrintTup));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, StrintTupArr); i++)
        TESTP_ADD_STRINT_TUPLE_TAG(&respParams, StrintTupArr, TESTP_GET_STRINT_TUPLE_PTR_TAG(reqParams, StrintTupArr, i), i);
    if (TESTP_EXISTS(reqParams, Struint8Tup))
        TESTP_ADD_STRUINT8_TUPLE(&respParams, Struint8Tup, TESTP_GET_STRUINT8_TUPLE_PTR(reqParams, Struint8Tup));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, Struint8TupArr); i++)
        TESTP_ADD_STRUINT8_TUPLE_TAG(&respParams, Struint8TupArr, TESTP_GET_STRUINT8_TUPLE_PTR_TAG(reqParams, Struint8TupArr, i), i);
    
    TESTP_ADD_BAG(&respMsg,TestEchoTuplePN_Resp,respParams);
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

static mpl_list_t *handle_TestEchoBagPN(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;
    int i;

    if (TESTP_EXISTS(reqParams, Bag))
        TESTP_ADD_BAG(&respParams, Bag, TESTP_GET_BAG_PTR(reqParams, Bag));
    for (i = 1; i <= TESTP_PARAM_COUNT(reqParams, BagArr); i++)
        TESTP_ADD_BAG_TAG(&respParams, BagArr, TESTP_GET_BAG_PTR_TAG(reqParams, BagArr, i), i);
    
    TESTP_ADD_BAG(&respMsg,TestEchoBagPN_Resp,respParams);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

