#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "personnel.h"
#include "personnel.hh"

using namespace personnel;

void usage() 
{
    printf("Usage: pers_cc_api options\n");
    printf("    -h Show command usage\n");
    printf("    -i input pipe\n");
    printf("    -o output pipe\n");
}

int main(int argc, char **argv)
{
    int i;
    int opt;
    char *pi = NULL;
    char *po = NULL;
    FILE *fi;
    FILE *fo;

    if (argc < 2) {
        usage();
        exit(-1);
    }

    printf("CC API DEMO started with arguments:");
    for (i = 0 ; i < argc; i++)
        printf(" %s", argv[i]);
    printf("\n");
    while (-1 != (opt = getopt(argc, argv, "i:o:h"))) {
        switch (opt) {
        case 'h':
            usage();
            return -1;
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

    if (!pi) {
        fprintf(stderr, "Input pipe not specified\n");
        exit(-1);
    }
    
    if (!po) {
        fprintf(stderr, "Output pipe not specified\n");
        exit(-1);
    }

    fo = fopen(po, "w");
    if (!fo) {
        fprintf(stderr, "Error opening file '%s' for writing\n", po);
        exit(-1);
    }

    fi = fopen(pi, "r");
    if (!fi) {
        fprintf(stderr, "Error opening file '%s' for reading\n", pi);
        exit(-1);
    }

    if (personnel_param_init())
    {
        printf("personnel_param_init() failed\n");
        return -1;
    }

    ////////
    // ADD #1
    // Note: no error checking in this demo...
    printf("ADD REQUEST/RESPONSE #1:\n");
    // Build objects
    Name myName((char*) "Per", NULL, (char*) "Sigmond");
    Date myDateOfBirth(1961, 3, 5);
    Date myHireDate(2013, 2, 21);
    Employee *me = new Employee(&myName,
                                &myDateOfBirth,
                                NULL,
                                personnel_Gender_male,
                                0,
                                &myHireDate,
                                NULL,
                                NULL
                               );
    void *userData = (void*)0xabbababe;
    Add_Req *addReqObj = new Add_Req(userData, me);
    delete me;

    // Generate message
    mpl_list_t *addReqMsg = persfile::send(addReqObj);
    delete addReqObj;

    // Pack and send message
    char buf[1024];
    mpl_param_list_pack(addReqMsg, buf, 1023);
    mpl_param_list_destroy(&addReqMsg);
    fprintf(fo, "%s\n", buf);
    printf("Sent: %s\n", buf);
    fflush(fo);
    
    // Receive response, unpack it and obtain object
    fgets(buf, 1024, fi);
    printf("Received: %s", buf);
    mpl_list_t *addRespMsg = mpl_param_list_unpack(buf);
    Add_Resp *addRespObj = (Add_Resp*) persfile::receive(addRespMsg);
    mpl_param_list_destroy(&addRespMsg);
    uint16_t myNumber = addRespObj->number;
    delete addRespObj;
    printf("Employee added with number=%d\n", myNumber);
    // ADD #1 end
    ////////

    ////////
    // ADD #2
    // Note: no error checking in this demo...
    printf("ADD REQUEST/RESPONSE #2:\n");
    // Build objects
    Name hisName((char*) "Buster", NULL, (char*) "Minal");
    Date hisDateOfBirth(1991, 7, 4);
    Date hisHireDate(2011, 8, 1);
    Employee *him = new Employee(&hisName,
                                 &hisDateOfBirth,
                                 NULL,
                                 personnel_Gender_male,
                                 0,
                                 &hisHireDate,
                                 NULL,
                                 NULL
                                );
    userData = (void*)0xabbababe;
    addReqObj = new Add_Req(userData, him);
    delete him;

    // Generate message
    addReqMsg = persfile::send(addReqObj);
    delete addReqObj;

    // Pack and send message
    mpl_param_list_pack(addReqMsg, buf, 1023);
    mpl_param_list_destroy(&addReqMsg);
    fprintf(fo, "%s\n", buf);
    printf("Sent: %s\n", buf);
    fflush(fo);
    
    // Receive response, unpack it and obtain object
    fgets(buf, 1024, fi);
    printf("Received: %s", buf);
    addRespMsg = mpl_param_list_unpack(buf);
    addRespObj = (Add_Resp*) persfile::receive(addRespMsg);
    mpl_param_list_destroy(&addRespMsg);
    uint16_t hisNumber = addRespObj->number;
    delete addRespObj;
    printf("Employee added with number=%d\n", hisNumber);
    // ADD #2 end
    ////////


    ////////
    // GET #1
    // Note: no error checking in this demo...
    printf("GET REQUEST/RESPONSE #1:\n");
    // Build objects
    Get_Req *getReqObj = new Get_Req(userData, myNumber);

    // Generate message
    mpl_list_t *getReqMsg = persfile::send(getReqObj);
    delete getReqObj;

    // Pack and send message
    mpl_param_list_pack(getReqMsg, buf, 1023);
    mpl_param_list_destroy(&getReqMsg);
    fprintf(fo, "%s\n", buf);
    printf("Sent: %s\n", buf);
    fflush(fo);
    
    // Receive response, unpack it and obtain object
    fgets(buf, 1024, fi);
    printf("Received: %s", buf);
    mpl_list_t *getRespMsg = mpl_param_list_unpack(buf);
    Get_Resp *getRespObj = (Get_Resp*) persfile::receive(getRespMsg);
    mpl_param_list_destroy(&getRespMsg);

    printf("Name: %s %s\n", getRespObj->employee->name->first, getRespObj->employee->name->last);
    delete getRespObj;
    // GET #1 end
    ////////

    ////////
    // GET #2
    // Note: no error checking in this demo...
    printf("GET REQUEST/RESPONSE #2:\n");
    // Build objects
    getReqObj = new Get_Req(userData, hisNumber);

    // Generate message
    getReqMsg = persfile::send(getReqObj);
    delete getReqObj;

    // Pack and send message
    mpl_param_list_pack(getReqMsg, buf, 1023);
    mpl_param_list_destroy(&getReqMsg);
    fprintf(fo, "%s\n", buf);
    printf("Sent: %s\n", buf);
    fflush(fo);
    
    // Receive response, unpack it and obtain object
    fgets(buf, 1024, fi);
    printf("Received: %s", buf);
    getRespMsg = mpl_param_list_unpack(buf);
    getRespObj = (Get_Resp*) persfile::receive(getRespMsg);
    mpl_param_list_destroy(&getRespMsg);

    printf("Name: %s %s\n", getRespObj->employee->name->first, getRespObj->employee->name->last);
    delete getRespObj;
    // GET #2 end
    ////////


    ////////
    // FIND
    // Note: no error checking in this demo...
    printf("FIND REQUEST/RESPONSE:\n");
    // Build objects
    Find_Req *findReqObj = new Find_Req(userData, NULL, NULL, NULL);

    // Generate message
    mpl_list_t *findReqMsg = persfile::send(findReqObj);
    delete findReqObj;

    // Pack and send message
    mpl_param_list_pack(findReqMsg, buf, 1023);
    mpl_param_list_destroy(&findReqMsg);
    fprintf(fo, "%s\n", buf);
    printf("Sent: %s\n", buf);
    fflush(fo);
    
    // Receive response, unpack it and obtain object
    fgets(buf, 1024, fi);
    printf("Received: %s", buf);
    mpl_list_t *findRespMsg = mpl_param_list_unpack(buf);
    Find_Resp *findRespObj = (Find_Resp*) persfile::receive(findRespMsg);
    mpl_param_list_destroy(&findRespMsg);

    for (int i = 0; i < findRespObj->num_employees; i++) {
        printf("Name #%d: %s %s\n", i+1, findRespObj->employees[i]->name->first, findRespObj->employees[i]->name->last);
    }
    delete findRespObj;
    // FIND end
    ////////

    ////////
    // DELETE #1
    // Note: no error checking in this demo...
    printf("DELETE REQUEST/RESPONSE #1:\n");
    // Build objects
    Delete_Req *deleteReqObj = new Delete_Req(userData, myNumber);

    // Generate message
    mpl_list_t *deleteReqMsg = persfile::send(deleteReqObj);
    delete deleteReqObj;

    // Pack and send message
    mpl_param_list_pack(deleteReqMsg, buf, 1023);
    mpl_param_list_destroy(&deleteReqMsg);
    fprintf(fo, "%s\n", buf);
    printf("Sent: %s\n", buf);
    fflush(fo);
    
    // Receive response, unpack it and obtain object
    fgets(buf, 1024, fi);
    printf("Received: %s", buf);
    mpl_list_t *deleteRespMsg = mpl_param_list_unpack(buf);
    Delete_Resp *deleteRespObj = (Delete_Resp*) persfile::receive(deleteRespMsg);
    mpl_param_list_destroy(&deleteRespMsg);

    printf("Result: %s\n", PERS_ENUM_VAR_TO_STRING_PTR(Error, deleteRespObj->error));
    delete deleteRespObj;
    // DELETE #1 end
    ////////

    ////////
    // DELETE #2
    // Note: no error checking in this demo...
    printf("DELETE REQUEST/RESPONSE #2:\n");
    // Build objects
    deleteReqObj = new Delete_Req(userData, hisNumber);

    // Generate message
    deleteReqMsg = persfile::send(deleteReqObj);
    delete deleteReqObj;

    // Pack and send message
    mpl_param_list_pack(deleteReqMsg, buf, 1023);
    mpl_param_list_destroy(&deleteReqMsg);
    fprintf(fo, "%s\n", buf);
    printf("Sent: %s\n", buf);
    fflush(fo);
    
    // Receive response, unpack it and obtain object
    fgets(buf, 1024, fi);
    printf("Received: %s", buf);
    deleteRespMsg = mpl_param_list_unpack(buf);
    deleteRespObj = (Delete_Resp*) persfile::receive(deleteRespMsg);
    mpl_param_list_destroy(&deleteRespMsg);

    printf("Result: %s\n", PERS_ENUM_VAR_TO_STRING_PTR(Error, deleteRespObj->error));
    delete deleteRespObj;
    // DELETE #2 end
    ////////

    printf("Halting the CC API DEMO\n");

    mpl_param_system_deinit();

    printf("DONE\n");
    return 0;
}
