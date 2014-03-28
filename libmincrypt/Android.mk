# Copyright 2008 The Android Open Source Project
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libmincrypt
LOCAL_SRC_FILES := dsa_sig.c p256.c p256_ec.c p256_ecdsa.c rsa.c sha.c sha256.c
LOCAL_CFLAGS := -Wall -Werror
include $(BUILD_STATIC_LIBRARY)

## Crippled version without an RSA implementation
## to coexist with libcrypto_static and provide SHA_hash
include $(CLEAR_VARS)
LOCAL_MODULE := libminshacrypt
LOCAL_SRC_FILES := sha.c sha256.c
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libmincrypt
LOCAL_SRC_FILES := dsa_sig.c p256.c p256_ec.c p256_ecdsa.c rsa.c sha.c sha256.c
LOCAL_CFLAGS := -Wall -Werror
include $(BUILD_HOST_STATIC_LIBRARY)

include $(LOCAL_PATH)/tools/Android.mk \
        $(LOCAL_PATH)/test/Android.mk
