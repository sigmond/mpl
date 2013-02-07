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

#include "personnel.h"
#include "mpl_file.h"
#include "pers_db.h"
#include <assert.h>

static FILE *dbfile;

PERS_ENUM_TYPE(Error) persdb_Add(mpl_bag_t *employee, uint16_t *number)
{
    mpl_list_t *tmp;
    mpl_list_t *dblist = NULL;
    uint16_t new_number = EMPLOYEE_NUMBER_BASE;
    uint16_t max_number = EMPLOYEE_NUMBER_UNDEFINED;
    int ret;

    PERS_ENUM_VAR_DECLARE_INIT(Error, error, success);

    if (PERS_GET_Employee_number(employee) != EMPLOYEE_NUMBER_UNDEFINED) {
        error = PERS_ENUM_VALUE(Error, parameter);
        *number = EMPLOYEE_NUMBER_UNDEFINED;
        goto finish;
    }

    dbfile = fopen(PERSDB_FILENAME, "r");
    if (dbfile) {
        ret = mpl_file_read_params(dbfile, &dblist, PERSONNEL_PARAM_SET_ID);
        fclose(dbfile);
        if (ret) {
            error = PERS_ENUM_VALUE(Error, general);
            *number = EMPLOYEE_NUMBER_UNDEFINED;
            goto finish;
        }
    }
    MPL_LIST_FOR_EACH(dblist, tmp) {
        uint16_t tmpnum;
        mpl_bag_t *tmpempl =
            MPL_LIST_CONTAINER(tmp, mpl_param_element_t, list_entry)->value_p;
        tmpnum = PERS_GET_Employee_number(tmpempl);
        max_number = tmpnum > max_number ? tmpnum : max_number;
    }
    if (max_number != EMPLOYEE_NUMBER_UNDEFINED){
        new_number = max_number + 1;
    }
    PERS_REMOVE_FIELD(employee, Employee, number);
    PERS_ADD_Employee_number(&employee, new_number);
    mpl_add_param_to_list(&dblist,
                          PERS_PARAM_ID(Employee),
                          employee
                         );
    dbfile = fopen(PERSDB_FILENAME, "w");
    if (!dbfile || mpl_file_write_params_no_prefix(dbfile, dblist)) {
        error = PERS_ENUM_VALUE(Error, general);
        *number = EMPLOYEE_NUMBER_UNDEFINED;
        goto finish;
    }
    fclose(dbfile);

finish:
    return error;
}

PERS_ENUM_TYPE(Error) persdb_Get(uint16_t number, mpl_bag_t **employee)
{
    mpl_list_t *tmp;
    mpl_list_t *dblist = NULL;
    int ret;
    PERS_ENUM_VAR_DECLARE_INIT(Error, error, not_found);

    if (number == EMPLOYEE_NUMBER_UNDEFINED) {
        error = PERS_ENUM_VALUE(Error, parameter);
        *employee = NULL;
        goto finish;
    }

    dbfile = fopen(PERSDB_FILENAME, "r");
    if (!dbfile) {
        error = PERS_ENUM_VALUE(Error, general);
        *employee = NULL;
        goto finish;
    }
    else {
        ret = mpl_file_read_params(dbfile, &dblist, PERSONNEL_PARAM_SET_ID);
        fclose(dbfile);
        if (ret) {
            error = PERS_ENUM_VALUE(Error, general);
            *employee = NULL;
            goto finish;
        }
    }
    MPL_LIST_FOR_EACH(dblist, tmp) {
        uint16_t tmpnum;
        mpl_bag_t *tmpempl =
            MPL_LIST_CONTAINER(tmp, mpl_param_element_t, list_entry)->value_p;
        if (PERS_GET_Employee_number(tmpempl) == number){
            *employee = mpl_param_list_clone(tmpempl);
            error = PERS_ENUM_VALUE(Error, success);
            break;
        }
    }

finish:
    return error;
}

PERS_ENUM_TYPE(Error) persdb_Delete(uint16_t number)
{
    mpl_list_t *tmp;
    mpl_list_t *tmp2;
    mpl_list_t *dblist = NULL;
    int ret;
    PERS_ENUM_VAR_DECLARE_INIT(Error, error, not_found);

    if (number == EMPLOYEE_NUMBER_UNDEFINED) {
        error = PERS_ENUM_VALUE(Error, parameter);
        goto finish;
    }

    dbfile = fopen(PERSDB_FILENAME, "r");
    if (!dbfile) {
        error = PERS_ENUM_VALUE(Error, general);
        goto finish;
    }
    else {
        ret = mpl_file_read_params(dbfile, &dblist, PERSONNEL_PARAM_SET_ID);
        fclose(dbfile);
        if (ret) {
            error = PERS_ENUM_VALUE(Error, general);
            goto finish;
        }
    }
    MPL_LIST_FOR_EACH_SAFE(dblist, tmp, tmp2) {
        uint16_t tmpnum;
        mpl_bag_t *tmpempl =
            MPL_LIST_CONTAINER(tmp, mpl_param_element_t, list_entry)->value_p;
        if (PERS_GET_Employee_number(tmpempl) == number){
            mpl_list_remove(&dblist, tmp);
            error = PERS_ENUM_VALUE(Error, success);
            break;
        }
    }

    dbfile = fopen(PERSDB_FILENAME, "w");
    if (!dbfile || mpl_file_write_params_no_prefix(dbfile, dblist)) {
        error = PERS_ENUM_VALUE(Error, general);
        goto finish;
    }
    fclose(dbfile);

finish:
    return error;
}

PERS_ENUM_TYPE(Error) persdb_Find(char *first,
                                  char *middle,
                                  char *last,
                                  mpl_list_t **employees)
{
    mpl_list_t *tmp;
    mpl_list_t *dblist = NULL;
    int ret;
    PERS_ENUM_VAR_DECLARE_INIT(Error, error, success);

    dbfile = fopen(PERSDB_FILENAME, "r");
    if (!dbfile) {
        error = PERS_ENUM_VALUE(Error, general);
        *employees = NULL;
        goto finish;
    }
    else {
        ret = mpl_file_read_params(dbfile, &dblist, PERSONNEL_PARAM_SET_ID);
        fclose(dbfile);
        if (ret) {
            error = PERS_ENUM_VALUE(Error, general);
            *employees = NULL;
            goto finish;
        }
    }
    MPL_LIST_FOR_EACH(dblist, tmp) {
        mpl_bag_t *name;
        mpl_bag_t *tmpempl =
            MPL_LIST_CONTAINER(tmp, mpl_param_element_t, list_entry)->value_p;
        name = PERS_GET_Employee_name_PTR(tmpempl);
        if (first)
            if ((PERS_Name_first_EXISTS(name) &&
                 strcmp(PERS_GET_Name_first_PTR(name), first)) ||
                !PERS_Name_first_EXISTS(name))
                continue;
        if (middle)
            if ((PERS_Name_middle_EXISTS(name) &&
                 strcmp(PERS_GET_Name_middle_PTR(name), middle)) ||
                !PERS_Name_middle_EXISTS(name))
                continue;
        if (last)
            if ((PERS_Name_last_EXISTS(name) &&
                 strcmp(PERS_GET_Name_last_PTR(name), last)) ||
                !PERS_Name_last_EXISTS(name))
                continue;
        mpl_add_param_to_list(employees,
                              PERS_PARAM_ID(Employee),
                              mpl_param_list_clone(tmpempl)
                             );
    }

finish:
    return error;
}

