# Copyright (C) 2017 The LineageOS Project
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

#ifeq ($(TARGET_AUDIOHAL_VARIANT),samsung)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
	audience.c \
	audio_hw.c \
	ril_interface.c \
	voice.c
	#compress_offload.c \

# TODO: remove resampler if possible when AudioFlinger supports downsampling from 48 to 8
LOCAL_SHARED_LIBRARIES := \
	liblog \
	libcutils \
	libaudioutils \
	libhardware \
	libtinyalsa \
	libaudioroute \
	libdl \
	libsecril-client
	#libtinycompress \


LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include \
	external/tinyalsa/include \
	hardware/libhardware/include \
	hardware/samsung/ril/libsecril-client \
	$(call include-path-for, audio-utils) \
	$(call include-path-for, audio-route) \
	$(call include-path-for, audio-effects)

	#external/tinycompress/include \

#LOCAL_CFLAGS := -Werror -Wall
LOCAL_CFLAGS := -Wall
#LOCAL_CFLAGS += -DPREPROCESSING_ENABLED
#LOCAL_CFLAGS += -DHW_AEC_LOOPBACK

LOCAL_MODULE := audio.primary.$(TARGET_BOOTLOADER_BOARD_NAME)

LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

# Mixer configurations
include $(CLEAR_VARS)

LOCAL_MODULE := mixer_paths_0.xml
LOCAL_MODULE_TAGS := optional eng
LOCAL_MODULE_CLASS := ETC

LOCAL_SRC_FILES := mixer_paths_0.xml

LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)

include $(BUILD_PREBUILT)

#endif
