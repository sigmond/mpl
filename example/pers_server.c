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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "personnel.h"
#include "pers_handlers.h"
#include <assert.h>

void usage() 
{
    printf("Usage: pers_server options\n");
    printf("    -h Show command usage\n");
    printf("    -i input pipe\n");
    printf("    -o output pipe\n");
}

int main(int argc, char *argv[])
{
    char buf[1024];
    int opt;
    char *pi = NULL;
    char *po = NULL;
    FILE *fi;
    FILE *fo;
    int i;

    if (argc < 2) {
        usage();
        exit(-1);
    }

    printf("PERS SERVER started with arguments:");
    for (i = 0 ; i < argc; i++)
        printf(" %s", argv[i]);
    printf("\n");
    while (-1 != (opt = getopt(argc, argv, "i:o:h"))) {
        switch (opt) {
            case 'h':
                usage();
                exit(-1);
            case 'i':
                pi = optarg;
                break;
            case 'o':
                po = optarg;
                break;
            default:
                printf("unsupported option received\n");
                exit(-1);
        }
    }

    if (!pi) {
        fprintf(stderr, "Input pipe not specified\n");
        exit(-1);
    }
    
    if (!po) {
        fprintf(stderr, "Output pipe not specified\n");
        exit(-1);
    }

    if (!strcmp(pi, "-")) {
        fi = stdin;
    }
    else {
        fi = fopen(pi, "r");
        if (!fi) {
            fprintf(stderr, "Error opening file '%s' for reading\n", pi);
            exit(-1);
        }
    }

    if (!strcmp(po, "-")) {
        fo = stdout;
    }
    else {
        fo = fopen(po, "w");
        if (!fo) {
            fprintf(stderr, "Error opening file '%s' for writing\n", po);
            exit(-1);
        }
    }

    fprintf(stderr, "persfile server STARTS\n");

    personnel_param_init();

    while (fgets(buf, 1024, fi) != NULL) {
        mpl_list_t *req;
        mpl_list_t *resp;

        req = mpl_param_list_unpack(buf);
        if (req) {
            resp = handle_persfile(req);
            if (resp) {
                if (mpl_param_list_pack(resp, buf, 1023) <= 0) {
                    fprintf(stderr, "!!! FAILED PACKING MESSAGE !!!\n");
                }
                mpl_param_list_destroy(&resp);
                if (fprintf(fo, "%s\n", buf) <= 0) {
                    fprintf(stderr, "!!! FAILED SENDING MESSAGE !!!\n");
                }
                fflush(fo);
            }
            else {
                fprintf(stderr, "!!! INVALID RESPONSE !!!\n");
            }
            mpl_param_list_destroy(&req);

        }
        else {
            fprintf(stderr, "!!! INVALID REQUEST !!!\n");
        }
    }

    if (fi != stdin)
        fclose(fi);
    if ((fo != stdout) && (fo != stderr))
        fclose(fo);
    mpl_param_system_deinit();
    fprintf(stderr, "persfile server QUITS\n");
    return 0;
}
