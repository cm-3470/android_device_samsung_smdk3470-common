# Copyright (C) 2013 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

PROPRIETARY_PATH := ../../../../../vendor/samsung/$(TARGET_DEVICE)/proprietary
CONFIG_PATH := ../config


# Audio HAL Wrapper
include $(CLEAR_VARS)

LOCAL_MODULE := audio.primary.$(TARGET_BOOTLOADER_BOARD_NAME)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := audio_hw.c ril_interface.c

LOCAL_C_INCLUDES += \
	external/tinyalsa/include

LOCAL_SHARED_LIBRARIES := liblog libcutils libtinyalsa libdl libhardware

include $(BUILD_SHARED_LIBRARY)


# Audio Zoom Dummy
# Samsungs Lollipop implementation contains text relocations
# which are not supported in Android N anymore.
# Use a dummy implementation instead.
include $(CLEAR_VARS)

LOCAL_MODULE := libsamsungRecord_zoom
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := audio_zoom.cpp

LOCAL_SHARED_LIBRARIES := liblog

include $(BUILD_SHARED_LIBRARY)


# Stock Audio HAL
include $(CLEAR_VARS)

LOCAL_MODULE		:= audio.vendor.universal3470
LOCAL_MODULE_TAGS	:= optional
LOCAL_MODULE_SUFFIX 	:= .so
LOCAL_SRC_FILES		:= $(PROPRIETARY_PATH)/lib/hw/audio.primary.universal3470.so
LOCAL_MODULE_CLASS 	:= SHARED_LIBRARIES
LOCAL_MODULE_PATH	:= $(TARGET_OUT_SHARED_LIBRARIES)/hw

include $(BUILD_PREBUILT)


# tinyucm configuration
include $(CLEAR_VARS)

LOCAL_MODULE := tinyucm.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(CONFIG_PATH)/tinyucm.conf
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)

include $(BUILD_PREBUILT)


# default_gain.conf
include $(CLEAR_VARS)

LOCAL_MODULE := default_gain.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(CONFIG_PATH)/default_gain.conf
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)

include $(BUILD_PREBUILT)
