/*
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-05-29
 * Description: pthread_once 相关接口实现
 */
#include <pthread.h>
#include "prt_posix_internal.h"

static pthread_mutex_t g_pthreadonce = PTHREAD_MUTEX_INITIALIZER;

int __pthread_once(pthread_once_t *control, void (*init)(void))
{
    pthread_once_t old;
    int ret;
    if (control == NULL || init == NULL) {
        return EINVAL;
    }

    ret = pthread_mutex_lock(&g_pthreadonce);
    if (ret != OS_OK) {
        return ret;
    }

    old = *control;
    *(int *)control = 1;

    ret =  pthread_mutex_unlock(&g_pthreadonce);
    if (ret != OS_OK) {
        return ret;
    }

    if (!old) {
        init();
    }

    return OS_OK;
}

weak_alias(__pthread_once, pthread_once);