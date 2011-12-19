LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= logwrapper.c
LOCAL_MODULE := logwrapper
LOCAL_STATIC_LIBRARIES := liblog
include $(BUILD_EXECUTABLE)


ifeq ($(BOARD_USES_BOOTMENU),true)
include $(CLEAR_VARS)
LOCAL_MODULE := logwrapper.bin
LOCAL_MODULE_TAGS := eng debug
LOCAL_SRC_FILES:= logwrapper.c
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_STATIC_LIBRARIES := libc libcutils liblog
include $(BUILD_EXECUTABLE)
endif
