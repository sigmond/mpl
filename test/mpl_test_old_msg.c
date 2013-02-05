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
/*
 * Test for message definitions
 *
 */


/********************************************************************************
 *
 * Include files
 *
 ********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <string.h>
#include "mpl_test_old_msg.h"


/********************************************************************************
 *
 * Defines & Type definitions
 *
 ********************************************************************************/

/********************************************************************************
 *
 * Global  variables
 *
 ********************************************************************************/
const char mpl_test_old_event_failure_buf[] = "message=event_failure";


/********************************************************************************
 *
 * Local variables
 *
 ********************************************************************************/

#define MPL_TEST_OLD_MYENUM_ELEMENT(LEVEL) #LEVEL,
static const char* mpl_test_old_names_myenum[] =
{
    MPL_TEST_OLD_MYENUM
};
#undef MPL_TEST_OLD_MYENUM_ELEMENT

/********************************************************************************
 *
 * Private function prototypes
 *
 ********************************************************************************/

/* Dummies */
static const char *mpl_test_old_names_dummy[] = {""};
#define mpl_test_old_number_of_dummy 0
#define mpl_test_old_dummy_t

#define MPL_TEST_OLD_PARAMETER_ID_ELEMENT(ELEMENT, TYPE, EXTRA, MAX, SET, GET, CONFIG, DEFAULT) \
  const mpl_paramset_pre_##TYPE mpl_test_old_ ##EXTRA ##_t  mpl_test_old_default_##ELEMENT mpl_paramset_post_##TYPE = \
      mpl_paramset_set_##TYPE ##_ ##DEFAULT; \
  const uint64_t mpl_test_old_max_##ELEMENT = mpl_paramset_set_##MAX;
MPL_TEST_OLD_PARAMETER_IDS
#undef MPL_TEST_OLD_PARAMETER_ID_ELEMENT

#define MPL_TEST_OLD_PARAMETER_ID_ELEMENT(ELEMENT, TYPE, EXTRA, MAX, SET, GET, CONFIG, DEFAULT) \
    {                                                               \
        #ELEMENT,                                                   \
        mpl_type_##TYPE,                                            \
        (SET),                                                      \
        (GET),                                                      \
        (CONFIG),                                                   \
        (mpl_paramset_is_##DEFAULT )?&mpl_test_old_default_##ELEMENT :NULL, \
        (mpl_paramset_is_##MAX)?&mpl_test_old_max_##ELEMENT : NULL, \
        mpl_pack_param_value_##TYPE,                                \
        mpl_unpack_param_value_##TYPE,                              \
        mpl_clone_param_value_##TYPE,                               \
        mpl_copy_param_value_##TYPE,                                \
        mpl_compare_param_value_##TYPE,                             \
        mpl_sizeof_param_value_##TYPE,                              \
        mpl_free_param_value_##TYPE,                                \
        mpl_test_old_names_##EXTRA,                                 \
        ARRAY_SIZE(mpl_test_old_names_##EXTRA) \
    },
const mpl_param_descr_t mpl_test_old_param_descr[] =
{
  MPL_TEST_OLD_PARAMETER_IDS
};
#undef MPL_TEST_OLD_PARAMETER_ID_ELEMENT

MPL_DEFINE_PARAM_DESCR_SET(mpl_test_old, MPL_TEST_OLD);

mpl_param_descr_set_t *mpl_test_old_param_descr_set_external_p = &mpl_test_old_param_descr_set;


/*******************************************************************************
 *
 * Public Functions
 *
 *******************************************************************************/

mpl_config_t mpl_test_old_config;


/**
 * test_init - Initiate library
 **/
int mpl_test_old_init(void)
{
    return mpl_param_init(&mpl_test_old_param_descr_set);
}



/*******************************************************************************
 *
 * Private Functions
 *
 *******************************************************************************/


