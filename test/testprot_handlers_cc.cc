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
static BAG *handle_TestEchoIntPN(TestEchoIntPN_Req *reqObj);
static BAG *handle_TestEchoEnum(TestEchoEnum_Req *reqObj);
static BAG *handle_TestEchoEnumPN(TestEchoEnumPN_Req *reqObj);
static BAG *handle_TestEchoBool(TestEchoBool_Req *reqObj);
static BAG *handle_TestEchoBoolPN(TestEchoBoolPN_Req *reqObj);
static BAG *handle_TestEchoString(TestEchoString_Req *reqObj);
static BAG *handle_TestEchoStringPN(TestEchoStringPN_Req *reqObj);
static BAG *handle_TestEchoAddr(TestEchoAddr_Req *reqObj);
static BAG *handle_TestEchoAddrPN(TestEchoAddrPN_Req *reqObj);
static BAG *handle_TestEchoArray(TestEchoArray_Req *reqObj);
static BAG *handle_TestEchoArrayPN(TestEchoArrayPN_Req *reqObj);
static BAG *handle_TestEchoTuple(TestEchoTuple_Req *reqObj);
static BAG *handle_TestEchoTuplePN(TestEchoTuplePN_Req *reqObj);
static BAG *handle_TestEchoBag(TestEchoBag_Req *reqObj);
static BAG *handle_TestEchoBagPN(TestEchoBagPN_Req *reqObj);

BAG *handle_testprot(BAG *reqObj)
{
    switch (reqObj->id()) {
        case TESTP_PARAM_ID(TestEchoInt_Req):
            return handle_TestEchoInt((TestEchoInt_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoIntPN_Req):
            return handle_TestEchoIntPN((TestEchoIntPN_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoEnum_Req):
            return handle_TestEchoEnum((TestEchoEnum_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoEnumPN_Req):
            return handle_TestEchoEnumPN((TestEchoEnumPN_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoBool_Req):
            return handle_TestEchoBool((TestEchoBool_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoBoolPN_Req):
            return handle_TestEchoBoolPN((TestEchoBoolPN_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoString_Req):
            return handle_TestEchoString((TestEchoString_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoStringPN_Req):
            return handle_TestEchoStringPN((TestEchoStringPN_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoAddr_Req):
            return handle_TestEchoAddr((TestEchoAddr_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoAddrPN_Req):
            return handle_TestEchoAddrPN((TestEchoAddrPN_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoArray_Req):
            return handle_TestEchoArray((TestEchoArray_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoArrayPN_Req):
            return handle_TestEchoArrayPN((TestEchoArrayPN_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoTuple_Req):
            return handle_TestEchoTuple((TestEchoTuple_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoTuplePN_Req):
            return handle_TestEchoTuplePN((TestEchoTuplePN_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoBag_Req):
            return handle_TestEchoBag((TestEchoBag_Req*)reqObj);
        case TESTP_PARAM_ID(TestEchoBagPN_Req):
            return handle_TestEchoBagPN((TestEchoBagPN_Req*)reqObj);
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

static BAG *handle_TestEchoIntPN(TestEchoIntPN_Req *reqObj)
{
    TestEchoIntPN_Req *copy = new TestEchoIntPN_Req(*reqObj); // Test copy constructor
    TestEchoIntPN_Req copy2(NULL,
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
    delete copy->_Uint64;
    copy->_Uint64 = new uint64_t(987654);
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoIntPN_Resp *resp = new TestEchoIntPN_Resp(copy2._Int,
                                                 copy2._IntArr,
                                                 copy2.num_IntArr,
                                                 copy2._Sint8,
                                                 copy2._Sint8Arr,
                                                 copy2.num_Sint8Arr,
                                                 copy2._Sint16,
                                                 copy2._Sint16Arr,
                                                 copy2.num_Sint16Arr,
                                                 copy2._Sint32,
                                                 copy2._Sint32Arr,
                                                 copy2.num_Sint32Arr,
                                                 copy2._Sint64,
                                                 copy2._Sint64Arr,
                                                 copy2.num_Sint64Arr,
                                                 copy2._Uint8,
                                                 copy2._Uint8Arr,
                                                 copy2.num_Uint8Arr,
                                                 copy2._Uint16,
                                                 copy2._Uint16Arr,
                                                 copy2.num_Uint16Arr,
                                                 copy2._Uint32,
                                                 copy2._Uint32Arr,
                                                 copy2.num_Uint32Arr,
                                                 copy2._Uint64,
                                                 copy2._Uint64Arr,
                                                 copy2.num_Uint64Arr
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

static BAG *handle_TestEchoEnumPN(TestEchoEnumPN_Req *reqObj)
{
    TestEchoEnumPN_Req *copy = new TestEchoEnumPN_Req(*reqObj); // Test copy constructor
    TestEchoEnumPN_Req copy2(NULL,
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
    delete copy->_Ue32;
    copy->_Ue32 = new testprotocol_Ue32_t(1);
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoEnumPN_Resp *resp = new TestEchoEnumPN_Resp(copy2._Se,
                                                        copy2._SeArr,
                                                        copy2.num_SeArr,
                                                        copy2._Se8,
                                                        copy2._Se8Arr,
                                                        copy2.num_Se8Arr,
                                                        copy2._Se16,
                                                        copy2._Se16Arr,
                                                        copy2.num_Se16Arr,
                                                        copy2._Se32,
                                                        copy2._Se32Arr,
                                                        copy2.num_Se32Arr,
                                                        copy2._Ue8,
                                                        copy2._Ue8Arr,
                                                        copy2.num_Ue8Arr,
                                                        copy2._Ue16,
                                                        copy2._Ue16Arr,
                                                        copy2.num_Ue16Arr,
                                                        copy2._Ue32,
                                                        copy2._Ue32Arr,
                                                        copy2.num_Ue32Arr
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

static BAG *handle_TestEchoBoolPN(TestEchoBoolPN_Req *reqObj)
{
    TestEchoBoolPN_Req *copy = new TestEchoBoolPN_Req(*reqObj); // Test copy constructor
    TestEchoBoolPN_Req copy2(NULL,
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
    delete copy->_Bool8;
    copy->_Bool8 = new uint8_t(1);
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoBoolPN_Resp *resp = new TestEchoBoolPN_Resp(copy2._MyBool,
                                                    copy2._BoolArr,
                                                    copy2.num_BoolArr,
                                                    copy2._Bool8,
                                                    copy2._Bool8Arr,
                                                    copy2.num_Bool8Arr
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

static BAG *handle_TestEchoStringPN(TestEchoStringPN_Req *reqObj)
{
    TestEchoStringPN_Req *copy = new TestEchoStringPN_Req(*reqObj); // Test copy constructor
    TestEchoStringPN_Req copy2(NULL,
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
    delete[] copy->_String;
    copy->_String = new char[strlen("hei")+1];
    strcpy(copy->_String,"hei");
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoStringPN_Resp *resp = new TestEchoStringPN_Resp(copy2._String,
                                                            copy2._StringArr,
                                                            copy2.num_StringArr,
                                                            copy2._WString,
                                                            copy2._WStringArr,
                                                            copy2.num_WStringArr
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

static BAG *handle_TestEchoAddrPN(TestEchoAddrPN_Req *reqObj)
{
    TestEchoAddrPN_Req *copy = new TestEchoAddrPN_Req(*reqObj); // Test copy constructor
    TestEchoAddrPN_Req copy2(NULL,
                             NULL,
                             0
                            );
    copy2 = *copy; // Test assignment operator
    if (copy2 == *copy) // Test comparison operator
        printf("Equal (correct)\n");
    else
        printf("Not equal (ERROR)\n");
    copy->_Addr = (void*)0xabcd;
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoAddrPN_Resp *resp = new TestEchoAddrPN_Resp(copy2._Addr,
                                                        copy2._AddrArr,
                                                        copy2.num_AddrArr
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

static BAG *handle_TestEchoArrayPN(TestEchoArrayPN_Req *reqObj)
{
    TestEchoArrayPN_Req *copy = new TestEchoArrayPN_Req(*reqObj); // Test copy constructor
    TestEchoArrayPN_Req copy2(NULL,
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
    delete copy->_U32a;
    uint32_t arr[2] = {0xab, 0xcd};
    copy->_U32a = new uint32_array(arr,2);
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoArrayPN_Resp *resp = new TestEchoArrayPN_Resp(copy2._U8a,
                                                          copy2._U8aArr,
                                                          copy2.num_U8aArr,
                                                          copy2._U16a,
                                                          copy2._U16aArr,
                                                          copy2.num_U16aArr,
                                                          copy2._U32a,
                                                          copy2._U32aArr,
                                                          copy2.num_U32aArr
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

static BAG *handle_TestEchoTuplePN(TestEchoTuplePN_Req *reqObj)
{
    TestEchoTuplePN_Req *copy = new TestEchoTuplePN_Req(*reqObj); // Test copy constructor
    TestEchoTuplePN_Req copy2(NULL,
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
    delete copy->_StringTup;
    copy->_StringTup = new string_tuple((char*)"hei",(char*)"hopp");
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoTuplePN_Resp *resp = new TestEchoTuplePN_Resp(copy2._StringTup,
                                                          copy2._StringTupArr,
                                                          copy2.num_StringTupArr,
                                                          copy2._IntTup,
                                                          copy2._IntTupArr,
                                                          copy2.num_IntTupArr,
                                                          copy2._StrintTup,
                                                          copy2._StrintTupArr,
                                                          copy2.num_StrintTupArr,
                                                          copy2._Struint8Tup,
                                                          copy2._Struint8TupArr,
                                                          copy2.num_Struint8TupArr
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

static BAG *handle_TestEchoBagPN(TestEchoBagPN_Req *reqObj)
{
    TestEchoBagPN_Req *copy = new TestEchoBagPN_Req(*reqObj); // Test copy constructor
    TestEchoBagPN_Req copy2(NULL,
                            NULL,
                            0
                           );
    copy2 = *copy; // Test assignment operator
    if (copy2 == *copy) // Test comparison operator
        printf("Equal (correct)\n");
    else
        printf("Not equal (ERROR)\n");
    delete copy->_Bag;
    copy->_Bag = new Bag(100,NULL);
    if (copy2 == *copy)
        printf("Equal (ERROR)\n");
    else
        printf("Not equal (correct)\n");
    TestEchoBagPN_Resp *resp = new TestEchoBagPN_Resp(copy2._Bag,
                                                      copy2._BagArr,
                                                      copy2.num_BagArr
                                                     );
    delete copy;
    return resp;
}

