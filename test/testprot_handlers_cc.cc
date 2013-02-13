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
#include "testprotocol.hh"
#include "testprot_handlers_cc.hh"
#include <stdlib.h>

using namespace testprotocol;

static BAG *handle_TestEchoInt(TestEchoInt_Req *reqObj);
static BAG *handle_TestEchoEnum(TestEchoEnum_Req *reqObj);
static BAG *handle_TestEchoBool(TestEchoBool_Req *reqObj);
static BAG *handle_TestEchoString(TestEchoString_Req *reqObj);
static BAG *handle_TestEchoAddr(TestEchoAddr_Req *reqObj);
static BAG *handle_TestEchoArray(TestEchoArray_Req *reqObj);
static BAG *handle_TestEchoTuple(TestEchoTuple_Req *reqObj);
static BAG *handle_TestEchoBag(TestEchoBag_Req *reqObj);

BAG *handle_testprot(BAG *reqObj)
{
    switch (reqObj->id()) {
        case TESTP_PARAM_ID(TestEchoInt_Req):
            return handle_TestEchoInt((TestEchoInt_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoEnum_Req):
            return handle_TestEchoEnum((TestEchoEnum_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoBool_Req):
            return handle_TestEchoBool((TestEchoBool_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoString_Req):
            return handle_TestEchoString((TestEchoString_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoAddr_Req):
            return handle_TestEchoAddr((TestEchoAddr_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoArray_Req):
            return handle_TestEchoArray((TestEchoArray_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoTuple_Req):
            return handle_TestEchoTuple((TestEchoTuple_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoBag_Req):
            return handle_TestEchoBag((TestEchoBag_Req*)reqObj);
        default:
            break;
    }
    return NULL;
}

static BAG *handle_TestEchoInt(TestEchoInt_Req *reqObj)
{
    TestEchoInt_Req *copy = new TestEchoInt_Req(*reqObj); // Test copy constructor
    TestEchoInt_Resp *resp = new TestEchoInt_Resp(copy->i,
                                                 copy->i_arr,
                                                 copy->num_i_arr,
                                                 copy->s8,
                                                 copy->s8_arr,
                                                 copy->num_s8_arr,
                                                 copy->s16,
                                                 copy->s16_arr,
                                                 copy->num_s16_arr,
                                                 copy->s32,
                                                 copy->s32_arr,
                                                 copy->num_s32_arr,
                                                 copy->s64,
                                                 copy->s64_arr,
                                                 copy->num_s64_arr,
                                                 copy->u8,
                                                 copy->u8_arr,
                                                 copy->num_u8_arr,
                                                 copy->u16,
                                                 copy->u16_arr,
                                                 copy->num_u16_arr,
                                                 copy->u32,
                                                 copy->u32_arr,
                                                 copy->num_u32_arr,
                                                 copy->u64,
                                                 copy->u64_arr,
                                                 copy->num_u64_arr
                                                );
    delete copy;
    return resp;
}

static BAG *handle_TestEchoEnum(TestEchoEnum_Req *reqObj)
{
    TestEchoEnum_Req *copy = new TestEchoEnum_Req(*reqObj); // Test copy constructor
    TestEchoEnum_Resp *resp = new TestEchoEnum_Resp(copy->se,
                                                    copy->se_arr,
                                                    copy->num_se_arr,
                                                    copy->se8,
                                                    copy->se8_arr,
                                                    copy->num_se8_arr,
                                                    copy->se16,
                                                    copy->se16_arr,
                                                    copy->num_se16_arr,
                                                    copy->se32,
                                                    copy->se32_arr,
                                                    copy->num_se32_arr,
                                                    copy->ue8,
                                                    copy->ue8_arr,
                                                    copy->num_ue8_arr,
                                                    copy->ue16,
                                                    copy->ue16_arr,
                                                    copy->num_ue16_arr,
                                                    copy->ue32,
                                                    copy->ue32_arr,
                                                    copy->num_ue32_arr
                                                   );
    delete copy;
    return resp;
}

static BAG *handle_TestEchoBool(TestEchoBool_Req *reqObj)
{
    return new TestEchoBool_Resp(reqObj->b,
                                 reqObj->b_arr,
                                 reqObj->num_b_arr,
                                 reqObj->b8,
                                 reqObj->b8_arr,
                                 reqObj->num_b8_arr
                                );
}

static BAG *handle_TestEchoString(TestEchoString_Req *reqObj)
{
    TestEchoString_Req *copy = new TestEchoString_Req(*reqObj); // Test copy constructor
    TestEchoString_Resp *resp = new TestEchoString_Resp(copy->s,
                                                        copy->s_arr,
                                                        copy->num_s_arr,
                                                        copy->ws,
                                                        copy->ws_arr,
                                                        copy->num_ws_arr
                                                       );
    delete copy;
    return resp;
}

static BAG *handle_TestEchoAddr(TestEchoAddr_Req *reqObj)
{
    TestEchoAddr_Req *copy = new TestEchoAddr_Req(*reqObj); // Test copy constructor
    TestEchoAddr_Resp *resp = new TestEchoAddr_Resp(copy->a,
                                                    copy->a_arr,
                                                    copy->num_a_arr
                                                   );
    delete copy;
    return resp;
}

static BAG *handle_TestEchoArray(TestEchoArray_Req *reqObj)
{
    TestEchoArray_Req *copy = new TestEchoArray_Req(*reqObj); // Test copy constructor
    TestEchoArray_Resp *resp = new TestEchoArray_Resp(copy->u8a,
                                                      copy->u8a_arr,
                                                      copy->num_u8a_arr,
                                                      copy->u16a,
                                                      copy->u16a_arr,
                                                      copy->num_u16a_arr,
                                                      copy->u32a,
                                                      copy->u32a_arr,
                                                      copy->num_u32a_arr
                                                     );
    delete copy;
    return resp;
}

static BAG *handle_TestEchoTuple(TestEchoTuple_Req *reqObj)
{
    TestEchoTuple_Req *copy = new TestEchoTuple_Req(*reqObj); // Test copy constructor
    TestEchoTuple_Resp *resp = new TestEchoTuple_Resp(copy->stringtup,
                                                      copy->stringtup_arr,
                                                      copy->num_stringtup_arr,
                                                      copy->inttup,
                                                      copy->inttup_arr,
                                                      copy->num_inttup_arr,
                                                      copy->strinttup,
                                                      copy->strinttup_arr,
                                                      copy->num_strinttup_arr,
                                                      copy->struint8tup,
                                                      copy->struint8tup_arr,
                                                      copy->num_struint8tup_arr
                                                     );
    delete copy;
    return resp;
}

static BAG *handle_TestEchoBag(TestEchoBag_Req *reqObj)
{
    TestEchoBag_Req *copy = new TestEchoBag_Req(*reqObj); // Test copy constructor
    TestEchoBag_Resp *resp = new TestEchoBag_Resp(copy->b,
                                                  copy->b_arr,
                                                  copy->num_b_arr
                                                 );
    delete copy;
    return resp;
}

