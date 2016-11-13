/*
 * Copyright (C) 2016 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define LOG_TAG "AudioZoom"

#include <log/log.h>

#define UNUSED(expr) (void)(expr)

namespace android {

/**
 * Dummy implementation for libsamsungRecord_zoom.
 *
 * This library is not used at all if the "audio_zoom" audio parameter
 * is not set (default).
 */
class SamsungRecord_Zoom {
public:
    SamsungRecord_Zoom();

    ~SamsungRecord_Zoom();

#ifdef AUDIO_USES_KK_LIBS
    void Init(int unk1);
#else
    void Init(int unk1, float unk2);
#endif

    void Process(short *in, short *out, float zoom, unsigned int unk);
};

SamsungRecord_Zoom::SamsungRecord_Zoom() {
}

SamsungRecord_Zoom::~SamsungRecord_Zoom() {
}

#ifdef AUDIO_USES_KK_LIBS
void SamsungRecord_Zoom::Init(int unk1) {
    UNUSED(unk1);
    ALOGE("%s called", __FUNCTION__);
}
#else
void SamsungRecord_Zoom::Init(int unk1, float unk2) {
    UNUSED(unk1);
    UNUSED(unk2);
    ALOGE("%s called", __FUNCTION__);
}
#endif

void SamsungRecord_Zoom::Process(short *in, short *out, float zoom, unsigned int unk) {
    UNUSED(in);
    UNUSED(out);
    UNUSED(zoom);
    UNUSED(unk);
    ALOGE("%s called", __FUNCTION__);
}

}


