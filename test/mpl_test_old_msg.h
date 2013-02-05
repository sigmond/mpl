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
 * Test for old style paremeter description definitions
 *
 */
#ifndef MPL_TEST_OLD_H
#define MPL_TEST_OLD_H

#include "mpl_param.h"
#include "mpl_config.h"

#ifdef  __cplusplus
extern "C" {
#endif


#define MPL_TEST_OLD_PARAM_SET_ID 6348
#define MPL_TEST_OLD_PARAMID_PREFIX "mpl_test_old"

extern mpl_config_t mpl_test_old_config;
extern mpl_param_descr_set_t *mpl_test_old_param_descr_set_external_p;

extern const char mpl_test_old_event_failure_buf[];

#define MPL_TEST_OLD_PARAMETER_IDS                                      \
  /* Parameter description:         Name       Type    Enum      MAX,       Set?   Get?   Config? Default Value */ \
  MPL_TEST_OLD_PARAMETER_ID_ELEMENT(myfirst,   int,    dummy,    no_max,    false, false, false,  no_default) \
  MPL_TEST_OLD_PARAMETER_ID_ELEMENT(mystring,  string, dummy,    max(20),   true,  true,  true,   default("mydefault")) \
  MPL_TEST_OLD_PARAMETER_ID_ELEMENT(myint,     int,    dummy,    max(1000), false, false, true,   default(99)) \
  MPL_TEST_OLD_PARAMETER_ID_ELEMENT(mysint8,   sint8,  dummy,    max(100),  false, false, true,   default(99)) \
  MPL_TEST_OLD_PARAMETER_ID_ELEMENT(mysint16,  sint16, dummy,    max(100),  false, false, true,   default(99)) \
  MPL_TEST_OLD_PARAMETER_ID_ELEMENT(mysint32,  sint32, dummy,    max(100),  false, false, true,   default(99)) \
  MPL_TEST_OLD_PARAMETER_ID_ELEMENT(myuint8,   uint8,  dummy,    max(100),  false, false, true,   default(99)) \
  MPL_TEST_OLD_PARAMETER_ID_ELEMENT(myuint16,  uint16, dummy,    max(100),  false, false, true,   default(99)) \
  MPL_TEST_OLD_PARAMETER_ID_ELEMENT(myuint32,  uint32, dummy,    max(100),  false, false, true,   default(99)) \
  MPL_TEST_OLD_PARAMETER_ID_ELEMENT(myuint64,  uint64, dummy,    max(100),  false, false, true,   default(99)) \
  MPL_TEST_OLD_PARAMETER_ID_ELEMENT(myenum,    enum,   myenum,   no_max,    false, false, false,  default(mpl_test_old_myenum_val1)) \
  MPL_TEST_OLD_PARAMETER_ID_ELEMENT(mylast,    int,    dummy,    no_max,    false, false, false,  no_default) \

#define MPL_TEST_OLD_PARAMETER_ID_ELEMENT(ELEMENT, TYPE, EXTRA, MAX, SET, GET, CONFIG, DEFAULT) \
  mpl_test_old_paramid_##ELEMENT,
typedef enum
{
  mpl_test_old_paramid_base = MPL_PARAM_SET_ID_TO_PARAMID_BASE(MPL_TEST_OLD_PARAM_SET_ID),
  MPL_TEST_OLD_PARAMETER_IDS
  mpl_test_old_end_of_paramids
} mpl_test_old_test_paramid_t;
#undef MPL_TEST_OLD_PARAMETER_ID_ELEMENT


#define MPL_TEST_OLD_MYENUM                    \
    MPL_TEST_OLD_MYENUM_ELEMENT(val1)          \
    MPL_TEST_OLD_MYENUM_ELEMENT(val2)

#define MPL_TEST_OLD_MYENUM_ELEMENT(LEVEL) mpl_test_old_myenum_##LEVEL,
typedef enum
{
    MPL_TEST_OLD_MYENUM
    mpl_test_old_number_of_myenum
} mpl_test_old_myenum_t;
#undef MPL_TEST_OLD_MYENUM_ELEMENT


int mpl_test_old_init(void);

#ifdef  __cplusplus
}
#endif

#endif /* TEST_MSG_H */
