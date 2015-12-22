LOCAL_PATH := $(call my-dir)
PREBUILT_DIR_PATH := $(LOCAL_PATH)

ifeq ($(call is-board-platform,msm8226),true)
  -include $(LOCAL_PATH)/target/product/msm8226/Android.mk
endif

ifeq ($(call is-board-platform,msm8610),true)
  -include $(LOCAL_PATH)/target/product/msm8610/Android.mk
endif

-include $(sort $(wildcard $(PREBUILT_DIR_PATH)/*/*/Android.mk))
