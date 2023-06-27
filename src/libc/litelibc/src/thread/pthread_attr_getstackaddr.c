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
 * Description: pthread_attr_getstackaddr 接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackAddr)
{
    if (attr == NULL || stackAddr == NULL || attr->stackaddr_set == 0) {
        return EINVAL;
    }

    *stackAddr = attr->stackaddr;

    return OS_OK;
}