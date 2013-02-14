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
    TestEchoInt_Req copy2(NULL,
                          NULL,
                          0,
                          NULL,
                          NULL,
                          0,
                          NULL,
                          NULL,
                          0,
                          NULL,
                          NULL,
                          0,
                          NULL,
                          NULL,
                          0,
                          NULL,
                          NULL,
                          0,
                          NULL,
                          NULL,
                          0,
                          NULL,
                          NULL,
                          0,
                          NULL,
                          NULL,
                          0
                         );
    copy2 = *copy; // Test assignment operator
    if (copy2 == *copy) // Test comparison operator
        printf("Equal (correct)\n");
    else
        printf("Not equal (ERROR)\n");
    delete copy->u64;
    copy->u64 = new uint64_t(987654);
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoInt_Resp *resp = new TestEchoInt_Resp(copy2.i,
                                                 copy2.i_arr,
                                                 copy2.num_i_arr,
                                                 copy2.s8,
                                                 copy2.s8_arr,
                                                 copy2.num_s8_arr,
                                                 copy2.s16,
                                                 copy2.s16_arr,
                                                 copy2.num_s16_arr,
                                                 copy2.s32,
                                                 copy2.s32_arr,
                                                 copy2.num_s32_arr,
                                                 copy2.s64,
                                                 copy2.s64_arr,
                                                 copy2.num_s64_arr,
                                                 copy2.u8,
                                                 copy2.u8_arr,
                                                 copy2.num_u8_arr,
                                                 copy2.u16,
                                                 copy2.u16_arr,
                                                 copy2.num_u16_arr,
                                                 copy2.u32,
                                                 copy2.u32_arr,
                                                 copy2.num_u32_arr,
                                                 copy2.u64,
                                                 copy2.u64_arr,
                                                 copy2.num_u64_arr
                                                );
    delete copy;
    return resp;
}

static BAG *handle_TestEchoEnum(TestEchoEnum_Req *reqObj)
{
    TestEchoEnum_Req *copy = new TestEchoEnum_Req(*reqObj); // Test copy constructor
    TestEchoEnum_Req copy2(NULL,
                           NULL,
                           0,
                           NULL,
                           NULL,
                           0,
                           NULL,
                           NULL,
                           0,
                           NULL,
                           NULL,
                           0,
                           NULL,
                           NULL,
                           0,
                           NULL,
                           NULL,
                           0,
                           NULL,
                           NULL,
                           0
                          );
    copy2 = *copy; // Test assignment operator
    if (copy2 == *copy) // Test comparison operator
        printf("Equal (correct)\n");
    else
        printf("Not equal (ERROR)\n");
    delete copy->ue32;
    copy->ue32 = new testprotocol_Ue32_t(1);
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoEnum_Resp *resp = new TestEchoEnum_Resp(copy2.se,
                                                    copy2.se_arr,
                                                    copy2.num_se_arr,
                                                    copy2.se8,
                                                    copy2.se8_arr,
                                                    copy2.num_se8_arr,
                                                    copy2.se16,
                                                    copy2.se16_arr,
                                                    copy2.num_se16_arr,
                                                    copy2.se32,
                                                    copy2.se32_arr,
                                                    copy2.num_se32_arr,
                                                    copy2.ue8,
                                                    copy2.ue8_arr,
                                                    copy2.num_ue8_arr,
                                                    copy2.ue16,
                                                    copy2.ue16_arr,
                                                    copy2.num_ue16_arr,
                                                    copy2.ue32,
                                                    copy2.ue32_arr,
                                                    copy2.num_ue32_arr
                                                   );
    delete copy;
    return resp;
}

static BAG *handle_TestEchoBool(TestEchoBool_Req *reqObj)
{
    TestEchoBool_Req *copy = new TestEchoBool_Req(*reqObj); // Test copy constructor
    TestEchoBool_Req copy2(NULL,
                           NULL,
                           0,
                           NULL,
                           NULL,
                           0
                          );
    copy2 = *copy; // Test assignment operator
    if (copy2 == *copy) // Test comparison operator
        printf("Equal (correct)\n");
    else
        printf("Not equal (ERROR)\n");
    delete copy->b8;
    copy->b8 = new uint8_t(1);
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoBool_Resp *resp = new TestEchoBool_Resp(copy2.b,
                                                    copy2.b_arr,
                                                    copy2.num_b_arr,
                                                    copy2.b8,
                                                    copy2.b8_arr,
                                                    copy2.num_b8_arr
                                                   );
    delete copy;
    return resp;
}

static BAG *handle_TestEchoString(TestEchoString_Req *reqObj)
{
    TestEchoString_Req *copy = new TestEchoString_Req(*reqObj); // Test copy constructor
    TestEchoString_Req copy2(NULL,
                           NULL,
                           0,
                           NULL,
                           NULL,
                           0
                          );
    copy2 = *copy; // Test assignment operator
    if (copy2 == *copy) // Test comparison operator
        printf("Equal (correct)\n");
    else
        printf("Not equal (ERROR)\n");
    delete[] copy->s;
    copy->s = new char[strlen("hei")+1];
    strcpy(copy->s,"hei");
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoString_Resp *resp = new TestEchoString_Resp(copy2.s,
                                                        copy2.s_arr,
                                                        copy2.num_s_arr,
                                                        copy2.ws,
                                                        copy2.ws_arr,
                                                        copy2.num_ws_arr
                                                       );
    delete copy;
    return resp;
}

static BAG *handle_TestEchoAddr(TestEchoAddr_Req *reqObj)
{
    TestEchoAddr_Req *copy = new TestEchoAddr_Req(*reqObj); // Test copy constructor
    TestEchoAddr_Req copy2(NULL,
                           NULL,
                           0
                          );
    copy2 = *copy; // Test assignment operator
    if (copy2 == *copy) // Test comparison operator
        printf("Equal (correct)\n");
    else
        printf("Not equal (ERROR)\n");
    copy->a = (void*)0xabcd;
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoAddr_Resp *resp = new TestEchoAddr_Resp(copy2.a,
                                                    copy2.a_arr,
                                                    copy2.num_a_arr
                                                   );
    delete copy;
    return resp;
}

static BAG *handle_TestEchoArray(TestEchoArray_Req *reqObj)
{
    TestEchoArray_Req *copy = new TestEchoArray_Req(*reqObj); // Test copy constructor
    TestEchoArray_Req copy2(NULL,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            0
                           );
    copy2 = *copy; // Test assignment operator
    if (copy2 == *copy) // Test comparison operator
        printf("Equal (correct)\n");
    else
        printf("Not equal (ERROR)\n");
    delete copy->u32a;
    uint32_t arr[2] = {0xab, 0xcd};
    copy->u32a = new uint32_array(arr,2);
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoArray_Resp *resp = new TestEchoArray_Resp(copy2.u8a,
                                                      copy2.u8a_arr,
                                                      copy2.num_u8a_arr,
                                                      copy2.u16a,
                                                      copy2.u16a_arr,
                                                      copy2.num_u16a_arr,
                                                      copy2.u32a,
                                                      copy2.u32a_arr,
                                                      copy2.num_u32a_arr
                                                     );
    delete copy;
    return resp;
}

static BAG *handle_TestEchoTuple(TestEchoTuple_Req *reqObj)
{
    TestEchoTuple_Req *copy = new TestEchoTuple_Req(*reqObj); // Test copy constructor
    TestEchoTuple_Req copy2(NULL,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            0
                           );
    copy2 = *copy; // Test assignment operator
    if (copy2 == *copy) // Test comparison operator
        printf("Equal (correct)\n");
    else
        printf("Not equal (ERROR)\n");
    delete copy->stringtup;
    copy->stringtup = new string_tuple((char*)"hei",(char*)"hopp");
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoTuple_Resp *resp = new TestEchoTuple_Resp(copy2.stringtup,
                                                      copy2.stringtup_arr,
                                                      copy2.num_stringtup_arr,
                                                      copy2.inttup,
                                                      copy2.inttup_arr,
                                                      copy2.num_inttup_arr,
                                                      copy2.strinttup,
                                                      copy2.strinttup_arr,
                                                      copy2.num_strinttup_arr,
                                                      copy2.struint8tup,
                                                      copy2.struint8tup_arr,
                                                      copy2.num_struint8tup_arr
                                                     );
    delete copy;
    return resp;
}

static BAG *handle_TestEchoBag(TestEchoBag_Req *reqObj)
{
    TestEchoBag_Req *copy = new TestEchoBag_Req(*reqObj); // Test copy constructor
    TestEchoBag_Req copy2(NULL,
                          NULL,
                          0
                         );
    copy2 = *copy; // Test assignment operator
    if (copy2 == *copy) // Test comparison operator
        printf("Equal (correct)\n");
    else
        printf("Not equal (ERROR)\n");
    delete copy->b;
    copy->b = new Bag(100,NULL);
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoBag_Resp *resp = new TestEchoBag_Resp(copy2.b,
                                                  copy2.b_arr,
                                                  copy2.num_b_arr
                                                 );
    delete copy;
    return resp;
}

