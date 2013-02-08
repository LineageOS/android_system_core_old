# Copyright 2005 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	auditd.c \
	netlink.c \
	libaudit.c \
	audit_log.c

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libc

LOCAL_MODULE_TAGS:=optional
LOCAL_MODULE:=auditd

include $(BUILD_EXECUTABLE)
