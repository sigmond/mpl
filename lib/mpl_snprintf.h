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
 * File name: mpl_snprintf.h
 *
 * Description: MPL snprintf porting file
 *
 **************************************************************************/
#ifndef MPL_SNPRINTF_H
#define MPL_SNPRINTF_H

/*
 * ======================
 * ===    Includes    ===
 * ======================
 */

#include <stdio.h>

#if MPL_USE_MVS_WIN32_COMPILER
#define SNPRINTF _snprintf
#else
#define SNPRINTF snprintf
#endif

#endif
