#!/bin/bash

CURRENT_DIR=$(dirname $(readlink -f $0))
CM_BASE_DIR=$CURRENT_DIR/../../../..

# replace_all <old> <new> <file>
# binary string replacement
function replace_all {
    echo "Patching: $(basename $3)"
    perl -pi -e "s/$1/$2/g" $3
}

for device in kminilte degaslte; do 
    VENDOR_DIR=${CM_BASE_DIR}/vendor/samsung/${device}/proprietary
    echo "Applying patches for device: ${device}"
    if [ ! -d "${VENDOR_DIR}" ]; then
        echo "Device sources missing, skipped"
        continue
    fi

    # add audioserver to the list of allowed services to connect to sec-ril.
    # change UID from 0x3E8 (mediaserver) to 0x411 (audioserver)
    replace_all '\/system\/bin\/mwlan_helper' '\/system\/bin\/audioserver\0' ${VENDOR_DIR}/lib/libsec-ril.so
    #mwlan_helper_addr="\x{2C}\x{EC}\x{23}\x{00}" # G800FXXU1BPE3
    mwlan_helper_addr="\x{34}\x{EC}\x{23}\x{00}" # G800FXXU1BOL4
    postfix="\x{00}\x{00}\x{FF}\x{FF}\x{FF}\x{FF}$mwlan_helper_addr"
    replace_all "\x{E8}\x{03}$postfix" "\x{11}\x{04}$postfix" ${VENDOR_DIR}/lib/libsec-ril.so 

    # KitKat/Lollipop SensorManager is not compatible with Marshmallow, use a compatibility wrapper
    replace_all 'SensorManager' 'SensorMgrComp' ${VENDOR_DIR}/bin/gpsd
    # CRYPTO_malloc is now OPENSSL_malloc in BoringSSL which maps to a simple malloc
    replace_all 'CRYPTO_malloc' 'malloc\0\0\0\0\0\0\0' ${VENDOR_DIR}/bin/gpsd
    
    # Do NOT import libsec-ril.so to avoid importing libsec-ril's Google Protobuffer implementation.
    # CM-13 comes with version 2.6.0, libsec-ril with the old (and imcompatible) version 2.3.0.
    # When the symbols are exported, gpsd will crash loading libGLES_trace.so with a Protobuffer version mismatch 
    # error, as it erroneously uses the libsec-ril Protobuffer implementation instead of the CM-13 one.
    replace_all 'libsec-ril.so' 'libril.so\0\0\0\0' ${VENDOR_DIR}/lib/libwrappergps.so

    # LP camera hal has a reference to libion.so, although it is not using it.
    # libexynoscamera.so wants to use ion_alloc()/... of libion_exynos.so but as 
    # libion.so (with the functions of the same name) is already referenced,
    # the functions of libion.so are erroneously used instead of the libion_exynos.so ones.
    # This causes crashes whenever the camera is used (in ion_alloc).
    # Solution: replace libion.so with liblog.so which is already referenced by the lib (-> no-op)
    replace_all 'libion.so' 'liblog.so' ${VENDOR_DIR}/lib/hw/camera.universal3470.so

    # Signature changed from Lollipop:
    #   GraphicBufferMapper::lock(buffer_handle_t handle, int usage, const Rect& bounds, void** vaddr)
    #     [_ZN7android19GraphicBufferMapper4lockEPK13native_handleiRKNS_4RectEPPv]
    # to Marshmallow:
    #   GraphicBufferMapper::lock(buffer_handle_t handle, uint32_t usage, const Rect& bounds, void** vaddr) 
    #     [_ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPv]
    for lib in ${VENDOR_DIR}/lib/omx/libOMX.Exynos.*.so; do
        replace_all '_ZN7android19GraphicBufferMapper4lockEPK13native_handleiRKNS_4RectEPPv' \
                    '_ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPv' $lib
    done
done
