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
 * Description: pthread_getname_np 相关接口实现
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <pthread.h>
#include "prt_posix_internal.h"

int pthread_getname_np(pthread_t thread, char *name, size_t len)
{
    U32 ret;
    char *str = NULL;
    if (name == NULL || len == 0) {
        return EINVAL;
    }
    ret = PRT_TaskGetName((TskHandle)thread, &str);
    if (ret != OS_OK) {
        return ret;
    }

    if (str == NULL || strncpy_s(name, len, str, strlen(str)) != EOK) {
        return EINVAL;
    }
    name[len - 1] = '\0';
    return OS_OK;
}