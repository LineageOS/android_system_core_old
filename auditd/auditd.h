/*
 * Copyright 2012, Samsung Telecommunications of America
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Written by William Roberts <w.roberts@sta.samsung.com>
 */

#ifndef _AUDITD_H_
#define _AUDITD_H_

#define LOG_TAG "auditd"

#include <cutils/log.h>

/* Debugging statements that go to the system log
 * aka logcat.
 * The naming convention follows the severity of the
 * message to print.
 */
#define INFO(...)     SLOGI(__VA_ARGS__)
#define ERROR(...)    SLOGE(__VA_ARGS__)
#define WARNING(...)  SLOGW(__VA_ARGS__)

/*
 * Given an errno variable, returns the abs value of it
 * Some of the existing interfaces return a -errno instead of -1
 * with errno set. This is just a way of ensuring that the errno
 * you are using, is a positive value.
 */
#define GETERRNO(x) ((x < 0) ? -x : x)

#endif

