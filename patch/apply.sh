#!/bin/bash

MYABSPATH=$(readlink -f "$0")
PATCHBASE=$(dirname "$MYABSPATH")
CMBASE=$(readlink -f "$PATCHBASE/../../../../")

cd "$CMBASE/bootable/recovery"
if git remote -v | grep omnirom; then
    TWRP=true
else
    TWRP=false
fi

for i in $(find "$PATCHBASE"/* -type d); do
    PATCHNAME=$(basename "$i")
    PATCHTARGET=$PATCHNAME
    for j in $(seq 4); do
        PATCHTARGET=$(echo $PATCHTARGET | sed 's/_/\//')
        if [ -d "$CMBASE/$PATCHTARGET" ]; then break; fi
    done

    echo
    echo applying $PATCHNAME to $PATCHTARGET
    cd "$CMBASE/$PATCHTARGET" || exit 1

    if compgen -G "$PATCHBASE/$PATCHNAME/*.patch" > /dev/null; then
        if ! git am --ignore-whitespace -3 "$PATCHBASE/$PATCHNAME"/*.patch; then
            git am --abort
            exit 1
        fi
    fi
done
