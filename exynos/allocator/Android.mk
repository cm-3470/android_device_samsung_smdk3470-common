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

include $(CLEAR_VARS)

LOCAL_MODULE := android.hidl.allocator@1.0-impl.ion
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := IonAllocator.cpp HidlFetch.cpp

#    init_rc: ["android.hidl.allocator@1.0-service.ion.rc"],

LOCAL_SHARED_LIBRARIES := \
        android.hidl.base@1.0 \
        android.hidl.allocator@1.0 \
        libhidlbase \
        libhidltransport \
        libhwbinder \
        libbase \
        liblog \
        libutils \
        libcutils \
        libion_exynos

include $(BUILD_SHARED_LIBRARY)

