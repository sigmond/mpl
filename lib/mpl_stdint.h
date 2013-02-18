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
 *   Author: Harald Johansen <hajohans1@gmail.com>
 *   Author: Emil B. Viken <emil.b.viken@gmail.com>
 *
 */

/*************************************************************************
 *
 * File name: mpl_stdint.h
 *
 * Description: MPL stdint porting file
 *
 **************************************************************************/
#ifndef MPL_STDINT_H
#define MPL_STDINT_H

/*
 * ======================
 * ===    Includes    ===
 * ======================
 */

#include <stdint.h>
#include <limits.h>

#ifndef __sint8_t_defined
#define __sint8_t_defined
typedef int8_t sint8_t;
#endif

#ifndef __sint16_t_defined
#define __sint16_t_defined
typedef int16_t sint16_t;
#endif

#ifndef __sint32_t_defined
#define __sint32_t_defined
typedef int32_t sint32_t;
#endif

#ifndef __sint64_t_defined
#define __sint64_t_defined
typedef int64_t sint64_t;
#endif

#endif
