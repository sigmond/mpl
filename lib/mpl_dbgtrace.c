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
 * File name: mpl_dbgtrace.c
 *
 * Description: MPL debugtrace implementation
 *
 **************************************************************************/


/*****************************************************************************
 *
 * Include files
 *
 *****************************************************************************/
#include "mpl_dbgtrace.h"
//#include "t_basicdefinitions.h"

#if !defined(UNIT_TEST) && (MPL_INCLUDE_TV_LOGGING==1)
#include "r_log_rmm_macros_c.h"
#include "mpl_tv_print.def"
#include "r_log_rmm_macros_c_init.h"
#endif

/*****************************************************************************
 *
 * Defines & Type definitions
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Global variables
 *
 *****************************************************************************/
#define MPL_ERROR_CODE_ELEMENT(CODE) #CODE,
const char *MPL_ErrorCodeNames[] =
{
  MPL_DBG_ERROR_CODES
};
#undef MPL_ERROR_CODE_ELEMENT

/*****************************************************************************
 *
 * Local variables
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Private function prototypes
 *
 *****************************************************************************/

/****************************************************************************
 *
 * Public Functions
 *
 ****************************************************************************/

void mpl_debugtrace_init(void)
{
#if !defined(UNIT_TEST) && (MPL_INCLUDE_TV_LOGGING==1)
  #include "mpl_tv_print.def"
#endif
}

/****************************************************************************
 *
 * Private Functions
 *
 ****************************************************************************/