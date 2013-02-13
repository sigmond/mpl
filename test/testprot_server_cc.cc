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
#include <stdio.h>
#include <stdlib.h>

using namespace testprotocol;

int main(int argc, char *argv[])
{
    char buf[1024];
    FILE *fi;
    FILE *fo;

    if (argc > 2) {
        fi = fopen(argv[1], "r");
        if (!fi) {
            fprintf(stderr, "Error opening file '%s' for reading\n", argv[1]);
            exit(-1);
        }
        fo = fopen(argv[2], "w");
        if (!fo) {
            fprintf(stderr, "Error opening file '%s' for writing\n", argv[2]);
            exit(-1);
        }
    }
    else {
        fi = stdin;
        fo = stdout;
    }

    fprintf(stderr, "testprot cc-api server STARTS\n");

    testprotocol_param_init();

    while (fgets(buf, 1024, fi) != NULL) {
        mpl_list_t *req;

        req = mpl_param_list_unpack(buf);
        if (req && TESTPROT_IS_COMMAND(req)) {
            BAG *reqObj = testprot::receive(req);
            BAG *respObj = handle_testprot(reqObj);
            delete reqObj;
            mpl_list_t *resp = NULL;
            if (respObj) {
                resp = testprot::send(respObj);
                delete respObj;
            }
            else {
                fprintf(stderr, "!!! INVALID RESPONSE !!!\n");
                mpl_bag_t *respParams = NULL;
                mpl_add_param_to_list(&resp,
                                      TESTPROT_COMMAND_ID_TO_RESPONSE_ID(TESTPROT_GET_COMMAND_ID(req)),
                                      respParams);
                mpl_param_list_destroy(&respParams);
            }
            if (mpl_param_list_pack(resp, buf, 1023) <= 0) {
                fprintf(stderr, "!!! FAILED PACKING MESSAGE !!!\n");
            }
            mpl_param_list_destroy(&resp);
            if (fprintf(fo, "%s\n", buf) <= 0) {
                fprintf(stderr, "!!! FAILED SENDING MESSAGE !!!\n");
            }
            else {
                printf("%s\n", buf);
            }
            fflush(fo);
        }
        else {
            fprintf(stderr, "!!! INVALID REQUEST !!!\n");
        }
    }

    fprintf(stderr, "testprot cc-api server QUITS\n");
    return 0;
}

