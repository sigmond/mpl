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
 * File name: mpl_pthread.h
 *
 * Description: MPL posix thread porting file
 *
 **************************************************************************/
#ifndef MPL_PTHREAD_H
#define MPL_PTHREAD_H

/*
 * ======================
 * ===    Includes    ===
 * ======================
 */

#if defined(MPL_USE_PTHREAD_MUTEX)

#include <pthread.h>

#define mpl_mutex_t pthread_mutex_t

#define mpl_threads_init()
#define mpl_threads_deinit()

int mpl_mutex_init(mpl_mutex_t **mutex_pp);
int mpl_mutex_destroy(mpl_mutex_t *mutex_p);
#define mpl_mutex_lock(mutex_p) pthread_mutex_lock(mutex_p)
#define mpl_mutex_unlock(mutex_p) pthread_mutex_unlock(mutex_p)
#define mpl_thread_t pthread_t
#define mpl_get_current_thread_id pthread_self

#elif defined(MPL_USE_OSE_MUTEX)

#include "r_os.h"

#define mpl_mutex_t MUTEX
#define mpl_thread_t PROCESS

extern MUTEX mpl_osemutex;

#define mpl_threads_init()
#define mpl_threads_deinit()

int mpl_mutex_init(mpl_mutex_t **mutex_pp);
int mpl_mutex_destroy(mpl_mutex_t *mutex_p);

#define mpl_mutex_lock(mutex_p)  ose_mutex_lock(mutex_p)
#define mpl_mutex_unlock(mutex_p)  ose_mutex_unlock(mutex_p)

#define mpl_get_current_thread_id CURRENT_PROC

#else

#include "apr_thread_proc.h"
#include "apr_thread_cond.h"
#include "apr_thread_mutex.h"
#include "apr_portable.h"

int mpl_threads_init(void);
int mpl_threads_deinit(void);

extern apr_pool_t *mpl_global_pool_p;

#define mpl_thread_t apr_os_thread_t
#define mpl_mutex_t apr_thread_mutex_t

#define mpl_mutex_init(mutex_pp)                      \
    apr_thread_mutex_create(mutex_pp,                 \
                            APR_THREAD_MUTEX_DEFAULT, \
                            mpl_global_pool_p)

#define mpl_mutex_lock(mutex_p) \
    apr_thread_mutex_lock(mutex_p)
#define mpl_mutex_unlock(mutex_p) \
    apr_thread_mutex_unlock(mutex_p)
#define mpl_mutex_destroy(mutex_p) \
    apr_thread_mutex_destroy(mutex_p)

#define mpl_get_current_thread_id apr_os_thread_current

#endif // COMPILER_ARM

#endif
