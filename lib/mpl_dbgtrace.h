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
 *   Author: Per Sigmond <per@sigmond.no>
 *   Author: Harald Johansen <harald.johansen@stericsson.com>
 *   Author: Emil B. Viken <emil.b.viken@stericsson.com>
 *
 */

/*************************************************************************
 *
 * File name: mpl_dbgtrace.h
 *
 * Description: MPL debugtrace declarations
 *
 **************************************************************************/
#ifndef MPL_DBGTRACE_H
#define MPL_DBGTRACE_H


/*****************************************************************************
 *
 * Include files
 *
 ********************************************************************************/

#include <string.h>
#include <stdio.h>

/*****************************************************************************
 *
 * Defines & Type definitions
 *
 *****************************************************************************/


extern const char *MPL_ErrorCodeNames[];

#define MPL_DBG_TRACE_ERROR(E_CODE,E_INFO)                              \
  do                                                                    \
  {                                                                     \
    char *file_p = "/"__FILE__;                                         \
    file_p = strrchr(file_p,'/');                                       \
    if (file_p != NULL)                                                 \
        file_p++;                                                       \
    printf("%s:%d ERROR! %s ",                                          \
           file_p!=NULL?file_p:"",__LINE__,MPL_ErrorCodeNames[E_CODE]); \
    printf E_INFO;                                                      \
  } while(0)

#define MPL_DBG_ERROR_CODES                                 \
  /* 0  */ MPL_ERROR_CODE_ELEMENT(FAILED_ALLOCATING_MEMORY) \
  /* 1  */ MPL_ERROR_CODE_ELEMENT(INVALID_OPERATION)        \
  /* 2  */ MPL_ERROR_CODE_ELEMENT(FAILED_OPERATION)         \
  /* 17 */ MPL_ERROR_CODE_ELEMENT(INVALID_PARAMETER)

#define MPL_ERROR_CODE_ELEMENT(CODE) E_MPL_##CODE,
typedef enum
{
  MPL_DBG_ERROR_CODES
  MPL_ERROR_NUMBER_OF_ERROR_CODES
} MPL_ErrorCode_t;
#undef MPL_ERROR_CODE_ELEMENT


/****************************************************************************
 *
 * Public Functions
 *
 ****************************************************************************/
void mpl_debugtrace_init(void);

#endif
