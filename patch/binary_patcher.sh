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
    #mwlan_helper_addr="\x{34}\x{EC}\x{23}\x{00}" # G800FXXU1BOL4
    mwlan_helper_addr="\x{F6}\x{8A}\x{20}\x{00}" # G800FXXU1CPK5
    postfix="\x{00}\x{00}\x{FF}\x{FF}\x{FF}\x{FF}$mwlan_helper_addr"
    replace_all "\x{E8}\x{03}$postfix" "\x{11}\x{04}$postfix" ${VENDOR_DIR}/lib/libsec-ril.so 

    # LP camera hal has a reference to libion.so, although it is not using it.
    # libexynoscamera.so wants to use ion_alloc()/... of libion_exynos.so but as 
    # libion.so (with the functions of the same name) is already referenced,
    # the functions of libion.so are erroneously used instead of the libion_exynos.so ones.
    # This causes crashes whenever the camera is used (in ion_alloc).
    # Solution: replace libion.so with liblog.so which is already referenced by the lib (-> no-op)
    replace_all 'libion.so' 'liblog.so' ${VENDOR_DIR}/lib/hw/camera.universal3470.so
done
