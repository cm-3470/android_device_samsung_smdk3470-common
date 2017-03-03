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

# Sound
PRODUCT_PACKAGES += \
    sound_trigger.primary.universal3470

# Audio
PRODUCT_PACKAGES += \
    audio.a2dp.default \
    audio.primary.universal3470 \
    audio.r_submix.default \
    audio.usb.default

# needed by open-source audio-hal
PRODUCT_PACKAGES += \
    mixer_paths.xml

# needed by stock audio-hal
PRODUCT_PACKAGES += \
    audio.vendor.universal3470 \
    libsamsungRecord_zoom \
    default_gain.conf \
    tinyucm.conf

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
    Snap

# Wifi
PRODUCT_PACKAGES += \
    hostapd \
    wpa_supplicant

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/wifi/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf \
    $(LOCAL_PATH)/wifi/wpa_supplicant_overlay.conf:system/etc/wifi/wpa_supplicant_overlay.conf \
    $(LOCAL_PATH)/wifi/p2p_supplicant_overlay.conf:system/etc/wifi/p2p_supplicant_overlay.conf

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml

# Browser
PRODUCT_PACKAGES += \
    Gello

# FlipFlap
PRODUCT_PACKAGES += \
    FlipFlap

# IPv6 tethering
PRODUCT_PACKAGES += \
    ebtables \
    ethertypes
    
# libstlport
PRODUCT_PACKAGES += \
    libstlport 

# RIL
PRODUCT_PACKAGES += \
    libsecril-client

# Deps for libsec-ril.so and audio.primary.universal3470.so
PRODUCT_PACKAGES += \
    libxml2 \
    libprotobuf-cpp-full

PRODUCT_PROPERTY_OVERRIDES += \
    ro.telephony.ril_class=Exynos3470RIL

# Set bluetooth soc to rome (TODO: check if "cherokee" or "rome")
PRODUCT_PROPERTY_OVERRIDES += \
    qcom.bluetooth.soc=rome

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

# USB Accesory
PRODUCT_PACKAGES += \
    com.android.future.usb.accessory
