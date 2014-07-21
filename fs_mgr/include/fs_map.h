/*
 * Copyright (C) 2014 The CyanogenMod Project
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

#ifndef __CORE_FS_MAP_H
#define __CORE_FS_MAP_H

struct fs_map {
	const char	*fs_type;
	const char	*mount_point;
	const char	*mount_options;
};

/*
 * These are default, fallback mount options for the 'auto'
 * type. NULL signifies a default if no option for mount_point
 * exists for the partition attempting to be mounted.
 */

// TODO - use sane values here, i'm just testing around for now

static struct fs_map fs_map_array[] = {
/*  fs_type  	mount_point	mount_options*/
  { "ext2",	NULL,		"noatime,nosuid,nodev,barrier=1,data=ordered,noauto_da_alloc,journal_async_commit,errors=panic wait,check"},
  { "ext3",	NULL,		"noatime,nosuid,nodev,barrier=1,data=ordered,noauto_da_alloc,journal_async_commit,errors=panic wait,check"},
  { "ext4",	"/system",	"ro,seclabel,noatime,data=ordered"},
  { "ext4",	NULL,		"noatime,nosuid,nodev,barrier=1,data=ordered,noauto_da_alloc,journal_async_commit,errors=panic wait,check"},
  { "f2fs",	NULL,		"rw,discard,nosuid,nodev,noatime,nodiratime,inline_xattr,errors=recover"},
  { "vfat",	NULL,		"ro,shortname=lower,uid=1000,gid=1000,dmask=227,fmask=337"}
};

#endif /* __CORE_FS_MAP_H */
