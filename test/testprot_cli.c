#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "testprotocol.h"
#include "testprotocol_cli.h"

#include "linenoise.h"

static char **completionStrings = NULL;
static int maxStrings = 1024;

FILE *fi;
FILE *fo;

static int checkCommand(mpl_list_t *cmdMsg);

static int pack_and_send(mpl_list_t *msg)
{
    char *buf = NULL;
    int len;
    int ret = -1;

    len = mpl_param_list_pack(msg, NULL, 0);
    if (len <= 0) {
        printf("!!! FAILED PACKING MESSAGE !!!\n");
        goto exit;
    }
    buf = malloc(len+1);
    len = mpl_param_list_pack(msg, buf, len+1);
    if (len <= 0) {
        printf("!!! FAILED PACKING MESSAGE !!!\n");
        goto exit;
    }

    if (fprintf(fo, "%s\n", buf) <= 0) {
        fprintf(stderr, "!!! FAILED SENDING MESSAGE !!!\n");
    }
    fflush(fo);
    ret = 0;

  exit:
    free(buf);
    return ret;
}

static void completion(const char *buf, linenoiseCompletions *lc) {
    int numCompletionStrings;
    int i;
    int completionPosIndex;

    if (!strncmp(buf, "quit", strlen(buf))) {
        linenoiseAddCompletion(lc,"quit");
    }
    if (!strncmp(buf, "help", strlen(buf))) {
        linenoiseAddCompletion(lc,"help");
    }

    completionPosIndex = 0;
    numCompletionStrings = testprot_get_command_completions(buf, &completionPosIndex, completionStrings, maxStrings);

    for (i = 0; i < numCompletionStrings; i++) {
        linenoiseAddCompletion(lc,completionStrings[i]);
        free(completionStrings[i]);
    }
}

static char buf[1024];

static int readlines(void) {
    char *line;
    mpl_list_t *reqMsg = NULL;
    int quit = 0;
    mpl_list_t *tmp;

    completionStrings = calloc(maxStrings, sizeof(char *));

    while((line = linenoise("TESTPROT> ")) != NULL) {
        if (line[0] != '\0') {
            if (strcmp(line,"quit") && strncmp(line, "help ", 5)) {
                char *req;
                int ret = -1;
                req = formatCmdLine(line, "Req");
                reqMsg = mpl_param_list_unpack_param_set(req, TESTPROTOCOL_PARAM_SET_ID);
                if (!reqMsg) {
                    printf("Protocol error\n");
                    ret = -1;
                }
                else if (checkCommand(reqMsg) == 0) {
                    ret = pack_and_send(reqMsg);
                }
                mpl_param_list_destroy(&reqMsg);
                free(req);
                if (ret == 0) {
                    fgets(buf, 1024, fi);
                    printf("%s\n", buf);
                }
            }
            else if (!strncmp(line, "help ", 5)) {
                char *helptext;
                testprot_get_command_help(line, &helptext);
                if (helptext != NULL) {
                    printf("%s", helptext);
                    free(helptext);
                }
                else
                    printf("No help\n");
            }
            else {
                quit = 1;
            }
            linenoiseHistoryAdd(line);
            linenoiseHistorySave("history.txt"); /* Save every new entry */
            free(line);
            while ((tmp = mpl_list_remove(&completionCallstack, NULL)) != NULL) {
                completion_callstack_entry_t *e;
                e = MPL_LIST_CONTAINER(tmp, completion_callstack_entry_t, list_entry);
                free(e);
            }
        }
        if (quit)
            break;
    }
    free(completionStrings);
    return 0;
}

int main(int argc, char **argv)
{
    int i;
    int opt;
    char *pi = NULL;
    char *po = NULL;

    printf("TESTPROT CLI started with arguments:");
    for (i = 0 ; i < argc; i++)
        printf(" %s", argv[i]);
    printf("\n");
    while (-1 != (opt = getopt(argc, argv, "i:o:h"))) {
        switch (opt) {
        case 'h':
            printf("Usage: testprot_cli <options>\n");
            printf("    -h Show command usage\n");
            printf("    -i input pipe\n");
            printf("    -o output pipe\n");
            return 0;
        case 'i':
            pi = optarg;
            break;
        case 'o':
            po = optarg;
            break;
        default:
            printf("unsupported option received\n");
            return -1;
        }
    }

    if (po) {
        fo = fopen(po, "w");
        if (!fo) {
            fprintf(stderr, "Error opening file '%s' for writing\n", po);
            exit(-1);
        }
    }
    else {
        fo = stdout;
    }

    if (pi) {
        fi = fopen(pi, "r");
        if (!fi) {
            fprintf(stderr, "Error opening file '%s' for reading\n", pi);
            exit(-1);
        }
    }
    else {
        fi = stdin;
    }

    if (testprotocol_param_init())
    {
        printf("testprotocol_param_init() failed\n");
        return -1;
    }
    linenoiseSetCompletionCallback(completion);
    linenoiseHistoryLoad("history.txt"); /* Load the history at startup */


    /* Loop reading stdin */
    readlines();


    printf("Halting the CLI\n");

exit:
    mpl_param_deinit();

    printf("DONE\n");
    return 0;
}

static int checkCommand(mpl_list_t *cmdMsg)
{
    char *buf = NULL;
    int len;
    mpl_param_element_t *elem_p;
    mpl_list_t *check_result_list_p = NULL;
    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT;
    const char *prompt = "Protocol error: ";
    options.force_field_pack_mode = true;

    elem_p = mpl_param_list_find(TESTP_PARAM_ID(Req), cmdMsg);
    if (elem_p == NULL) {
        printf("%sno command\n", prompt);
        return -1;
    }
    if (testprotocol_checkBag_Req(elem_p, &check_result_list_p)) {
        len = mpl_param_list_pack_extended(check_result_list_p,
                                           NULL,
                                           0,
                                           &options);
        buf = calloc(1, len + 1);
        strcat(buf, prompt);
        if (buf != NULL) {
            (void)mpl_param_list_pack_extended(check_result_list_p,
                                               buf,
                                               len+1,
                                               &options);
            printf("%s%s\n", prompt, buf);
            free(buf);
        }
        else {
            printf("%smemory error\n", prompt);
        }
        return -1;
    }
    return 0;
}
