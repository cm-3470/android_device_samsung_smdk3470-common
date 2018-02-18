#
# Copyright (C) 2014 The CyanogenMod Project
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
#

$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)

COMMON_PATH := device/samsung/smdk3470-common

# HIDL
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/manifest.xml:system/vendor/manifest.xml

# Audio
PRODUCT_PACKAGES += \
    audio.a2dp.default \
    audio.primary.universal3470 \
    audio.r_submix.default \
    audio.usb.default \
    android.hardware.audio@2.0-impl \
    android.hardware.audio.effect@2.0-impl

# needed by open-source audio-hal
PRODUCT_PACKAGES += \
    mixer_paths.xml

# needed by stock audio-hal
PRODUCT_PACKAGES += \
    audio.vendor.universal3470 \
    libsamsungRecord_zoom \
    default_gain.conf \
    tinyucm.conf

# Sound
PRODUCT_PACKAGES += \
    sound_trigger.primary.universal3470

# HW composer
PRODUCT_PACKAGES += \
    libion

# Media
PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:system/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:system/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video_le.xml:system/etc/media_codecs_google_video_le.xml

# Samsung
PRODUCT_PACKAGES += \
    SamsungServiceMode

# Camera
PRODUCT_PACKAGES += \
    camera.universal3470 \
    camera.vendor.universal3470 \
    camera.device@1.0-impl-legacy \
    android.hardware.camera.provider@2.4-impl-legacy \
    libshim_camera

# DRM
PRODUCT_PACKAGES += \
    android.hardware.drm@1.0-impl

# Graphics
PRODUCT_PACKAGES += \
    android.hardware.graphics.allocator@2.0-impl \
    android.hardware.graphics.allocator@2.0-service \
    android.hardware.graphics.composer@2.1-impl \
    android.hardware.graphics.mapper@2.0-impl

# RenderScript HAL
PRODUCT_PACKAGES += \
    android.hardware.renderscript@1.0-impl

# Memory
PRODUCT_PACKAGES += \
    android.hardware.memtrack@1.0-impl

# Wifi
PRODUCT_PACKAGES += \
    android.hardware.wifi@1.0 \
    android.hardware.wifi@1.0-impl \
    android.hardware.wifi@1.0-service \
    libwpa_client \
    hostapd \
    wificond \
    wifilogd \
    wpa_supplicant \
    wifiloader

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/wifi/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf \
    $(LOCAL_PATH)/wifi/wpa_supplicant_overlay.conf:system/etc/wifi/wpa_supplicant_overlay.conf \
    $(LOCAL_PATH)/wifi/p2p_supplicant_overlay.conf:system/etc/wifi/p2p_supplicant_overlay.conf

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/vendor/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/vendor/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/vendor/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/vendor/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.telephony.cdma.xml:system/vendor/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/vendor/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/vendor/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/vendor/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/vendor/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.audio.low_latency.xml:system/vendor/etc/permissions/android.hardware.audio.low_latency.xml \
    frameworks/native/data/etc/android.hardware.ethernet.xml:system/vendor/etc/permissions/android.hardware.ethernet.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/vendor/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/vendor/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/handheld_core_hardware.xml:system/vendor/etc/permissions/handheld_core_hardware.xml

# FlipFlap
PRODUCT_PACKAGES += \
    FlipFlap

# IPv6 tethering
PRODUCT_PACKAGES += \
    ebtables \
    ethertypes
    
# libstlport (only copies a prebuilt lib since oreo)
#PRODUCT_PACKAGES += \
#    libstlport 

# RIL
PRODUCT_PACKAGES += \
    libsecril-client \
    android.hardware.radio@1.0 \
    android.hardware.radio.deprecated@1.0

# Deps for libsec-ril.so and audio.primary.universal3470.so
# We need libprotobuf-cpp-full 3.0 (libhwui.so) and 2.6 (libsec-ril.so)
PRODUCT_PACKAGES += \
    libxml2 \
    libprotobuf-cpp-full \
    libprotobuf-cpp-fl26

# Bluetooth
PRODUCT_PACKAGES += \
    android.hardware.bluetooth@1.0-impl \
    libbt-vendor

# Set bluetooth soc to rome (TODO: check if "cherokee" or "rome")
PRODUCT_PROPERTY_OVERRIDES += \
    qcom.bluetooth.soc=rome

# GNSS HAL
PRODUCT_PACKAGES += \
    android.hardware.gnss@1.0-impl

# Keymaster
PRODUCT_PACKAGES += \
    android.hardware.keymaster@3.0-impl \
    android.hardware.keymaster@3.0-service

# Storage
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sys.sdcardfs=true

# Set default USB interface
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp

# System properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.arch=exynos3470 \
    debug.hwui.render_dirty_regions=false \
    ro.opengles.version=131072 \
    ro.zygote.disable_gl_preload=true

# Camera HAL1 compatibility
PRODUCT_PROPERTY_OVERRIDES += \
    media.stagefright.legacyencoder=true \
    media.stagefright.less-secure=true

# Default OMX service to non-Treble
PRODUCT_PROPERTY_OVERRIDES += \
    persist.media.treble_omx=false

# USB Accesory
PRODUCT_PACKAGES += \
    com.android.future.usb.accessory
