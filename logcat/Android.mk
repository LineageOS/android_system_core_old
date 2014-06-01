# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= logcat.cpp event.logtags

LOCAL_SHARED_LIBRARIES := liblog

LOCAL_MODULE:= logcat

include $(BUILD_EXECUTABLE)

SYMLINKS := $(TARGET_OUT)/bin/lolcat
SYMLINKS += $(TARGET_OUT)/bin/logdog
SYMLINKS += $(TARGET_OUT)/bin/loldog
$(SYMLINKS): LOGCAT_BINARY := $(LOCAL_MODULE)
$(SYMLINKS): $(LOCAL_INSTALLED_MODULE) $(LOCAL_PATH)/Android.mk
	@echo "Symlink: $@ -> $(LOGCAT_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf $(LOGCAT_BINARY) $@

ALL_DEFAULT_INSTALLED_MODULES += $(SYMLINKS)
