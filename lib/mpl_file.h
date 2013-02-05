/*
 *   Copyright 2013 ST-Ericsson SA
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
 *   Author: Per Sigmond <per.xx.sigmond@stericsson.com>
 *   Author: Harald Johansen <harald.johansen@stericsson.com>
 *   Author: Emil B. Viken <emil.b.viken@stericsson.com>
 *
 */

/*************************************************************************
 *
 * File name: mpl_file.h
 *
 * Description: MPL file access API declarations
 *
 **************************************************************************/
#ifndef MPL_FILE_H
#define MPL_FILE_H

/**
 * @file mpl_file.h
 * @brief MPL file handling
 */

/** @defgroup MPL_FILE MPL file handling
 *  @ingroup MPL
 *  The MPL file API is primarily a back-end to the @ref MPL_CONFIG, but it
 *  can also be used separately to save/load parameter lists to the file
 *  system.
 */

/*****************************************************************************
 *
 * Include files
 *
 *****************************************************************************/

#include "stdio.h"
#include "mpl_list.h"
#include "mpl_param.h"

/*****************************************************************************
 *
 * Defines & Type definitions
 *
 *****************************************************************************/


/****************************************************************************
 *
 * Public Functions
 *
 ****************************************************************************/
/**
 * @ingroup MPL_FILE
 * mpl_file_read_params
 *
 * Read parameters from given file
 *
 * @param fp                  file descriptor (must be open for reading)
 * @param param_list_pp       pointer to list
 * @param param_set_id        default parameter set id
 *
 * @return              0 on success, -1 on error
 *
 **/
#define mpl_file_read_params(fp, param_list_pp, param_set_id) \
    mpl_file_read_params_bl(fp, param_list_pp, param_set_id, NULL)

/**
 * @ingroup MPL_FILE
 * mpl_file_read_params_bl
 *
 * Read parameters from given file filtered through
 *                           a blacklist
 *
 * @param fp                  file descriptor (must be open for reading)
 * @param param_list_pp       pointer to list
 * @param param_set_id        default parameter set id
 * @param blacklist      parameters present in the blacklist will be skipped
 *
 * @return              0 on success, -1 on error
 *
 **/
int mpl_file_read_params_bl(FILE *fp,
                            mpl_list_t **param_list_pp,
                            int param_set_id,
                            mpl_blacklist_t blacklist);

/**
 * @ingroup MPL_FILE
 *
 * mpl_file_write_params
 *
 * Write parameters to given file (including the
 *                         parameter set prefix)
 *
 * @param fp                  file descriptor (must be open for writing)
 * @param param_list_p        parameter list
 *
 * @return              0 on success, -1 on error
 *
 **/
#define mpl_file_write_params(fp, param_list_p) \
    mpl_file_write_params_internal(fp, param_list_p, false)

/**
 * @ingroup MPL_FILE
 *
 * mpl_file_write_params_no_prefix
 *
 * Write parameters to given file (omitting
 *                                   the parameter set prefix)
 *
 * @param fp                  file descriptor (must be open for writing)
 * @param param_list_p        parameter list
 *
 * @return              0 on success, -1 on error
 *
 **/
#define mpl_file_write_params_no_prefix(fp, param_list_p) \
    mpl_file_write_params_internal(fp, param_list_p, true)

int mpl_file_write_params_internal(FILE *fp,
                                   mpl_list_t *param_list_p,
                                   bool no_prefix);




#endif
