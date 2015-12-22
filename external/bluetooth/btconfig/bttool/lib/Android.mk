ifeq ($(BOARD_HAVE_BLUETOOTH),true)

#Build libbt_tool.so, install to /system/lib/
#
#tools which uses this lib should include bt_tool.h using follow cmd in Android.mk
#LOCAL_C_INCLUDES:= $(TARGET_OUT_HEADERS)/bt_tool/inc
#
#$( TARGET_OUT_HEADERS) = out/.../<product>/obj/include

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#copy bt_tool.h to out/.../<product>/obj/include/bt_tool/inc/
LOCAL_COPY_HEADERS_TO   := bt_tool/inc
LOCAL_COPY_HEADERS += bt_tool.h


LOCAL_C_INCLUDES:= \
        $(LOCAL_PATH)/include

LOCAL_SRC_FILES:= \
	bt_tool.c

LOCAL_SHARED_LIBRARIES += \
	libcutils   \
	libutils

LOCAL_MODULE:=libbttool

LOCAL_CFLAGS+=-O3

include $(BUILD_SHARED_LIBRARY)

#add static library for testmode
include $(CLEAR_VARS)

#copy bt_tool.h to out/.../<product>/obj/include/bt_tool/inc/
LOCAL_COPY_HEADERS_TO   := bt_tool/inc
LOCAL_COPY_HEADERS += bt_tool.h


LOCAL_C_INCLUDES:= \
        $(LOCAL_PATH)/include

LOCAL_SRC_FILES:= \
	bt_tool.c

LOCAL_SHARED_LIBRARIES += \
	libcutils   \
	libutils

LOCAL_MODULE:=libbttool

LOCAL_CFLAGS+=-O3

include $(BUILD_STATIC_LIBRARY)

endif
