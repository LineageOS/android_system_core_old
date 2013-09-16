# Copyright 2011 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= fs_mgr.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_MODULE:= libfs_mgr
LOCAL_STATIC_LIBRARIES := liblogwrap

ifeq ($(TARGET_USERIMAGES_USE_F2FS),true)
  LOCAL_SRC_FILES += recover_userdata.c

  LOCAL_CFLAGS += -DTARGET_USERIMAGES_USE_F2FS

  LOCAL_C_INCLUDES += \
      system/vold \
      system/extras/ext4_utils

  LOCAL_STATIC_LIBRARIES += libext4_utils_static libsparse_static libz libselinux
endif
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

ifneq ($(BOARD_USERIMAGE_BLOCK_SIZE),)
  LOCAL_CFLAGS += -DBOARD_USERIMAGE_BLOCK_SIZE=$(BOARD_USERIMAGE_BLOCK_SIZE)
endif

include $(BUILD_STATIC_LIBRARY)



include $(CLEAR_VARS)

LOCAL_SRC_FILES:= fs_mgr_main.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_MODULE:= fs_mgr

LOCAL_MODULE_TAGS := optional
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)/sbin
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_UNSTRIPPED)

LOCAL_STATIC_LIBRARIES := libfs_mgr liblogwrap libcutils liblog libc

include $(BUILD_EXECUTABLE)

