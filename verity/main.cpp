/*
 * Copyright (C) 2018 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>

#include "verity.h"

int main(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        if (strncmp(argv[i], "--enable", 8) == 0) {
            return toggle_verity(true);
        } else if (strncmp(argv[i], "--disable", 9) == 0) {
            return toggle_verity(false);
        }
    }

    printf("veritytool - toggle block device verification\n"
           "    --help        show this help\n"
           "    --enable      enable dm-verity\n"
           "    --disable     disable dm-verity\n");
    return 0;
}
