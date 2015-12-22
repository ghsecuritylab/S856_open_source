ifeq ($(BOARD_HAVE_BLUETOOTH),true)

#Build bt_tool, install in /system/xbin/
#$( TARGET_OUT_HEADERS) = out/.../<product>/obj/include

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	bt_main.c

LOCAL_C_INCLUDES:= \
	$(TARGET_OUT_HEADERS)/bt_tool/inc

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE:=bttool

LOCAL_SHARED_LIBRARIES += \
	libcutils   \
	libutils	\
	libbttool

include $(BUILD_EXECUTABLE)

endif
