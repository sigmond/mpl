#
#   Copyright 2013 Per Sigmond <per@sigmond.no>
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

%option parameter_set testprotocol short_name testp;

parameter_set testprotocol {

    prefix tp;

    parameters {

        # Integer types
        int Int;
        int IntArr;
        sint8 Sint8;
        sint8 Sint8Arr;
        sint16 Sint16;
        sint16 Sint16Arr;
        sint32 Sint32;
        sint32 Sint32Arr;
        sint64 Sint64;
        sint64 Sint64Arr;
        uint8 Uint8;
        uint8 Uint8Arr;
        uint16 Uint16;
        uint16 Uint16Arr;
        uint32 Uint32;
        uint32 Uint32Arr;
        uint64 Uint64;
        uint64 Uint64Arr;
        # Enum types
        enum Se {
            zero,
            one,
            minus_one = -1
        };
        enum SeArr {
            zero,
            one,
            minus_one = -1
        };
        signed_enum8 Se8 {
            zero,
            one,
            minus_one = -1
        };
        signed_enum8 Se8Arr {
            zero,
            one,
            minus_one = -1
        };
        signed_enum16 Se16 {
            zero,
            one,
            minus_one = -1
        };
        signed_enum16 Se16Arr {
            zero,
            one,
            minus_one = -1
        };
        signed_enum32 Se32 {
            zero,
            one,
            minus_one = -1
        };
        signed_enum32 Se32Arr {
            zero,
            one,
            minus_one = -1
        };
        enum8 Ue8 {
            zero,
            one
        };
        enum8 Ue8Arr {
            zero,
            one
        };
        enum16 Ue16 {
            zero,
            one
        };
        enum16 Ue16Arr {
            zero,
            one
        };
        enum32 Ue32 {
            zero,
            one
        };
        enum32 Ue32Arr {
            zero,
            one
        };
        # Bool types
        bool MyBool;
        bool BoolArr;
        bool8 Bool8;
        bool8 Bool8Arr;
        # String types
        string String;
        string StringArr;
        wstring WString;
        wstring WStringArr;
        # Addr types:
        addr Addr;
        addr AddrArr;
        # Array types
        uint8_array U8a;
        uint8_array U8aArr;
        uint16_array U16a;
        uint16_array U16aArr;
        uint32_array U32a;
        uint32_array U32aArr;
        string_tuple StringTup;
        string_tuple StringTupArr;
        int_tuple IntTup;
        int_tuple IntTupArr;
        strint_tuple StrintTup;
        strint_tuple StrintTupArr;
        struint8_tuple Struint8Tup;
        struint8_tuple Struint8TupArr;
        bag Bag {
            Int i,
            Bag *b
        };
        bag BagArr {
            Int,
            *Bag
        };
    };
};

category testprot using testprotocol {

    command_bag Req {
    };

    response_bag Resp {
    };

    event_bag Evt {
    };

    commands {
        TestEchoInt(inout Int *i,
                    inout Int *i_arr[],
                    inout Sint8 *s8,
                    inout Sint8 *s8_arr[],
                    inout Sint16 *s16,
                    inout Sint16 *s16_arr[],
                    inout Sint32 *s32,
                    inout Sint32 *s32_arr[],
                    inout Sint64 *s64,
                    inout Sint64 *s64_arr[],
                    inout Uint8 *u8,
                    inout Uint8 *u8_arr[],
                    inout Uint16 *u16,
                    inout Uint16 *u16_arr[],
                    inout Uint32 *u32,
                    inout Uint32 *u32_arr[],
                    inout Uint64 *u64,
                    inout Uint64 *u64_arr[]
                    );
        TestEchoIntPN(inout *Int,
                    inout *IntArr[],
                    inout *Sint8,
                    inout *Sint8Arr[],
                    inout *Sint16,
                    inout *Sint16Arr[],
                    inout *Sint32,
                    inout *Sint32Arr[],
                    inout *Sint64,
                    inout *Sint64Arr[],
                    inout *Uint8,
                    inout *Uint8Arr[],
                    inout *Uint16,
                    inout *Uint16Arr[],
                    inout *Uint32,
                    inout *Uint32Arr[],
                    inout *Uint64,
                    inout *Uint64Arr[]
                    );
        TestEchoEnum(inout Se *se,
                    inout Se *se_arr[],
                    inout Se8 *se8,
                    inout Se8 *se8_arr[],
                    inout Se16 *se16,
                    inout Se16 *se16_arr[],
                    inout Se32 *se32,
                    inout Se32 *se32_arr[],
                    inout Ue8 *ue8,
                    inout Ue8 *ue8_arr[],
                    inout Ue16 *ue16,
                    inout Ue16 *ue16_arr[],
                    inout Ue32 *ue32,
                    inout Ue32 *ue32_arr[]
                    );
        TestEchoEnumPN(inout *Se,
                    inout *SeArr[],
                    inout *Se8,
                    inout *Se8Arr[],
                    inout *Se16,
                    inout *Se16Arr[],
                    inout *Se32,
                    inout *Se32Arr[],
                    inout *Ue8,
                    inout *Ue8Arr[],
                    inout *Ue16,
                    inout *Ue16Arr[],
                    inout *Ue32,
                    inout *Ue32Arr[]
                    );
        TestEchoBool(inout MyBool *b,
                    inout MyBool *b_arr[],
                    inout Bool8 *b8,
                    inout Bool8 *b8_arr[]
                    );
        TestEchoBoolPN(inout *MyBool,
                    inout *BoolArr[],
                    inout *Bool8,
                    inout *Bool8Arr[]
                    );
        TestEchoString(inout String *s,
                    inout String *s_arr[],
                    inout WString *ws,
                    inout WString *ws_arr[]
                    );
        TestEchoStringPN(inout *String,
                    inout *StringArr[],
                    inout *WString,
                    inout *WStringArr[]
                    );
        TestEchoAddr(inout Addr *a,
                    inout Addr *a_arr[]
                    );
        TestEchoAddrPN(inout *Addr,
                    inout *AddrArr[]
                    );
        TestEchoArray(inout U8a *u8a,
                    inout U8a *u8a_arr[],
                    inout U16a *u16a,
                    inout U16a *u16a_arr[],
                    inout U32a *u32a,
                    inout U32a *u32a_arr[]
                    );
        TestEchoArrayPN(inout *U8a,
                    inout *U8aArr[],
                    inout *U16a,
                    inout *U16aArr[],
                    inout *U32a,
                    inout *U32aArr[]
                    );
        TestEchoTuple(inout StringTup *stringtup,
                    inout StringTup *stringtup_arr[],    
                    inout IntTup *inttup,
                    inout IntTup *inttup_arr[],
                    inout StrintTup *strinttup,
                    inout StrintTup *strinttup_arr[],
                    inout Struint8Tup *struint8tup,
                    inout Struint8Tup *struint8tup_arr[]
                    );
        TestEchoTuplePN(inout *StringTup,
                    inout *StringTupArr[],
                    inout *IntTup,
                    inout *IntTupArr[],
                    inout *StrintTup,
                    inout *StrintTupArr[],
                    inout *Struint8Tup,
                    inout *Struint8TupArr[]
                    );
        TestEchoBag(inout Bag *b,
                    inout Bag *b_arr[]
                    );
        TestEchoBagPN(inout *Bag,
                    inout *BagArr[]
                    );
    };
};
