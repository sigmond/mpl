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

    if (TESTP_TestEchoInt_Req_i_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_i(&respParams, TESTP_GET_TestEchoInt_Req_i(reqParams));
    if (TESTP_TestEchoInt_Req_s8_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_s8(&respParams, TESTP_GET_TestEchoInt_Req_s8(reqParams));
    if (TESTP_TestEchoInt_Req_s16_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_s16(&respParams, TESTP_GET_TestEchoInt_Req_s16(reqParams));
    if (TESTP_TestEchoInt_Req_s32_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_s32(&respParams, TESTP_GET_TestEchoInt_Req_s32(reqParams));
    if (TESTP_TestEchoInt_Req_s64_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_s64(&respParams, TESTP_GET_TestEchoInt_Req_s64(reqParams));
    if (TESTP_TestEchoInt_Req_u8_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_u8(&respParams, TESTP_GET_TestEchoInt_Req_u8(reqParams));
    if (TESTP_TestEchoInt_Req_u16_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_u16(&respParams, TESTP_GET_TestEchoInt_Req_u16(reqParams));
    if (TESTP_TestEchoInt_Req_u32_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_u32(&respParams, TESTP_GET_TestEchoInt_Req_u32(reqParams));
    if (TESTP_TestEchoInt_Req_u64_EXISTS(reqParams))
        TESTP_ADD_TestEchoInt_Resp_u64(&respParams, TESTP_GET_TestEchoInt_Req_u64(reqParams));

    TESTP_ADD_BAG(&respMsg,TestEchoInt_Resp,respParams);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

static mpl_list_t *handle_TestEchoEnum(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;

    if (TESTP_TestEchoEnum_Req_se_EXISTS(reqParams))
        TESTP_ADD_TestEchoEnum_Resp_se(&respParams, TESTP_GET_TestEchoEnum_Req_se(reqParams));
    if (TESTP_TestEchoEnum_Req_se8_EXISTS(reqParams))
        TESTP_ADD_TestEchoEnum_Resp_se8(&respParams, TESTP_GET_TestEchoEnum_Req_se8(reqParams));
    if (TESTP_TestEchoEnum_Req_se16_EXISTS(reqParams))
        TESTP_ADD_TestEchoEnum_Resp_se16(&respParams, TESTP_GET_TestEchoEnum_Req_se16(reqParams));
    if (TESTP_TestEchoEnum_Req_se32_EXISTS(reqParams))
        TESTP_ADD_TestEchoEnum_Resp_se32(&respParams, TESTP_GET_TestEchoEnum_Req_se32(reqParams));
    if (TESTP_TestEchoEnum_Req_ue8_EXISTS(reqParams))
        TESTP_ADD_TestEchoEnum_Resp_ue8(&respParams, TESTP_GET_TestEchoEnum_Req_ue8(reqParams));
    if (TESTP_TestEchoEnum_Req_ue16_EXISTS(reqParams))
        TESTP_ADD_TestEchoEnum_Resp_ue16(&respParams, TESTP_GET_TestEchoEnum_Req_ue16(reqParams));
    if (TESTP_TestEchoEnum_Req_ue32_EXISTS(reqParams))
        TESTP_ADD_TestEchoEnum_Resp_ue32(&respParams, TESTP_GET_TestEchoEnum_Req_ue32(reqParams));

    TESTP_ADD_BAG(&respMsg,TestEchoEnum_Resp,respParams);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

static mpl_list_t *handle_TestEchoBool(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;

    if (TESTP_TestEchoBool_Req_b_EXISTS(reqParams))
        TESTP_ADD_TestEchoBool_Resp_b(&respParams, TESTP_GET_TestEchoBool_Req_b(reqParams));
    if (TESTP_TestEchoBool_Req_b8_EXISTS(reqParams))
        TESTP_ADD_TestEchoBool_Resp_b8(&respParams, TESTP_GET_TestEchoBool_Req_b8(reqParams));

    TESTP_ADD_BAG(&respMsg,TestEchoBool_Resp,respParams);
    mpl_param_list_destroy(&respParams);
    return respMsg;
}

static mpl_list_t *handle_TestEchoString(mpl_bag_t *reqParams)
{
    mpl_list_t *respMsg = NULL;
    mpl_bag_t *respParams = NULL;

    if (TESTP_TestEchoString_Req_s_EXISTS(reqParams))
        TESTP_ADD_TestEchoString_Resp_s(&respParams, TESTP_GET_TestEchoString_Req_s_PTR(reqParams));
    if (TESTP_TestEchoString_Req_ws_EXISTS(reqParams))
        TESTP_ADD_TestEchoString_Resp_ws(&respParams, TESTP_GET_TestEchoString_Req_ws_PTR(reqParams));

    TESTP_ADD_BAG(&respMsg,TestEchoString_Resp,respParams);
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
