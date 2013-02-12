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
        sint8 Sint8;
        sint16 Sint16;
        sint32 Sint32;
        sint64 Sint64;
        uint8 Uint8;
        uint16 Uint16;
        uint32 Uint32;
        uint64 Uint64;
        # Enum types
        enum Se {
            zero,
            one,
            minus_one = -1
        };
        signed_enum8 Se8 {
            zero,
            one,
            minus_one = -1
        };
        signed_enum16 Se16 {
            zero,
            one,
            minus_one = -1
        };
        signed_enum32 Se32 {
            zero,
            one,
            minus_one = -1
        };
        enum8 Ue8 {
            zero,
            one
        };
        enum16 Ue16 {
            zero,
            one
        };
        enum32 Ue32 {
            zero,
            one
        };
        # Bool types
        bool Bool;
        bool8 Bool8;
        # String types
        string String;
        wstring WString;
        # Addr types:
        addr Addr;
        # Array types
        uint8_array U8a;
        uint16_array U16a;
        uint32_array U32a;
        string_tuple StringTup;
        int_tuple IntTup;
        strint_tuple StrintTup;
        struint8_tuple Struint8Tup;
        bag Bag {
            Int i,
            Bag *b
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
                    inout Sint8 *s8,
                    inout Sint16 *s16,
                    inout Sint32 *s32,
                    inout Sint64 *s64,
                    inout Uint8 *u8,
                    inout Uint16 *u16,
                    inout Uint32 *u32,
                    inout Uint64 *u64
                    );
        TestEchoEnum(inout Se *se,
                    inout Se8 *se8,
                    inout Se16 *se16,
                    inout Se32 *se32,
                    inout Ue8 *ue8,
                    inout Ue16 *ue16,
                    inout Ue32 *ue32
                    );
        TestEchoBool(inout Bool *b,
                    inout Bool8 *b8
                    );
        TestEchoString(inout String *s,
                    inout WString *ws
                    );
        TestEchoAddr(inout Addr a);
        TestEchoArray(inout U8a *u8a,
                    inout U16a *u16a,
                    inout U32a *u32a
                    );
        TestEchoTuple(inout StringTup *stringtup,
                    inout IntTup *inttup,
                    inout StrintTup *strinttup,
                    inout Struint8Tup *struint8tup
                    );
        TestEchoBag(inout Bag b);
    };
};
