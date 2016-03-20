/*
 * Copyright (C) 2011 The Android Open Source Project
 * Copyright (C) 2012 Wolfson Microelectronics plc
 * Copyright (C) 2012 The CyanogenMod Project
 *               Daniel Hillenbrand <codeworkx@cyanogenmod.com>
 *               Guillaume "XpLoDWilD" Lesniak <xplodgui@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "audio_hw_primary"
#define LOG_NDEBUG 0

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>

#include <cutils/log.h>
#include <cutils/str_parms.h>
#include <cutils/properties.h>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <hardware/audio.h>

#include <tinyalsa/asoundlib.h>
#include <audio_utils/resampler.h>
#include <audio_route/audio_route.h>

#include "audio_hw.h"
#include "ril_interface.h"

struct pcm_config pcm_config_mm = {
    .channels = 2,
    .rate = MM_FULL_POWER_SAMPLING_RATE,
    .period_size = DEEP_BUFFER_LONG_PERIOD_SIZE,
    .period_count = PLAYBACK_DEEP_BUFFER_LONG_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct pcm_config pcm_config_tones = {
    .channels = 2,
    .rate = MM_FULL_POWER_SAMPLING_RATE,
    .period_size = SHORT_PERIOD_SIZE,
    .period_count = PLAYBACK_SHORT_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
    .start_threshold = 0,
    .avail_min = 0,
};

struct pcm_config pcm_config_capture = {
    .channels = 2,
    .rate = DEFAULT_IN_SAMPLING_RATE,
    .period_size = CAPTURE_PERIOD_SIZE,
    .period_count = CAPTURE_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct pcm_config pcm_config_vx = {
    .channels = 2,
    .rate = VX_WB_SAMPLING_RATE,
    .period_size = 160,
    .period_count = 2,
    .format = PCM_FORMAT_S16_LE,
};

#define MIN(x, y) ((x) > (y) ? (y) : (x))

struct audio_device {
    struct audio_hw_device hw_device;

    pthread_mutex_t lock;       /* see note below on mutex acquisition order */
    struct audio_route *ar;
    audio_mode_t mode;
    int active_out_device;
    int out_device;
    int active_in_device;
    int in_device;

    /* Call audio */
    struct pcm *pcm_modem_dl;
    struct pcm *pcm_modem_ul;

    /* SCO audio */
    struct pcm *pcm_bt_dl;
    struct pcm *pcm_bt_ul;

    bool in_call;
    float voice_volume;
    struct stream_in *active_input;
    struct stream_out *outputs[OUTPUT_TOTAL];
    bool mic_mute;
    int tty_mode;
    bool bluetooth_nrec;
    bool wb_amr;
    bool screen_off;
    bool two_mic;

    /* RIL */
    struct ril_handle ril;
};

struct stream_out {
    struct audio_stream_out stream;

    pthread_mutex_t lock;       /* see note below on mutex acquisition order */
    struct pcm_config config[PCM_TOTAL];
    struct pcm *pcm[PCM_TOTAL];
    struct resampler_itfe *resampler;
    char *buffer;
    size_t buffer_frames;
    int standby;
    int write_threshold;
    bool use_long_periods;
    audio_channel_mask_t channel_mask;
    audio_channel_mask_t sup_channel_masks[3];

    struct audio_device *dev;
};

struct stream_in {
    struct audio_stream_in stream;

    pthread_mutex_t lock;       /* see note below on mutex acquisition order */
    struct pcm_config config;
    struct pcm *pcm;
    int device;
    struct resampler_itfe *resampler;
    struct resampler_buffer_provider buf_provider;
    unsigned int requested_rate;
    int standby;
    int source;

    int16_t *read_buf;
    size_t read_buf_size;
    size_t read_buf_frames;

    int16_t *proc_buf_in;
    int16_t *proc_buf_out;
    size_t proc_buf_size;
    size_t proc_buf_frames;

    int16_t *ref_buf;
    size_t ref_buf_size;
    size_t ref_buf_frames;

    int read_status;

    uint32_t main_channels;
    struct audio_device *dev;
};

/**
 * NOTE: when multiple mutexes have to be acquired, always respect the following order:
 *        hw device > in stream > out stream
 */

static int adev_set_voice_volume(struct audio_hw_device *dev, float volume);
static int do_input_standby(struct stream_in *in);
static int do_output_standby(struct stream_out *out);

static const struct dev_name_map_t {
    int mask;
    const char *name;
} dev_names[] = {
    { AUDIO_DEVICE_OUT_EARPIECE, "Earpiece" },
    { AUDIO_DEVICE_OUT_SPEAKER, "Speaker" },
    { AUDIO_DEVICE_OUT_WIRED_HEADSET, "Headset Out" },
    { AUDIO_DEVICE_OUT_WIRED_HEADPHONE, "Headphone" },
    { AUDIO_DEVICE_OUT_BLUETOOTH_SCO, "SCO" },
    { AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET, "SCO Headset Out" },
    { AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT, "SCO Carkit" },
    { AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET, "Analogue Dock Out" },
    { AUDIO_DEVICE_OUT_AUX_DIGITAL, "AUX Digital Out" },

    { AUDIO_DEVICE_IN_BUILTIN_MIC, "Builtin Mic" },
    { AUDIO_DEVICE_IN_BACK_MIC, "Back Mic" },
    { AUDIO_DEVICE_IN_WIRED_HEADSET, "Headset In" },
    { AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET, "SCO Headset In" },
    { AUDIO_DEVICE_IN_FM_TUNER, "FMRadio In" },
};

#define VERB_NORMAL         "Normal"
#define VERB_RINGTONE       "Ringtone"
#define VERB_COMMUNICATION  "Communication"
#define VERB_VOICECALL      "Voicecall"
#define VERB_FMRADIO        "FMRadio"
#define VERB_LOOPBACK       "Loopback"

#define MODIFIER_NORMAL        "Normal"
#define MODIFIER_INCALL        "Incall"
#define MODIFIER_INCALL_IN     "IncallIn"
#define MODIFIER_RINGTONE      "Ringtone"
#define MODIFIER_DUAL_NORMAL   "Dual Normal"
#define MODIFIER_DUAL_RINGTONE "Dual Ringtone"
#define MODIFIER_CODEC_RX_MUTE "CodecRxMute"
/* Incommunication
 * VideoCall
 * TTY Mode Out
 * VoLTECpCallout
 * Voipout
 * SecVoipout
 * ImsVoipout
 * FMRadio
 * FMRadio_Mute
 * Loopback
 * ... */

#define PREFIX_DEVICE   "Device"
#define PREFIX_VERB     "Verb"
#define PREFIX_MODIFIER "Modifier"
#define ENABLE_FLAG(e) (e ? "On" : "Off")

static inline bool dev_contains_mask(audio_devices_t devices, audio_devices_t mask)
{
    if ((devices & AUDIO_DEVICE_BIT_IN) != (mask & AUDIO_DEVICE_BIT_IN))
        return false;
    return ((devices & mask) == mask);
}

static int apply_verb_route(struct audio_device *adev, const char* verb, bool enable)
{
    char route[256];
    snprintf(route, sizeof(route), PREFIX_VERB"|%s|%s", ENABLE_FLAG(enable), verb);
    ALOGV("applying route: %s\n", route);
    return audio_route_apply_and_update_path(adev->ar, route);
}

static int apply_device_route(struct audio_device *adev, const char* dev_name, bool enable)
{
    char route[256];
    snprintf(route, sizeof(route), PREFIX_DEVICE"|%s|%s", ENABLE_FLAG(enable), dev_name);
    ALOGV("applying route: %s\n", route);
    return audio_route_apply_and_update_path(adev->ar, route);
}

static int apply_modifier_routes(struct audio_device *adev, const char* modifier, bool enable)
{
    size_t devIdx, outIdx;
    char route[256];
    bool success;

    // try to apply simple modifier
    snprintf(route, sizeof(route), PREFIX_MODIFIER"|%s|%s", ENABLE_FLAG(enable), modifier);
    ALOGV("applying route: %s\n", route);
    if (audio_route_apply_and_update_path(adev->ar, route) == 0)
        success = true;

    for (devIdx = 0; devIdx < ARRAY_SIZE(dev_names); ++devIdx) {
        // try to apply supported output dev modifiers
        if (dev_contains_mask(adev->out_device, dev_names[devIdx].mask)) {
            snprintf(route, sizeof(route), "Modifier|%s|%s|%s", 
                        ENABLE_FLAG(enable),
                        modifier, 
                        dev_names[devIdx].name);
            ALOGV("applying route: %s\n", route);
            if (audio_route_apply_and_update_path(adev->ar, route) == 0)
                success = true;
        }

        // try to apply supported input dev modifiers
        if (dev_contains_mask(adev->in_device|AUDIO_DEVICE_BIT_IN, dev_names[devIdx].mask)) {
            snprintf(route, sizeof(route), "Modifier|%s|%s|%s", ENABLE_FLAG(enable), modifier, dev_names[devIdx].name);
            ALOGV("applying route: %s\n", route);
            if (audio_route_apply_and_update_path(adev->ar, route))
                success = true;
            
            // try to apply conditional input/output dev modifiers
            for (outIdx = 0; outIdx < ARRAY_SIZE(dev_names); ++outIdx) {
                if (dev_contains_mask(adev->out_device, dev_names[outIdx].mask)) {
                    snprintf(route, sizeof(route), "Modifier|%s|%s|%s|%s", 
                             ENABLE_FLAG(enable),
                             modifier, 
                             dev_names[devIdx].name, dev_names[outIdx].name);
                    ALOGV("applying route: %s\n", route);
                    if (audio_route_apply_and_update_path(adev->ar, route) == 0)
                        success = true;
                }
            }
        }        
    }
    
    // true if one route was applied
    return (success ? 0 : -1);
}

static const char* get_output_device_name(struct audio_device *adev)
{
    switch(adev->out_device) {
        case AUDIO_DEVICE_OUT_SPEAKER:
            return "AUDIO_DEVICE_OUT_SPEAKER";
        case AUDIO_DEVICE_OUT_WIRED_HEADSET:
            return "AUDIO_DEVICE_OUT_WIRED_HEADSET";
        case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
            return "AUDIO_DEVICE_OUT_WIRED_HEADPHONE";
        case AUDIO_DEVICE_OUT_EARPIECE:
            return "AUDIO_DEVICE_OUT_EARPIECE";
        case AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET:
            return "AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET";
        case AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET:
            return "AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET";
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
            return "AUDIO_DEVICE_OUT_BLUETOOTH_SCO";
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
            return "AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET";
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
            return "AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT";
        case AUDIO_DEVICE_OUT_ALL_SCO:
            return "AUDIO_DEVICE_OUT_ALL_SCO";
        case AUDIO_DEVICE_OUT_USB_ACCESSORY:
            return "AUDIO_DEVICE_OUT_USB_ACCESSORY";
        case AUDIO_DEVICE_OUT_USB_DEVICE:
            return "AUDIO_DEVICE_OUT_USB_DEVICE";
        default:
            return "AUDIO_DEVICE_OUT_ALL";
    }
}

static const char* get_input_device_name(struct audio_device *adev)
{
    switch(AUDIO_DEVICE_BIT_IN | adev->in_device) {
        case AUDIO_DEVICE_IN_BUILTIN_MIC:
            return "AUDIO_DEVICE_IN_BUILTIN_MIC";
        case AUDIO_DEVICE_IN_BACK_MIC:
            return "AUDIO_DEVICE_IN_BACK_MIC and AUDIO_DEVICE_IN_BUILTIN_MIC";
        case AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET:
            return "AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET";
        case AUDIO_DEVICE_IN_WIRED_HEADSET:
            return "AUDIO_DEVICE_IN_WIRED_HEADSET";
        default:
            return "UNKNOWN";
    }
}

/* Must be called with lock */
void select_devices(struct audio_device *adev)
{
    size_t i;
    
    if (adev->active_out_device == adev->out_device && adev->active_in_device == adev->in_device)
        return;

    ALOGV("Changing output device %x => %x (%s)\n", adev->active_out_device, adev->out_device, get_output_device_name(adev));
    ALOGV("Changing input device %x => %x (%s)\n", adev->active_in_device, adev->in_device, get_input_device_name(adev));
    
#if 0
    if (((AUDIO_DEVICE_BIT_IN | adev->in_device) == AUDIO_DEVICE_IN_BACK_MIC) || // Force use both mics for video recording
       (((AUDIO_DEVICE_BIT_IN | adev->in_device) == AUDIO_DEVICE_IN_BUILTIN_MIC) && adev->two_mic)) // two mic control
    {
            adev->in_device = (AUDIO_DEVICE_IN_BACK_MIC | AUDIO_DEVICE_IN_BUILTIN_MIC) & ~AUDIO_DEVICE_BIT_IN;
    }
#endif
    
    audio_route_reset(adev->ar);    
    audio_route_update_mixer(adev->ar);

    apply_modifier_routes(adev, MODIFIER_CODEC_RX_MUTE, true); 
    
    // each device in mixer_paths.xml represents one bit in the device mask (no compund devices)
    for (i = 0; i < ARRAY_SIZE(dev_names); ++i) {
        if (dev_contains_mask(adev->in_device|AUDIO_DEVICE_BIT_IN, dev_names[i].mask) ||
            dev_contains_mask(adev->out_device, dev_names[i].mask)) 
        {
            apply_device_route(adev, dev_names[i].name, true);
        }
    }
    
    if (adev->mode == AUDIO_MODE_IN_CALL) {
        apply_verb_route(adev, VERB_VOICECALL, true);
        apply_modifier_routes(adev, MODIFIER_INCALL, true);
        apply_modifier_routes(adev, MODIFIER_INCALL_IN, true);
        set_incall_device(adev);        
    } 
    else 
    {
        apply_verb_route(adev, VERB_NORMAL, true);
        apply_modifier_routes(adev, MODIFIER_NORMAL, true);
    }
 
    apply_modifier_routes(adev, MODIFIER_CODEC_RX_MUTE, false); 
    
    adev->active_out_device = adev->out_device;
    adev->active_in_device = adev->in_device;
}

static int start_call(struct audio_device *adev)
{
    ALOGV("Opening modem PCMs");
    int bt_on;

    bt_on = adev->out_device & AUDIO_DEVICE_OUT_ALL_SCO;

    if (bt_on) {
       /* use amr-nb for bluetooth */
       pcm_config_vx.rate = VX_NB_SAMPLING_RATE;
    } else {
       pcm_config_vx.rate = VX_NB_SAMPLING_RATE;//adev->wb_amr ? VX_WB_SAMPLING_RATE : VX_NB_SAMPLING_RATE;
    }

    /* Open modem PCM channels */
    if (adev->pcm_modem_dl == NULL) {
        ALOGD("Opening PCM modem DL stream");
            adev->pcm_modem_dl = pcm_open(CARD_DEFAULT, PORT_MODEM, PCM_OUT, &pcm_config_vx);
        if (!pcm_is_ready(adev->pcm_modem_dl)) {
            ALOGE("cannot open PCM modem DL stream: %s", pcm_get_error(adev->pcm_modem_dl));
            goto err_open_dl;
        }
        if (pcm_prepare(adev->pcm_modem_dl) != 0) {
            ALOGE("cannot open PCM modem DL stream: %s", pcm_get_error(adev->pcm_modem_dl));
            goto err_open_dl;
        }            
    }

    if (adev->pcm_modem_ul == NULL) {
        ALOGD("Opening PCM modem UL stream");
        adev->pcm_modem_ul = pcm_open(CARD_DEFAULT, PORT_MODEM, PCM_IN, &pcm_config_vx);
        if (!pcm_is_ready(adev->pcm_modem_ul)) {
            ALOGE("cannot open PCM modem UL stream: %s", pcm_get_error(adev->pcm_modem_ul));
            goto err_open_ul;
        }
        if (pcm_prepare(adev->pcm_modem_ul) != 0) {
            ALOGE("cannot open PCM modem UL stream: %s", pcm_get_error(adev->pcm_modem_ul));
            goto err_open_ul;
        }            
    }

    ALOGD("Starting PCM modem streams");
    //pcm_start(adev->pcm_modem_dl);
    //pcm_start(adev->pcm_modem_ul);

    /* Open bluetooth PCM channels */
    if (bt_on) {
        ALOGV("Opening bluetooth PCMs");

        if (adev->pcm_bt_dl == NULL) {
            ALOGD("Opening PCM bluetooth DL stream");
            adev->pcm_bt_dl = pcm_open(CARD_DEFAULT, PORT_BT, PCM_OUT, &pcm_config_vx);
            if (!pcm_is_ready(adev->pcm_bt_dl)) {
                ALOGE("cannot open PCM bluetooth DL stream: %s", pcm_get_error(adev->pcm_bt_dl));
                goto err_open_dl;
            }
            if (pcm_prepare(adev->pcm_bt_dl) != 0) {
                ALOGE("cannot open PCM bluetooth DL stream: %s", pcm_get_error(adev->pcm_bt_dl));
                goto err_open_ul;
            }            
        }

        if (adev->pcm_bt_ul == NULL) {
            ALOGD("Opening PCM bluetooth UL stream");
            adev->pcm_bt_ul = pcm_open(CARD_DEFAULT, PORT_BT, PCM_IN, &pcm_config_vx);
            if (!pcm_is_ready(adev->pcm_bt_ul)) {
                ALOGE("cannot open PCM bluetooth UL stream: %s", pcm_get_error(adev->pcm_bt_ul));
                goto err_open_ul;
            }
            if (pcm_prepare(adev->pcm_bt_ul) != 0) {
                ALOGE("cannot open PCM bluetooth UL stream: %s", pcm_get_error(adev->pcm_bt_ul));
                goto err_open_ul;
            }            
        }
        ALOGD("Starting PCM bluetooth streams");
        //pcm_start(adev->pcm_bt_dl);
        //pcm_start(adev->pcm_bt_ul);
    }

    return 0;

err_open_ul:
    pcm_close(adev->pcm_modem_ul);
    adev->pcm_modem_ul = NULL;
    pcm_close(adev->pcm_bt_ul);
    adev->pcm_bt_ul = NULL;
err_open_dl:
    pcm_close(adev->pcm_modem_dl);
    adev->pcm_modem_dl = NULL;
    pcm_close(adev->pcm_bt_dl);
    adev->pcm_bt_dl = NULL;

    return -ENOMEM;
}

static void end_call(struct audio_device *adev)
{
    int bt_on;
    bt_on = adev->out_device & AUDIO_DEVICE_OUT_ALL_SCO;

    if (adev->pcm_modem_dl != NULL) {
        ALOGD("Stopping modem DL PCM");
        pcm_stop(adev->pcm_modem_dl);
        ALOGV("Closing modem DL PCM");
        pcm_close(adev->pcm_modem_dl);
    }
    if (adev->pcm_modem_ul != NULL) {
        ALOGD("Stopping modem UL PCM");
        pcm_stop(adev->pcm_modem_ul);
        ALOGV("Closing modem UL PCM");
        pcm_close(adev->pcm_modem_ul);
    }
    adev->pcm_modem_dl = NULL;
    adev->pcm_modem_ul = NULL;

    if (bt_on) {
        if (adev->pcm_bt_dl != NULL) {
            ALOGD("Stopping bluetooth DL PCM");
            pcm_stop(adev->pcm_bt_dl);
            ALOGV("Closing bluetooth DL PCM");
            pcm_close(adev->pcm_bt_dl);
        }
        if (adev->pcm_bt_ul != NULL) {
            ALOGD("Stopping bluetooth UL PCM");
            pcm_stop(adev->pcm_bt_ul);
            ALOGV("Closing bluetooth UL PCM");
            pcm_close(adev->pcm_bt_ul);
        }
    }
    adev->pcm_bt_dl = NULL;
    adev->pcm_bt_ul = NULL;
}

void set_incall_device(struct audio_device *adev)
{
    enum ril_audio_path device_type;

    switch(adev->out_device) {
        case AUDIO_DEVICE_OUT_SPEAKER:
        case AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET:
        case AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET:
        case AUDIO_DEVICE_OUT_AUX_DIGITAL:
            device_type = SOUND_AUDIO_PATH_SPEAKER;
            break;
        case AUDIO_DEVICE_OUT_EARPIECE:
            device_type = SOUND_AUDIO_PATH_EARPIECE;
            break;
        case AUDIO_DEVICE_OUT_WIRED_HEADSET:
            device_type = SOUND_AUDIO_PATH_HEADSET;
            break;
        case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
            device_type = SOUND_AUDIO_PATH_HEADPHONE;
            break;
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
        case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
            if (adev->bluetooth_nrec) {
                device_type = SOUND_AUDIO_PATH_BLUETOOTH;
                //TODO:
                //device_type = SOUND_AUDIO_PATH_BLUETOOTH_WB,
            } else {
                device_type = SOUND_AUDIO_PATH_BLUETOOTH_NO_NR;
                //TODO:
                //device_type = SOUND_AUDIO_PATH_BLUETOOTH_WB_NO_NR            
            }
            break;
        case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP:
            device_type = SOUND_AUDIO_PATH_BLUETOOTH;
            break;
        default:
            /* if output device isn't supported, use handset by default */
            device_type = SOUND_AUDIO_PATH_EARPIECE;
            break;
    }

    ALOGV("%s: ril_set_call_audio_path(%d)", __func__, device_type);

    /* TODO: Figure out which devices need EXTRA_VOLUME_PATH set */
    ril_set_call_audio_path(&adev->ril, device_type, ORIGINAL_PATH);
    ril_set_two_mic_control(&adev->ril, AUDIENCE, TWO_MIC_SOLUTION_OFF);
}

static void force_all_standby(struct audio_device *adev)
{
    struct stream_in *in;
    struct stream_out *out;

    /* only needed for low latency output streams as other streams are not used
     * for voice use cases */
    if (adev->outputs[OUTPUT_LOW_LATENCY] != NULL &&
            !adev->outputs[OUTPUT_LOW_LATENCY]->standby) {
        out = adev->outputs[OUTPUT_LOW_LATENCY];
        pthread_mutex_lock(&out->lock);
        do_output_standby(out);
        pthread_mutex_unlock(&out->lock);
    }

    if (adev->active_input) {
        in = adev->active_input;
        pthread_mutex_lock(&in->lock);
        do_input_standby(in);
        pthread_mutex_unlock(&in->lock);
    }
}

static void select_mode(struct audio_device *adev)
{
    if (adev->mode == AUDIO_MODE_IN_CALL) {
        ALOGE("Entering IN_CALL state, in_call=%d", adev->in_call);
        if (!adev->in_call) {
            if (adev->out_device == AUDIO_DEVICE_NONE ||
                adev->out_device == AUDIO_DEVICE_OUT_SPEAKER) {
                adev->out_device = AUDIO_DEVICE_OUT_EARPIECE;
            }
            adev->in_device = AUDIO_DEVICE_IN_BUILTIN_MIC & ~AUDIO_DEVICE_BIT_IN;

            select_devices(adev);            
            ril_set_call_clock_sync(&adev->ril, SOUND_CLOCK_START);
            adev_set_voice_volume(&adev->hw_device, adev->voice_volume);
            ril_set_mute(&adev->ril, RX_UNMUTE);
            ril_set_mute(&adev->ril, TX_UNMUTE);
            start_call(adev);
            
            adev->in_call = true;
        }
    } else {
        ALOGE("Leaving IN_CALL state, in_call=%d, mode=%d",
             adev->in_call, adev->mode);
        
        if (adev->in_call) {
            adev->in_call = false;
            
            end_call(adev);
            ril_set_call_clock_sync(&adev->ril, SOUND_CLOCK_STOP);
            force_all_standby(adev);

            //Force Input Standby
            adev->in_device = AUDIO_DEVICE_NONE;
            select_devices(adev);
        }
    }
}

/* must be called with hw device and output stream mutexes locked */
static int start_output_stream_deep_buffer(struct stream_out *out)
{
    struct audio_device *adev = out->dev;

    if (adev->mode != AUDIO_MODE_IN_CALL) {
        select_devices(adev);
    }

    out->write_threshold = PLAYBACK_DEEP_BUFFER_LONG_PERIOD_COUNT * DEEP_BUFFER_LONG_PERIOD_SIZE;
    out->use_long_periods = true;

    ALOGE("%s: open pcm_out driver %d", __func__ , PORT_PLAYBACK);
    
    out->config[PCM_NORMAL] = pcm_config_mm;
    out->config[PCM_NORMAL].rate = MM_FULL_POWER_SAMPLING_RATE;
    out->pcm[PCM_NORMAL] = pcm_open(CARD_DEFAULT, PORT_PLAYBACK,
                                        PCM_OUT | PCM_MMAP | PCM_NOIRQ, &out->config[PCM_NORMAL]);
    if (out->pcm[PCM_NORMAL] && !pcm_is_ready(out->pcm[PCM_NORMAL])) {
        ALOGE("%s: cannot open pcm_out driver: %s", __func__, pcm_get_error(out->pcm[PCM_NORMAL]));
        pcm_close(out->pcm[PCM_NORMAL]);
        out->pcm[PCM_NORMAL] = NULL;
        return -ENOMEM;
    }
    out->buffer_frames = DEEP_BUFFER_SHORT_PERIOD_SIZE * 2;
    if (out->buffer == NULL)
        out->buffer = malloc(PLAYBACK_DEEP_BUFFER_LONG_PERIOD_COUNT * DEEP_BUFFER_LONG_PERIOD_SIZE);

    return 0;
}

static int check_input_parameters(uint32_t sample_rate, audio_format_t format, int channel_count)
{
    if (format != AUDIO_FORMAT_PCM_16_BIT)
        return -EINVAL;

    if ((channel_count < 1) || (channel_count > 2))
        return -EINVAL;

    switch(sample_rate) {
    case 8000:
    case 11025:
    case 16000:
    case 22050:
    case 24000:
    case 32000:
    case 44100:
    case 48000:
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static size_t get_input_buffer_size(uint32_t sample_rate, audio_format_t format, int channel_count)
{
    size_t size;
    size_t device_rate;

    if (check_input_parameters(sample_rate, format, channel_count) != 0)
        return 0;

    /* take resampling into account and return the closest majoring
    multiple of 16 frames, as audioflinger expects audio buffers to
    be a multiple of 16 frames */
    size = (pcm_config_capture.period_size * sample_rate) / pcm_config_capture.rate;
    size = ((size + 15) / 16) * 16;

    return size * channel_count * sizeof(short);
}

/* API functions */

static uint32_t out_get_sample_rate(const struct audio_stream *stream)
{
    return DEFAULT_OUT_SAMPLING_RATE;
}

static int out_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
    return -ENOSYS;
}

static size_t out_get_buffer_size_deep_buffer(const struct audio_stream *stream)
{
    struct stream_out *out = (struct stream_out *)stream;

    /* take resampling into account and return the closest majoring
    multiple of 16 frames, as audioflinger expects audio buffers to
    be a multiple of 16 frames. Note: we use the default rate here
    from pcm_config_mm.rate. */
    size_t size = (DEEP_BUFFER_SHORT_PERIOD_SIZE * DEFAULT_OUT_SAMPLING_RATE) /
                        pcm_config_mm.rate;
    size = ((size + 15) / 16) * 16;
    return size * audio_stream_out_frame_size((struct audio_stream *)stream);
}

static audio_channel_mask_t out_get_channels(const struct audio_stream *stream)
{
    struct stream_out *out = (struct stream_out *)stream;

    return out->channel_mask;
}

static audio_format_t out_get_format(const struct audio_stream *stream)
{
    return AUDIO_FORMAT_PCM_16_BIT;
}

static int out_set_format(struct audio_stream *stream, audio_format_t format)
{
    return -ENOSYS;
}

/* must be called with hw device and output stream mutexes locked */
static int do_output_standby(struct stream_out *out)
{
    struct audio_device *adev = out->dev;
    int i;
    bool all_outputs_in_standby = true;

    if (!out->standby) {
        for (i = 0; i < PCM_TOTAL; i++) {
            if (out->pcm[i]) {
                pcm_close(out->pcm[i]);
                out->pcm[i] = NULL;
            }
        }
        out->standby = true;

        for (i = 0; i < OUTPUT_TOTAL; i++) {
            if (adev->outputs[i] != NULL && !adev->outputs[i]->standby) {
                all_outputs_in_standby = false;
                break;
            }
        }

        /* force standby on low latency output stream so that it can reuse HDMI driver if
         * necessary when restarted */
        if (out == adev->outputs[OUTPUT_HDMI]) {
            if (adev->outputs[OUTPUT_LOW_LATENCY] != NULL &&
                    !adev->outputs[OUTPUT_LOW_LATENCY]->standby) {
                struct stream_out *ll_out = adev->outputs[OUTPUT_LOW_LATENCY];
                pthread_mutex_lock(&ll_out->lock);
                do_output_standby(ll_out);
                pthread_mutex_unlock(&ll_out->lock);
            }
        }
   }
    return 0;
}

static int out_standby(struct audio_stream *stream)
{
    struct stream_out *out = (struct stream_out *)stream;
    int status;

    pthread_mutex_lock(&out->dev->lock);
    pthread_mutex_lock(&out->lock);
    status = do_output_standby(out);
    pthread_mutex_unlock(&out->lock);
    pthread_mutex_unlock(&out->dev->lock);
    return status;
}

static int out_dump(const struct audio_stream *stream, int fd)
{
    return 0;
}

static int out_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
    struct stream_out *out = (struct stream_out *)stream;
    struct audio_device *adev = out->dev;
    struct stream_in *in;
    struct str_parms *parms;
    char *str;
    char value[32];
    int ret, val = 0;
    bool force_input_standby = false;

    parms = str_parms_create_str(kvpairs);

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING,
                            value, sizeof(value));
    if (ret >= 0) {
        val = atoi(value);

        pthread_mutex_lock(&adev->lock);
        pthread_mutex_lock(&out->lock);
        if (((adev->out_device) != val) && (val != 0)) {
            /* this is needed only when changing device on low latency output
             * as other output streams are not used for voice use cases nor
             * handle duplication to HDMI or SPDIF */
            if (out == adev->outputs[OUTPUT_LOW_LATENCY] && !out->standby) {
                /* a change in output device may change the microphone selection */
                if (adev->active_input &&
                        adev->active_input->source == AUDIO_SOURCE_VOICE_COMMUNICATION) {
                    force_input_standby = true;
                }
                /* force standby if moving to/from HDMI/SPDIF or if the output
                 * device changes when in HDMI/SPDIF mode */
                /* FIXME also force standby when in call as some audio path switches do not work
                 * while in call and an output stream is active (e.g BT SCO => earpiece) */

                /* FIXME workaround for audio being dropped when switching path without forcing standby
                 * (several hundred ms of audio can be lost: e.g beginning of a ringtone. We must understand
                 * the root cause in audio HAL, driver or ABE.
                if (((val & AUDIO_DEVICE_OUT_AUX_DIGITAL) ^
                        (adev->out_device & AUDIO_DEVICE_OUT_AUX_DIGITAL)) ||
                        ((val & AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET) ^
                        (adev->out_device & AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET)) ||
                        (adev->out_device & (AUDIO_DEVICE_OUT_AUX_DIGITAL |
                                         AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET)))
                */
                if (((val & AUDIO_DEVICE_OUT_AUX_DIGITAL) ^
                        (adev->out_device & AUDIO_DEVICE_OUT_AUX_DIGITAL)) ||
                        ((val & AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET) ^
                        (adev->out_device & AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET)) ||
                        (adev->out_device & (AUDIO_DEVICE_OUT_AUX_DIGITAL |
                                         AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET)) ||
                        ((val & AUDIO_DEVICE_OUT_SPEAKER) ^
                        (adev->out_device & AUDIO_DEVICE_OUT_SPEAKER)) ||
                        (adev->mode == AUDIO_MODE_IN_CALL))
                    do_output_standby(out);
            }
            if (out != adev->outputs[OUTPUT_HDMI]) {
                adev->out_device = val;
                select_devices(adev);
            }
        }
        pthread_mutex_unlock(&out->lock);
        if (force_input_standby) {
            in = adev->active_input;
            pthread_mutex_lock(&in->lock);
            do_input_standby(in);
            pthread_mutex_unlock(&in->lock);
        }
        pthread_mutex_unlock(&adev->lock);
    }

    str_parms_destroy(parms);
    return ret;
}

static char *out_get_parameters(const struct audio_stream *stream, const char *keys)
{
    struct stream_out *out = (struct stream_out *)stream;
    struct str_parms *query = str_parms_create_str(keys);
    char *str;
    char value[256];
    struct str_parms *reply = str_parms_create();
    size_t i, j;
    int ret;
    bool first = true;

    ret = str_parms_get_str(query, AUDIO_PARAMETER_STREAM_SUP_CHANNELS, value, sizeof(value));
    if (ret >= 0) {
        value[0] = '\0';
        i = 0;
        while (out->sup_channel_masks[i] != 0) {
            for (j = 0; j < ARRAY_SIZE(out_channels_name_to_enum_table); j++) {
                if (out_channels_name_to_enum_table[j].value == out->sup_channel_masks[i]) {
                    if (!first) {
                        strcat(value, "|");
                    }
                    strcat(value, out_channels_name_to_enum_table[j].name);
                    first = false;
                    break;
                }
            }
            i++;
        }
        str_parms_add_str(reply, AUDIO_PARAMETER_STREAM_SUP_CHANNELS, value);
        str = strdup(str_parms_to_str(reply));
    } else {
        str = strdup(keys);
    }

    str_parms_destroy(query);
    str_parms_destroy(reply);
    return str;
}

static uint32_t out_get_latency_deep_buffer(const struct audio_stream_out *stream)
{
    struct stream_out *out = (struct stream_out *)stream;

    /*  Note: we use the default rate here from pcm_config_mm.rate */
    return (DEEP_BUFFER_LONG_PERIOD_SIZE * PLAYBACK_DEEP_BUFFER_LONG_PERIOD_COUNT * 1000) /
                    pcm_config_mm.rate;
}

static int out_set_volume(struct audio_stream_out *stream, float left,
                          float right)
{
    return -ENOSYS;
}

static ssize_t out_write_deep_buffer(struct audio_stream_out *stream, const void* buffer,
                         size_t bytes)
{
    int ret;
    struct stream_out *out = (struct stream_out *)stream;
    struct audio_device *adev = out->dev;
    size_t frame_size = audio_stream_out_frame_size(&out->stream.common);
    size_t in_frames = bytes / frame_size;
    size_t out_frames;
    bool use_long_periods;
    int kernel_frames;
    void *buf;

    /* acquiring hw device mutex systematically is useful if a low priority thread is waiting
     * on the output stream mutex - e.g. executing select_mode() while holding the hw device
     * mutex
     */
    pthread_mutex_lock(&adev->lock);
    pthread_mutex_lock(&out->lock);
    if (out->standby) {
        ret = start_output_stream_deep_buffer(out);
        if (ret != 0) {
            pthread_mutex_unlock(&adev->lock);
            goto exit;
        }
        out->standby = 0;
    }
    use_long_periods = adev->screen_off && !adev->active_input;
    pthread_mutex_unlock(&adev->lock);

    if (use_long_periods != out->use_long_periods) {
        size_t period_size;
        size_t period_count;

        if (use_long_periods) {
            period_size = DEEP_BUFFER_LONG_PERIOD_SIZE;
            period_count = PLAYBACK_DEEP_BUFFER_LONG_PERIOD_COUNT;
        } else {
            period_size = DEEP_BUFFER_SHORT_PERIOD_SIZE;
            period_count = PLAYBACK_DEEP_BUFFER_SHORT_PERIOD_COUNT;
        }
        out->write_threshold = period_size * period_count;
        pcm_set_avail_min(out->pcm[PCM_NORMAL], period_size);
        out->use_long_periods = use_long_periods;
    }

    /* only use resampler if required */
    if (out->config[PCM_NORMAL].rate != DEFAULT_OUT_SAMPLING_RATE) {
        out_frames = out->buffer_frames;
        out->resampler->resample_from_input(out->resampler,
                                            (int16_t *)buffer,
                                            &in_frames,
                                            (int16_t *)out->buffer,
                                            &out_frames);
        buf = (void *)out->buffer;
    } else {
        out_frames = in_frames;
        buf = (void *)buffer;
    }

    /* do not allow more than out->write_threshold frames in kernel pcm driver buffer */
    do {
        struct timespec time_stamp;

        if (pcm_get_htimestamp(out->pcm[PCM_NORMAL],
                               (unsigned int *)&kernel_frames, &time_stamp) < 0)
            break;
        kernel_frames = pcm_get_buffer_size(out->pcm[PCM_NORMAL]) - kernel_frames;

        if (kernel_frames > out->write_threshold) {
            unsigned long time = (unsigned long)
                    (((int64_t)(kernel_frames - out->write_threshold) * 1000000) /
                            MM_FULL_POWER_SAMPLING_RATE);
            if (time < MIN_WRITE_SLEEP_US)
                time = MIN_WRITE_SLEEP_US;
            usleep(time);
        }
    } while (kernel_frames > out->write_threshold);

    ret = pcm_mmap_write(out->pcm[PCM_NORMAL], buf, out_frames * frame_size);

exit:
    pthread_mutex_unlock(&out->lock);

    if (ret != 0) {
        usleep(bytes * 1000000 / audio_stream_out_frame_size(&stream->common) /
               out_get_sample_rate(&stream->common));
    }

    return bytes;
}

static int out_get_render_position(const struct audio_stream_out *stream,
                                   uint32_t *dsp_frames)
{
    return -EINVAL;
}

/** audio_stream_in implementation **/

/* must be called with hw device and input stream mutexes locked */
static int start_input_stream(struct stream_in *in)
{
    int ret = 0;
    struct audio_device *adev = in->dev;

    adev->active_input = in;

    if (adev->mode != AUDIO_MODE_IN_CALL) {
        adev->in_device = in->device;
        select_devices(adev);
    }

    ALOGE("open pcm_plin: %d\n", PORT_CAPTURE);

    /* this assumes routing is done previously */
    in->pcm = pcm_open(CARD_DEFAULT, PORT_CAPTURE, PCM_IN, &in->config);
    if (!pcm_is_ready(in->pcm)) {
        ALOGE("cannot open pcm_in driver: %s", pcm_get_error(in->pcm));
        pcm_close(in->pcm);
        adev->active_input = NULL;
        return -ENOMEM;
    }

    /* force read and proc buf reallocation case of frame size or channel count change */
    in->read_buf_frames = 0;
    in->read_buf_size = 0;
    in->proc_buf_frames = 0;
    in->proc_buf_size = 0;
    /* if no supported sample rate is available, use the resampler */
    if (in->resampler) {
        in->resampler->reset(in->resampler);
    }
    return 0;
}

/** audio_stream_in implementation **/
static uint32_t in_get_sample_rate(const struct audio_stream *stream)
{
    struct stream_in *in = (struct stream_in *)stream;

    return in->requested_rate;
}

static int in_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
    return 0;
}

static audio_channel_mask_t in_get_channels(const struct audio_stream *stream)
{
    struct stream_in *in = (struct stream_in *)stream;

    return in->main_channels;
}


static size_t in_get_buffer_size(const struct audio_stream *stream)
{
    struct stream_in *in = (struct stream_in *)stream;

    return get_input_buffer_size(in->requested_rate,
                                 AUDIO_FORMAT_PCM_16_BIT,
                                 popcount(in->main_channels));
}

static audio_format_t in_get_format(const struct audio_stream *stream)
{
    return AUDIO_FORMAT_PCM_16_BIT;
}

static int in_set_format(struct audio_stream *stream, audio_format_t format)
{
    return -ENOSYS;
}

/* must be called with hw device and input stream mutexes locked */
static int do_input_standby(struct stream_in *in)
{
    struct audio_device *adev = in->dev;
  
    if (!in->standby) {
        pcm_close(in->pcm);
        in->pcm = NULL;

        adev->active_input = 0;
        if (adev->mode != AUDIO_MODE_IN_CALL) {
            adev->in_device = AUDIO_DEVICE_NONE;
            select_devices(adev);
        }

        in->standby = true;
    }
    return 0;
}

static int in_standby(struct audio_stream *stream)
{
    struct stream_in *in = (struct stream_in *)stream;
    int status;

    pthread_mutex_lock(&in->dev->lock);
    pthread_mutex_lock(&in->lock);
    status = do_input_standby(in);
    pthread_mutex_unlock(&in->lock);
    pthread_mutex_unlock(&in->dev->lock);
    return status;
}

static int in_dump(const struct audio_stream *stream, int fd)
{
    return 0;
}

static int in_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
    struct stream_in *in = (struct stream_in *)stream;
    struct audio_device *adev = in->dev;
    struct str_parms *parms;
    char value[32];
    int ret;
    int val;
    bool do_standby = false;

    parms = str_parms_create_str(kvpairs);

    pthread_mutex_lock(&adev->lock);
    pthread_mutex_lock(&in->lock);
    ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_INPUT_SOURCE,
                            value, sizeof(value));
    if (ret >= 0) {
        val = atoi(value);
        /* no audio source uses val == 0 */
        if ((in->source != val) && (val != 0)) {
            in->source = val;
            do_standby = true;
        }
    }

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING,
                            value, sizeof(value));
    if (ret >= 0) {
        /* strip AUDIO_DEVICE_BIT_IN to allow bitwise comparisons */
        val = atoi(value) & ~AUDIO_DEVICE_BIT_IN;
        /* no audio device uses val == 0 */
        if ((in->device != val) && (val != 0)) {
            in->device = val;
            do_standby = true;
        }
    }

    if (do_standby)
        do_input_standby(in);
    pthread_mutex_unlock(&in->lock);
    pthread_mutex_unlock(&adev->lock);

    str_parms_destroy(parms);
    return ret;
}

static char * in_get_parameters(const struct audio_stream *stream,
                                const char *keys)
{
    return strdup("");
}

static int in_set_gain(struct audio_stream_in *stream, float gain)
{
    return 0;
}

static int get_next_buffer(struct resampler_buffer_provider *buffer_provider,
                                   struct resampler_buffer* buffer)
{
    struct stream_in *in;

    if (buffer_provider == NULL || buffer == NULL)
        return -EINVAL;

    in = (struct stream_in *)((char *)buffer_provider -
                                   offsetof(struct stream_in, buf_provider));

    if (in->pcm == NULL) {
        buffer->raw = NULL;
        buffer->frame_count = 0;
        in->read_status = -ENODEV;
        return -ENODEV;
    }

    if (in->read_buf_frames == 0) {
        size_t size_in_bytes = pcm_frames_to_bytes(in->pcm, in->config.period_size);
        if (in->read_buf_size < in->config.period_size) {
            in->read_buf_size = in->config.period_size;
            in->read_buf = (int16_t *) realloc(in->read_buf, size_in_bytes);
            ALOG_ASSERT((in->read_buf != NULL),
                        "%s failed to reallocate read_buf", __func__);
            ALOGV("%s: read_buf %p extended to %d bytes",
                  __func__, in->read_buf, size_in_bytes);
        }

        in->read_status = pcm_read(in->pcm, (void*)in->read_buf, size_in_bytes);

        if (in->read_status != 0) {
            ALOGE("%s: pcm_read error %d", __func__, in->read_status);
            buffer->raw = NULL;
            buffer->frame_count = 0;
            return in->read_status;
        }
        in->read_buf_frames = in->config.period_size;
    }

    buffer->frame_count = (buffer->frame_count > in->read_buf_frames) ?
                                in->read_buf_frames : buffer->frame_count;
    buffer->i16 = in->read_buf + (in->config.period_size - in->read_buf_frames) *
                                                in->config.channels;

    return in->read_status;

}

static void release_buffer(struct resampler_buffer_provider *buffer_provider,
                                  struct resampler_buffer* buffer)
{
    struct stream_in *in;

    if (buffer_provider == NULL || buffer == NULL)
        return;

    in = (struct stream_in *)((char *)buffer_provider -
                                   offsetof(struct stream_in, buf_provider));

    in->read_buf_frames -= buffer->frame_count;
}

/* read_frames() reads frames from kernel driver, down samples to capture rate
 * if necessary and output the number of frames requested to the buffer specified */
static ssize_t read_frames(struct stream_in *in, void *buffer, ssize_t frames)
{
    ssize_t frames_wr = 0;

    while (frames_wr < frames) {
        size_t frames_rd = frames - frames_wr;
        if (in->resampler != NULL) {
            in->resampler->resample_from_provider(in->resampler,
                                                  (int16_t *)((char *)buffer +
                                                      pcm_frames_to_bytes(in->pcm ,frames_wr)),
                                                  &frames_rd);

        } else {
            struct resampler_buffer buf = {
                    { raw : NULL, },
                    frame_count : frames_rd,
            };
            get_next_buffer(&in->buf_provider, &buf);
            if (buf.raw != NULL) {
                memcpy((char *)buffer +
                            pcm_frames_to_bytes(in->pcm, frames_wr),
                        buf.raw,
                        pcm_frames_to_bytes(in->pcm, buf.frame_count));
                frames_rd = buf.frame_count;
            }
            release_buffer(&in->buf_provider, &buf);
        }
        /* in->read_status is updated by getNextBuffer() also called by
         * in->resampler->resample_from_provider() */
        if (in->read_status != 0)
            return in->read_status;

        frames_wr += frames_rd;
    }
    return frames_wr;
}

static ssize_t in_read(struct audio_stream_in *stream, void* buffer,
                       size_t bytes)
{
    int ret = 0;
    struct stream_in *in = (struct stream_in *)stream;
    struct audio_device *adev = in->dev;
    size_t frames_rq = bytes / audio_stream_in_frame_size(&stream->common);
   
    /* acquiring hw device mutex systematically is useful if a low priority thread is waiting
     * on the input stream mutex - e.g. executing select_mode() while holding the hw device
     * mutex
     */
    pthread_mutex_lock(&adev->lock);
    pthread_mutex_lock(&in->lock);
    if (in->standby) {
        ret = start_input_stream(in);
        if (ret == 0)
            in->standby = 0;
    }
    pthread_mutex_unlock(&adev->lock);

    if (ret < 0)
        goto exit;

    if (in->resampler != NULL)
        ret = read_frames(in, buffer, frames_rq);
    else
        ret = pcm_read(in->pcm, buffer, bytes);

    if (ret > 0)
        ret = 0;

    if (ret == 0 && adev->mic_mute)
        memset(buffer, 0, bytes);

exit:
    if (ret < 0)
        usleep(bytes * 1000000 / audio_stream_in_frame_size(&stream->common) /
               in_get_sample_rate(&stream->common));

    pthread_mutex_unlock(&in->lock);
    return bytes;
}

static uint32_t in_get_input_frames_lost(struct audio_stream_in *stream)
{
    return 0;
}

#define GET_COMMAND_STATUS(status, fct_status, cmd_status) \
            do {                                           \
                if (fct_status != 0)                       \
                    status = fct_status;                   \
                else if (cmd_status != 0)                  \
                    status = cmd_status;                   \
            } while(0)

#define MAX_NUM_CHANNEL_CONFIGS 10

static int out_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
    return 0;
}

static int out_remove_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
    return 0;
}

static int adev_open_output_stream(struct audio_hw_device *dev,
                                   audio_io_handle_t handle,
                                   audio_devices_t devices,
                                   audio_output_flags_t flags,
                                   struct audio_config *config,
                                   struct audio_stream_out **stream_out,
                                   const char *address __unused)
{
    struct audio_device *ladev = (struct audio_device *)dev;
    struct stream_out *out;
    int ret;
    int output_type;
    *stream_out = NULL;
   
    out = (struct stream_out *)calloc(1, sizeof(struct stream_out));
    if (!out)
        return -ENOMEM;

    out->sup_channel_masks[0] = AUDIO_CHANNEL_OUT_STEREO;
    out->channel_mask = AUDIO_CHANNEL_OUT_STEREO;

    if (ladev->outputs[OUTPUT_DEEP_BUF] != NULL) {
        ret = -ENOSYS;
        goto err_open;
    }
    output_type = OUTPUT_DEEP_BUF;
    out->channel_mask = AUDIO_CHANNEL_OUT_STEREO;
    out->stream.common.get_buffer_size = out_get_buffer_size_deep_buffer;
    out->stream.common.get_sample_rate = out_get_sample_rate;
    out->stream.get_latency = out_get_latency_deep_buffer;
    out->stream.write = out_write_deep_buffer;

    ret = create_resampler(DEFAULT_OUT_SAMPLING_RATE,
                           MM_FULL_POWER_SAMPLING_RATE,
                           2,
                           RESAMPLER_QUALITY_DEFAULT,
                           NULL,
                           &out->resampler);
    if (ret != 0)
        goto err_open;

    out->stream.common.set_sample_rate = out_set_sample_rate;
    out->stream.common.get_channels = out_get_channels;
    out->stream.common.get_format = out_get_format;
    out->stream.common.set_format = out_set_format;
    out->stream.common.standby = out_standby;
    out->stream.common.dump = out_dump;
    out->stream.common.set_parameters = out_set_parameters;
    out->stream.common.get_parameters = out_get_parameters;
    out->stream.common.add_audio_effect = out_add_audio_effect;
    out->stream.common.remove_audio_effect = out_remove_audio_effect;
    out->stream.set_volume = out_set_volume;
    out->stream.get_render_position = out_get_render_position;

    out->dev = ladev;
    out->standby = 1;

    /* FIXME: when we support multiple output devices, we will want to
     * do the following:
     * adev->out_device = out->device;
     * select_devices(adev);
     * This is because out_set_parameters() with a route is not
     * guaranteed to be called after an output stream is opened. */

    config->format = out->stream.common.get_format(&out->stream.common);
    config->channel_mask = out->stream.common.get_channels(&out->stream.common);
    config->sample_rate = out->stream.common.get_sample_rate(&out->stream.common);

    *stream_out = &out->stream;
    ladev->outputs[output_type] = out;

    return 0;

err_open:
    free(out);
    return ret;
}

static void adev_close_output_stream(struct audio_hw_device *dev,
                                     struct audio_stream_out *stream)
{
    struct audio_device *adev;
    struct stream_out *out = (struct stream_out *)stream;
    enum output_type type;

    out_standby(&stream->common);
    adev = (struct audio_device *)dev;
    pthread_mutex_lock(&adev->lock);
    for (type = 0; type < OUTPUT_TOTAL; ++type) {
        if (adev->outputs[type] == out) {
            adev->outputs[type] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&adev->lock);

    if (out->buffer)
        free(out->buffer);
    if (out->resampler)
        release_resampler(out->resampler);
    free(stream);
}

static int adev_set_parameters(struct audio_hw_device *dev, const char *kvpairs)
{
    struct audio_device *adev = (struct audio_device *)dev;
    struct str_parms *parms;
    char *str;
    char value[64];
    int ret;

    parms = str_parms_create_str(kvpairs);
    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_TTY_MODE, value, sizeof(value));
    if (ret >= 0) {
        int tty_mode;

        if (strcmp(value, AUDIO_PARAMETER_VALUE_TTY_OFF) == 0)
            tty_mode = TTY_MODE_OFF;
        else if (strcmp(value, AUDIO_PARAMETER_VALUE_TTY_VCO) == 0)
            tty_mode = TTY_MODE_VCO;
        else if (strcmp(value, AUDIO_PARAMETER_VALUE_TTY_HCO) == 0)
            tty_mode = TTY_MODE_HCO;
        else if (strcmp(value, AUDIO_PARAMETER_VALUE_TTY_FULL) == 0)
            tty_mode = TTY_MODE_FULL;
        else
            return -EINVAL;

        pthread_mutex_lock(&adev->lock);
        if (tty_mode != adev->tty_mode) {
            adev->tty_mode = tty_mode;
            if (adev->mode == AUDIO_MODE_IN_CALL)
                select_devices(adev);
        }
        pthread_mutex_unlock(&adev->lock);
    }

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_BT_NREC, value, sizeof(value));
    if (ret >= 0) {
        if (strcmp(value, AUDIO_PARAMETER_VALUE_ON) == 0)
            adev->bluetooth_nrec = true;
        else
            adev->bluetooth_nrec = false;
    }

    ret = str_parms_get_str(parms, "screen_off", value, sizeof(value));
    if (ret >= 0) {
        if (strcmp(value, AUDIO_PARAMETER_VALUE_ON) == 0)
            adev->screen_off = false;
        else
            adev->screen_off = true;
    }

    ret = str_parms_get_str(parms, "test", value, sizeof(value));
    if (ret >= 0) {
        ALOGE("Got test parameter: %s", value);
        if (strcmp(value, "t0") == 0) {
            ril_set_two_mic_control(&adev->ril, AUDIENCE, TWO_MIC_SOLUTION_OFF);
        }
        else if (strcmp(value, "t1") == 0) {
            ril_set_two_mic_control(&adev->ril, AUDIENCE, TWO_MIC_SOLUTION_ON);
        }
        else if (strcmp(value, "v") == 0) {
            ril_set_call_volume(&adev->ril, SOUND_TYPE_VOICE, 1.0);
            ril_set_call_volume(&adev->ril, SOUND_TYPE_HEADSET, 1.0);
            ril_set_call_volume(&adev->ril, SOUND_TYPE_SPEAKER, 1.0);
        }
        else if (strcmp(value, "m0") == 0) {
            ril_set_mute(&adev->ril, TX_UNMUTE);
            ril_set_mute(&adev->ril, RX_UNMUTE);
        }
        else if (strcmp(value, "m1") == 0) {
            ril_set_mute(&adev->ril, TX_MUTE);
            ril_set_mute(&adev->ril, RX_MUTE);
        }
        else if (strcmp(value, "s") == 0) {
            ril_set_call_clock_sync(&adev->ril, SOUND_CLOCK_START);
        }
        else if (strncmp(value, "p", 1) == 0) {
            int path = value[1]-'0';
            ALOGE("Set audio path: %d", path);
            ril_set_call_audio_path(&adev->ril, path, 0);
        }
    }
    ret = str_parms_get_str(parms, "mixer", value, sizeof(value));
    if (ret >= 0) {
        ALOGE("Got mixer parameter: %s", value);
        audio_route_apply_and_update_path(adev->ar, value);
    }
    
    ret = str_parms_get_str(parms, "noise_suppression", value, sizeof(value));
#if 0
    if (ret >= 0) {
        if (strcmp(value, "on") == 0) {
            ALOGV("%s: enabling two mic control", __func__);
            ril_set_two_mic_control(&adev->ril, AUDIENCE, TWO_MIC_SOLUTION_ON);
            /* sub mic */
            adev->two_mic = true;
        } else {
            ALOGV("%s: disabling two mic control", __func__);
            ril_set_two_mic_control(&adev->ril, AUDIENCE, TWO_MIC_SOLUTION_OFF);
            /* sub mic */
            adev->two_mic = false;
        }
    }
#endif

#if 0
    ret = str_parms_get_str(parms, "wb_amr", value, sizeof(value));
    if (ret >= 0) {
        bool enable;
        
        if (strcmp(value, "on") == 0) {
            ALOGV("%s: enabling wb_amr", __func__);
            enable = true;
        } else {
            ALOGV("%s: disabling wb_amr", __func__);
            enable = false;
        }

        if (adev->wb_amr != enable) {
            adev->wb_amr = enable;
            
            /* reopen the modem PCMs at the new rate */
            if (adev->in_call) {
                end_call(adev);
                select_devices(adev);
                start_call(adev);
            }
        }        
    }
#endif

    ret = str_parms_get_str(parms, "volume_boost", value, sizeof(value));
    if (ret >= 0) {
        ALOGV("%s: volume_boost=%s", __func__, value);
        // TODO
    }
    
    str_parms_destroy(parms);
    return ret;
}

static char *adev_get_parameters(const struct audio_hw_device *dev,
                                 const char *keys)
{
    return strdup("");
}

static int adev_init_check(const struct audio_hw_device *dev)
{
    return 0;
}

static int adev_set_voice_volume(struct audio_hw_device *dev, float volume)
{
    struct audio_device *adev = (struct audio_device *)dev;

    adev->voice_volume = volume;

    if (adev->mode == AUDIO_MODE_IN_CALL) {
        enum ril_sound_type sound_type;

        switch (adev->out_device) {
            case AUDIO_DEVICE_OUT_SPEAKER:
                sound_type = SOUND_TYPE_SPEAKER;
                break;
            case AUDIO_DEVICE_OUT_WIRED_HEADSET:
            case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
                sound_type = SOUND_TYPE_HEADSET;
                break;
            case AUDIO_DEVICE_OUT_BLUETOOTH_SCO:
            case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET:
            case AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT:
            case AUDIO_DEVICE_OUT_ALL_SCO:
                sound_type = SOUND_TYPE_BTVOICE;
                break;
            default:
                sound_type = SOUND_TYPE_VOICE;
        }

        ALOGV("%s: set call volume: type=%d, vol=%f\n", __func__, sound_type, volume);
        ril_set_call_volume(&adev->ril, sound_type, volume);
    }

    return 0;
}

static int adev_set_master_volume(struct audio_hw_device *dev, float volume)
{
    return -ENOSYS;
}

static int adev_set_mode(struct audio_hw_device *dev, audio_mode_t mode)
{
    struct audio_device *adev = (struct audio_device *)dev;

    pthread_mutex_lock(&adev->lock);
    if (adev->mode != mode) {
        adev->mode = mode;
        select_mode(adev);
    }
    pthread_mutex_unlock(&adev->lock);

    return 0;
}

static int adev_set_mic_mute(struct audio_hw_device *dev, bool state)
{
    struct audio_device *adev = (struct audio_device *)dev;
    enum ril_mute_state mute_state = state ? TX_MUTE : TX_UNMUTE;

    ALOGV("%s: set mic mute: %d\n", __func__, state);

    adev->mic_mute = state;

    if (adev->in_call) {
        ril_set_mute(&adev->ril, mute_state);
    }

    return 0;
}

static int adev_get_mic_mute(const struct audio_hw_device *dev, bool *state)
{
    struct audio_device *adev = (struct audio_device *)dev;

    *state = adev->mic_mute;

    return 0;
}

static size_t adev_get_input_buffer_size(const struct audio_hw_device *dev,
                                         const struct audio_config *config)
{
    size_t size;
    int channel_count = popcount(config->channel_mask);
    if (check_input_parameters(config->sample_rate, config->format, channel_count) != 0)
        return 0;

    return get_input_buffer_size(config->sample_rate, config->format, channel_count);
}

static int adev_open_input_stream(struct audio_hw_device *dev,
                                  audio_io_handle_t handle,
                                  audio_devices_t devices,
                                  struct audio_config *config,
                                  struct audio_stream_in **stream_in,
                                  audio_input_flags_t flags,
                                  const char *address __unused,
                                  audio_source_t source __unused)
{
    struct audio_device *adev = (struct audio_device *)dev;
    struct stream_in *in;
    int ret;

    *stream_in = NULL;
   
    /* Respond with a request for stereo if a different format is given. */
    if (config->channel_mask != AUDIO_CHANNEL_IN_STEREO) {
        config->channel_mask = AUDIO_CHANNEL_IN_STEREO;
        return -EINVAL;
    }

    int channel_count = popcount(config->channel_mask);

    if (check_input_parameters(config->sample_rate, config->format, channel_count) != 0)
        return -EINVAL;

    in = (struct stream_in *)calloc(1, sizeof(struct stream_in));
    if (in == NULL) {
        return -ENOMEM;
    }

    in->stream.common.get_sample_rate = in_get_sample_rate;
    in->stream.common.set_sample_rate = in_set_sample_rate;
    in->stream.common.get_buffer_size = in_get_buffer_size;
    in->stream.common.get_channels = in_get_channels;
    in->stream.common.get_format = in_get_format;
    in->stream.common.set_format = in_set_format;
    in->stream.common.standby = in_standby;
    in->stream.common.dump = in_dump;
    in->stream.common.set_parameters = in_set_parameters;
    in->stream.common.get_parameters = in_get_parameters;
    in->stream.common.add_audio_effect = 0;
    in->stream.common.remove_audio_effect = 0;
    in->stream.set_gain = in_set_gain;
    in->stream.read = in_read;
    in->stream.get_input_frames_lost = in_get_input_frames_lost;

    in->dev = adev;
    in->standby = true;
    in->requested_rate = config->sample_rate;
    /* strip AUDIO_DEVICE_BIT_IN to allow bitwise comparisons */
    in->device = devices & ~AUDIO_DEVICE_BIT_IN;

    memcpy(&in->config, &pcm_config_capture, sizeof(pcm_config_capture));
    in->config.channels = channel_count;

    in->main_channels = config->channel_mask;

    if (in->requested_rate != in->config.rate) {
        in->buf_provider.get_next_buffer = get_next_buffer;
        in->buf_provider.release_buffer = release_buffer;

        ret = create_resampler(in->config.rate,
                               in->requested_rate,
                               in->config.channels,
                               RESAMPLER_QUALITY_DEFAULT,
                               &in->buf_provider,
                               &in->resampler);
        if (ret != 0) {
            ret = -EINVAL;
            goto err;
        }
    }

    *stream_in = &in->stream;
    return 0;

err:
    if (in->resampler)
        release_resampler(in->resampler);

    free(in);
    return ret;
}

static void adev_close_input_stream(struct audio_hw_device *dev,
                                   struct audio_stream_in *stream)
{
    struct stream_in *in = (struct stream_in *)stream;
    int i;
    
    in_standby(&stream->common);

    free(in->read_buf);
    if (in->resampler) {
        release_resampler(in->resampler);
        in->resampler = NULL;
    }
    if (in->proc_buf_in)
        free(in->proc_buf_in);
    if (in->proc_buf_out)
        free(in->proc_buf_out);
    if (in->ref_buf)
        free(in->ref_buf);

    free(stream);
}

static int adev_dump(const audio_hw_device_t *device, int fd)
{
    return 0;
}

static int adev_close(hw_device_t *device)
{
    struct audio_device *adev = (struct audio_device *)device;

    audio_route_free(adev->ar);
    
    /* RIL */
    ril_close(&adev->ril);

    free(device);
    return 0;
}

static int audio_pcm_clk_state_callback(struct audio_device *adev, int enable) {
    //pthread_mutex_lock(&adev->lock);
    ALOGE("### onUnsolPcmClkState : Clock - %s", (enable ? "enable" : "disable"));
    //if (enable)
    //    this->doRoutingPcmState();
    //pthread_mutex_unlock(&adev->lock);
    return 0;
}

static int audio_dha_callback(struct audio_device *adev, const dha_data_t *dha) {
    ALOGE("### onUnsolDHA: dhaOn = %d selectLR = %d###", dha->dhaOn, dha->selectLR);
    return 0;
}

static int adev_open(const hw_module_t* module, const char* name,
                     hw_device_t** device)
{
    struct audio_device *adev;
    int ret;

    if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0) {
        return -EINVAL;
    }

    adev = calloc(1, sizeof(struct audio_device));
    if (adev == NULL) {
        return -ENOMEM;
    }

    adev->hw_device.common.tag = HARDWARE_DEVICE_TAG;
    adev->hw_device.common.version = AUDIO_DEVICE_API_VERSION_2_0;
    adev->hw_device.common.module = (struct hw_module_t *) module;
    adev->hw_device.common.close = adev_close;

    adev->hw_device.init_check = adev_init_check;
    adev->hw_device.set_voice_volume = adev_set_voice_volume;
    adev->hw_device.set_master_volume = adev_set_master_volume;
    adev->hw_device.set_mode = adev_set_mode;
    adev->hw_device.set_mic_mute = adev_set_mic_mute;
    adev->hw_device.get_mic_mute = adev_get_mic_mute;
    adev->hw_device.set_parameters = adev_set_parameters;
    adev->hw_device.get_parameters = adev_get_parameters;
    adev->hw_device.get_input_buffer_size = adev_get_input_buffer_size;
    adev->hw_device.open_output_stream = adev_open_output_stream;
    adev->hw_device.close_output_stream = adev_close_output_stream;
    adev->hw_device.open_input_stream = adev_open_input_stream;
    adev->hw_device.close_input_stream = adev_close_input_stream;
    adev->hw_device.dump = adev_dump;
    adev->hw_device.set_master_mute = NULL;
    adev->hw_device.get_master_mute = NULL;

    adev->ar = audio_route_init(CARD_DEFAULT, NULL);

    /* Set the default route before the PCM stream is opened */
    pthread_mutex_lock(&adev->lock);
    adev->mode = AUDIO_MODE_NORMAL;
    adev->out_device = AUDIO_DEVICE_OUT_SPEAKER;
    adev->in_device = AUDIO_DEVICE_NONE;
    select_devices(adev);

    adev->pcm_modem_dl = NULL;
    adev->pcm_modem_ul = NULL;
    adev->pcm_bt_dl = NULL;
    adev->pcm_bt_ul = NULL;
    adev->voice_volume = 1.0f;
    adev->tty_mode = TTY_MODE_OFF;
    adev->bluetooth_nrec = true;
    adev->wb_amr = false;
    adev->two_mic = false;

    /* RIL */
    ril_open(&adev->ril);
    pthread_mutex_unlock(&adev->lock);
    
    /* TODO: remove, as set_parameters() is used instead */
    //ril_register_set_wb_amr_callback(audio_set_wb_amr_callback, (void *)adev);
    ril_register_dha_callback(audio_dha_callback, (void *)adev);
    ril_register_pcm_clk_state_callback(audio_pcm_clk_state_callback, (void *)adev);

    *device = &adev->hw_device.common;

    return 0;

err:
    return -EINVAL;
}

static struct hw_module_methods_t hal_module_methods = {
    .open = adev_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "SMDK3xxx audio HW HAL",
        .author = "The CyanogenMod Project",
        .methods = &hal_module_methods,
    },
};
