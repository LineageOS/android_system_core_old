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

#ifndef _AUDIT_RULES_H_
#define _AUDIT_RULES_H_

extern int audit_rules_read_and_add(int audit_fd, const char *rulefile);

#endif
