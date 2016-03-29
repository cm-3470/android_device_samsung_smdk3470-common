/* //device/libs/telephony/ril_unsol_commands.h
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
    {SAMSUNG_UNSOL_RESPONSE_BASE, NULL, WAKE_PARTIAL},
    {RIL_UNSOL_RELEASE_COMPLETE_MESSAGE, responseVoid, WAKE_PARTIAL}, // 11001
    {RIL_UNSOL_STK_SEND_SMS_RESULT, responseInts, WAKE_PARTIAL}, // 11002
    {RIL_UNSOL_STK_CALL_CONTROL_RESULT, responseVoid, WAKE_PARTIAL}, // 11003
    {RIL_UNSOL_DUN_CALL_STATUS, responseVoid, WAKE_PARTIAL}, // 11004
    {11005, NULL, WAKE_PARTIAL},
    {11006, NULL, WAKE_PARTIAL},
    {RIL_UNSOL_O2_HOME_ZONE_INFO, responseVoid, WAKE_PARTIAL}, // 11007
    {RIL_UNSOL_DEVICE_READY_NOTI, responseVoid, WAKE_PARTIAL}, // 11008
    {RIL_UNSOL_GPS_NOTI, responseVoid, WAKE_PARTIAL}, // 11009
    {RIL_UNSOL_AM, responseString, WAKE_PARTIAL}, // 11010
    {RIL_UNSOL_DUN_PIN_CONTROL_SIGNAL, responseVoid, WAKE_PARTIAL}, // 11011
    {RIL_UNSOL_DATA_SUSPEND_RESUME, responseInts, WAKE_PARTIAL}, // 11012
    {RIL_UNSOL_SAP, responseVoid, WAKE_PARTIAL}, // 11013
    {11014, NULL, WAKE_PARTIAL},
    {11015, NULL, WAKE_PARTIAL},
    {RIL_UNSOL_HSDPA_STATE_CHANGED, responseVoid, WAKE_PARTIAL}, // 11016
    {RIL_UNSOL_WB_AMR_STATE, responseInts, WAKE_PARTIAL}, // 11017
    {RIL_UNSOL_TWO_MIC_STATE, responseInts, WAKE_PARTIAL}, // 11018
    {RIL_UNSOL_DHA_STATE, responseVoid, WAKE_PARTIAL}, // 11019
    {RIL_UNSOL_UART, responseVoid, WAKE_PARTIAL}, // 11020
    {RIL_UNSOL_SIM_PB_READY, responseVoid, WAKE_PARTIAL}, // 11021
    {RIL_UNSOL_PCMCLOCK_STATE, responseInts, WAKE_PARTIAL}, // 11022
    {RIL_UNSOL_1X_SMSPP, responseRaw, WAKE_PARTIAL},
    {11024, responseRaw, WAKE_PARTIAL},/*???*/
    {11025, NULL, WAKE_PARTIAL},
    {11026, NULL, WAKE_PARTIAL},
    {RIL_UNSOL_IMS_REGISTRATION_STATE_CHANGED, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_MODIFY_CALL, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_SRVCC_HANDOVER, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_CS_FALLBACK, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED, responseInts, WAKE_PARTIAL},
    {UNSOL_VOICE_SYSTEM_ID, responseInts, WAKE_PARTIAL},
    {11033, responseVoid, WAKE_PARTIAL},/*???*/
    {RIL_UNSOL_IMS_RETRYOVER, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_STK_ALPHA_ID, responseString, WAKE_PARTIAL},
    {RIL_UNSOL_PB_INIT_COMPLETE, responseVoid, WAKE_PARTIAL},
    {11037, NULL, WAKE_PARTIAL},
    {11038, NULL, WAKE_PARTIAL},
    {11039, NULL, WAKE_PARTIAL},
    {11040, NULL, WAKE_PARTIAL},
    {RIL_UNSOL_RTS_INDICATION, responseStrings, WAKE_PARTIAL},
    {RIL_UNSOL_RILD_RESET_NOTI, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_HOME_NETWORK_NOTI, responseVoid, WAKE_PARTIAL},
    {11044, NULL, WAKE_PARTIAL},
    {RIL_UNSOL_OMADM_SEND_DATA, responseVoid, WAKE_PARTIAL},
    {RIL_UNSOL_DUN, responseString, WAKE_PARTIAL},
    {RIL_UNSOL_SYSTEM_REBOOT, responseVoid, WAKE_PARTIAL},
    {11048, NULL, WAKE_PARTIAL},
    {RIL_UNSOL_UTS_GETSMSCOUNT, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_UTS_GETSMSMSG, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_UTS_GET_UNREAD_SMS_STATUS, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_MIP_CONNECT_STATUS, responseInts, WAKE_PARTIAL},
    {RIL_UNSOL_STK_CALL_STATUS, responseInts, WAKE_PARTIAL},
    /*{RIL_UNSOL_CP_ASSERTED_OR_RESETTING, responseVoid, WAKE_PARTIAL},*/
