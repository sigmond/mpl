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
 */

#include <stdlib.h>
#include <string.h>
#include "mpl_list.h"
#include "mpl_pthread.h"

#if defined(MPL_USE_PTHREAD_MUTEX)

int mpl_mutex_init(mpl_mutex_t **mutex_pp)
{
    pthread_mutex_t *mutex;
    int ret;

    mutex = malloc(sizeof(pthread_mutex_t));
    if (mutex == NULL)
        return -1;

    ret = pthread_mutex_init(mutex, NULL);
    *mutex_pp = mutex;

    return ret;
}

int mpl_mutex_destroy(mpl_mutex_t *mutex_p)
{
    int ret;
    ret = pthread_mutex_destroy(mutex_p);
    free(mutex_p);
    return ret;
}
#elif defined(MPL_USE_OSE_MUTEX)

int mpl_mutex_init(mpl_mutex_t **mutex_pp)
{
    MUTEX *mutex;
    mutex = malloc(sizeof(MUTEX));
    if (mutex == NULL)
        return -1;

    memset(mutex, 0, sizeof(MUTEX));
    ose_mutex_init(mutex, 0);
    *mutex_pp = mutex;
    return 0;
}

int mpl_mutex_destroy(mpl_mutex_t *mutex_p)
{
    ose_mutex_destroy(mutex_p);
    free(mutex_p);
    return 0;
}

#else

apr_pool_t *mpl_global_pool_p = NULL;

int mpl_threads_init(void)
{
    /* This may be run more than once. This is ok with apr_initialize(). */
    apr_initialize();
    if (mpl_global_pool_p == NULL)
        apr_pool_create(&mpl_global_pool_p, NULL);

    return 0;
}

int mpl_threads_deinit(void)
{
    if (mpl_global_pool_p != NULL){
        apr_pool_destroy(mpl_global_pool_p);
        mpl_global_pool_p = NULL;
    }

    apr_terminate();

    return 0;
}
#endif
