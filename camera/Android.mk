LOCAL_PATH := $(call my-dir)

PROPRIETARY_PATH := ../../../../vendor/samsung/$(TARGET_DEVICE)/proprietary

# Camera HAL Wrapper
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    system/media/camera/include \
    system/core/base/include \
    frameworks/native/libs/arect/include

LOCAL_SRC_FILES := \
    CameraWrapper.cpp

LOCAL_SHARED_LIBRARIES := \
    libhardware liblog libcamera_client libutils \
    android.hidl.token@1.0-utils \
    android.hardware.graphics.bufferqueue@1.0


LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE := camera.universal3470
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

# Stock Camera HAL
include $(CLEAR_VARS)

LOCAL_MODULE		:= camera.vendor.universal3470
LOCAL_MODULE_TAGS	:= optional
LOCAL_MODULE_SUFFIX 	:= .so
LOCAL_SRC_FILES		:= $(PROPRIETARY_PATH)/lib/hw/camera.universal3470.so
LOCAL_MODULE_CLASS 	:= SHARED_LIBRARIES
LOCAL_MODULE_PATH	:= $(TARGET_OUT_SHARED_LIBRARIES)/hw

include $(BUILD_PREBUILT)

