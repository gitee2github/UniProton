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
 * Description: 标准输入 输出 错误流，临时解决编译问题, 当前不支持文件操作
 */
#include "stdio_impl.h"

// TODO: 临时解决编译问题, 当前不支持文件操作
#undef stdin
static unsigned char stdin_buf[BUFSIZ+UNGET];
hidden FILE __stdin_FILE = {
	.buf = stdin_buf+UNGET,
	.buf_size = sizeof stdin_buf-UNGET,
	.fd = 0,
	.flags = F_PERM | F_NOWR,
	.read = NULL,
	.seek = NULL,
	.close = NULL,
	.lock = -1,
};
FILE *const stdin = &__stdin_FILE;
FILE *volatile __stdin_used = &__stdin_FILE;

#undef stdout
static unsigned char stdout_buf[BUFSIZ+UNGET];
hidden FILE __stdout_FILE = {
	.buf = stdout_buf+UNGET,
	.buf_size = sizeof stdout_buf-UNGET,
	.fd = 1,
	.flags = F_PERM | F_NORD,
	.lbf = '\n',
	.write = NULL,
	.seek = NULL,
	.close = NULL,
	.lock = -1,
};
FILE *const stdout = &__stdout_FILE;
FILE *volatile __stdout_used = &__stdout_FILE;

#undef stderr
static unsigned char stderr_buf[UNGET];
hidden FILE __stderr_FILE = {
	.buf = stderr_buf+UNGET,
	.buf_size = 0,
	.fd = 2,
	.flags = F_PERM | F_NORD,
	.lbf = -1,
	.write = NULL,
	.seek = NULL,
	.close = NULL,
	.lock = -1,
};
FILE *const stderr = &__stderr_FILE;
FILE *volatile __stderr_used = &__stderr_FILE;