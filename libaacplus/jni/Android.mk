LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

#this is the target being built
LOCAL_MODULE:= libheaacEncInterface

define all-c-files-under
$(patsubst ./%,%,$(shell find $(LOCAL_PATH) -name "platform" -prune -o -name "*.c" -and -not -name ".*"))
endef

define all-subdir-c-files
$(call all-c-files-under, .)
endef

#All of the source files that we will compile
LOCAL_SRC_FILES:= $(call all-subdir-c-files)

#All of the shared libraries we link against
LOCAL_SHARED_LIBRARIES :=

#Also need the JNI Headers.
LOCAL_C_INCLUDES += \
$(JNI_H_INCLUDES)\
$(LOCAL_PATH)

LOCAL_CFLAGS := -D__cpusplus -g -mfloat-abi=softfp -mfpu=neon -march=armv7-a -mtune=cortex-a8
LOCAL_LDLIBS := -lz -llog
TARGET_ARCH_ABI :=armeabi-v7a
LOCAL_ARM_MODE := arm

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
	# 采用NEON优化技术
	LOCAL_ARM_NEON := true
endif

LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)
