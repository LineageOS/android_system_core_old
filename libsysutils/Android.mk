LOCAL_PATH:= $(call my-dir)

common_src_files :=                           \
                  src/SocketListener.cpp      \
                  src/FrameworkListener.cpp   \
                  src/NetlinkListener.cpp     \
                  src/NetlinkEvent.cpp        \
                  src/FrameworkCommand.cpp    \
                  src/SocketClient.cpp        \
                  src/ServiceManager.cpp      \
                  EventLogTags.logtags

common_shared_libraries := \
        libcutils \
        liblog \
        libnl

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= $(common_src_files)
LOCAL_MODULE:= libsysutils
LOCAL_CFLAGS := -Werror
LOCAL_SHARED_LIBRARIES := $(common_shared_libraries)
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= $(common_src_files)
LOCAL_MODULE:= libsysutils
LOCAL_CFLAGS := -Werror
LOCAL_SHARED_LIBRARIES := $(common_shared_libraries)
include $(BUILD_STATIC_LIBRARY)
