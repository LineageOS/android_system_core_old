/*
 * Copyright 2013, Quark Security Inc
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
 * Written by Joshua Brindle <brindle@quarksecurity.com>
 */

#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/audit.h>

#define LOG_TAG "audit_fields"
#include <cutils/log.h>

#define L1(line) L2(line)
#define L2(line) str##line
static const union audit_to_string_data {
        struct {
#define S_(v, s) char L1(__LINE__)[sizeof(s)];
#include "fieldtab.h"
#undef  S_
        };
        char str[0];
} audit_to_string_data = {
        {
#define S_(v, s) s,
#include "fieldtab.h"
#undef  S_
        }
};
static const int audit_to_string[] = {
#define S_(v, s) offsetof(union audit_to_string_data, L1(__LINE__)),
#include "fieldtab.h"
#undef  S_
};

static const int audit_map[] = {
#define S_(v, s) v,
#include "fieldtab.h"
#undef  S_
};

#define FIELDS (sizeof(audit_to_string) / sizeof(audit_to_string[0]))

int string_to_audit_field(const char *s)
{
    unsigned int val;

    if (isdigit(s[0])) {
        val = atoi(s);
        if (val > 0 && val < FIELDS) {
            return audit_map[val];
        }
    } else {
        for (val = 0; val < FIELDS; val++) {
            if (strcmp(s, (audit_to_string_data.str
                           + audit_to_string[val])) == 0) {
                    return audit_map[val];
            }
        }
    }

    errno = EINVAL;
    return 0;
}

const char* audit_field_to_string(int field)
{
    unsigned int i;
    for (i=0; i < FIELDS; i++) {
        if (audit_map[i] == field) {
            return audit_to_string_data.str + audit_to_string[i];
        }
    }

    errno = EINVAL;
    return NULL;
}
