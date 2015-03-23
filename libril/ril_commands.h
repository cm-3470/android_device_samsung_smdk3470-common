/* //device/libs/telephony/ril_commands.h
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
    {0, NULL, NULL},                   //none
    {RIL_REQUEST_GET_SIM_STATUS, dispatchVoid, responseSimStatus},
    {RIL_REQUEST_ENTER_SIM_PIN, dispatchStrings, responseInts},
    {RIL_REQUEST_ENTER_SIM_PUK, dispatchStrings, responseInts},
    {RIL_REQUEST_ENTER_SIM_PIN2, dispatchStrings, responseInts},
    {RIL_REQUEST_ENTER_SIM_PUK2, dispatchStrings, responseInts},
    {RIL_REQUEST_CHANGE_SIM_PIN, dispatchStrings, responseInts},
    {RIL_REQUEST_CHANGE_SIM_PIN2, dispatchStrings, responseInts},
    {RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION, dispatchStrings, responseInts},
    {RIL_REQUEST_GET_CURRENT_CALLS, dispatchVoid, responseCallList},
    {RIL_REQUEST_DIAL, dispatchDial, responseVoid},
    {RIL_REQUEST_GET_IMSI, dispatchStrings, responseString},
    {RIL_REQUEST_HANGUP, dispatchInts, responseVoid},
    {RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND, dispatchVoid, responseVoid},
    {RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND, dispatchVoid, responseVoid},
    {RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, dispatchVoid, responseVoid},
    {RIL_REQUEST_CONFERENCE, dispatchVoid, responseVoid},
    {RIL_REQUEST_UDUB, dispatchVoid, responseVoid},
    {RIL_REQUEST_LAST_CALL_FAIL_CAUSE, dispatchVoid, responseInts},
    {RIL_REQUEST_SIGNAL_STRENGTH, dispatchVoid, responseRilSignalStrength},
    {RIL_REQUEST_VOICE_REGISTRATION_STATE, dispatchVoid, responseStrings},
    {RIL_REQUEST_DATA_REGISTRATION_STATE, dispatchVoid, responseStrings},
    {RIL_REQUEST_OPERATOR, dispatchVoid, responseStrings},
    {RIL_REQUEST_RADIO_POWER, dispatchInts, responseVoid},
    {RIL_REQUEST_DTMF, dispatchString, responseVoid},
    {RIL_REQUEST_SEND_SMS, dispatchStrings, responseSMS},
    {RIL_REQUEST_SEND_SMS_EXPECT_MORE, dispatchStrings, responseSMS},
    {RIL_REQUEST_SETUP_DATA_CALL, dispatchDataCall, responseSetupDataCall},
    {RIL_REQUEST_SIM_IO, dispatchSIM_IO, responseSIM_IO},
    {RIL_REQUEST_SEND_USSD, dispatchString, responseVoid},
    {RIL_REQUEST_CANCEL_USSD, dispatchVoid, responseVoid},
    {RIL_REQUEST_GET_CLIR, dispatchVoid, responseInts},
    {RIL_REQUEST_SET_CLIR, dispatchInts, responseVoid},
    {RIL_REQUEST_QUERY_CALL_FORWARD_STATUS, dispatchCallForward, responseCallForwards},
    {RIL_REQUEST_SET_CALL_FORWARD, dispatchCallForward, responseVoid},
    {RIL_REQUEST_QUERY_CALL_WAITING, dispatchInts, responseInts},
    {RIL_REQUEST_SET_CALL_WAITING, dispatchInts, responseVoid},
    {RIL_REQUEST_SMS_ACKNOWLEDGE, dispatchInts, responseVoid},
    {RIL_REQUEST_GET_IMEI, dispatchVoid, responseString},
    {RIL_REQUEST_GET_IMEISV, dispatchVoid, responseString},
    {RIL_REQUEST_ANSWER, dispatchInts, responseVoid},
    {RIL_REQUEST_DEACTIVATE_DATA_CALL, dispatchStrings, responseVoid},
    {RIL_REQUEST_QUERY_FACILITY_LOCK, dispatchStrings, responseInts},
    {RIL_REQUEST_SET_FACILITY_LOCK, dispatchStrings, responseInts},
    {RIL_REQUEST_CHANGE_BARRING_PASSWORD, dispatchStrings, responseVoid},
    {RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE, dispatchVoid, responseInts},
    {RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC, dispatchVoid, responseVoid},
    {RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL, dispatchString, responseVoid},
    {RIL_REQUEST_QUERY_AVAILABLE_NETWORKS , dispatchVoid, responseStringsNetworks},
    {RIL_REQUEST_DTMF_START, dispatchString, responseVoid},
    {RIL_REQUEST_DTMF_STOP, dispatchVoid, responseVoid},
    {RIL_REQUEST_BASEBAND_VERSION, dispatchVoid, responseString},
    {RIL_REQUEST_SEPARATE_CONNECTION, dispatchInts, responseVoid},
    {RIL_REQUEST_SET_MUTE, dispatchInts, responseVoid},
    {RIL_REQUEST_GET_MUTE, dispatchVoid, responseInts},
    {RIL_REQUEST_QUERY_CLIP, dispatchVoid, responseInts},
    {RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE, dispatchVoid, responseInts},
    {RIL_REQUEST_DATA_CALL_LIST, dispatchVoid, responseDataCallList},
    {RIL_REQUEST_RESET_RADIO, dispatchVoid, responseVoid},
    {RIL_REQUEST_OEM_HOOK_RAW, dispatchRaw, responseRaw},
    {RIL_REQUEST_OEM_HOOK_STRINGS, dispatchStrings, responseStrings},
    {RIL_REQUEST_SCREEN_STATE, dispatchInts, responseVoid},
    {RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION, dispatchInts, responseVoid},
    {RIL_REQUEST_WRITE_SMS_TO_SIM, dispatchSmsWrite, responseInts},
    {RIL_REQUEST_DELETE_SMS_ON_SIM, dispatchInts, responseVoid},
    {RIL_REQUEST_SET_BAND_MODE, dispatchInts, responseVoid},
    {RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE, dispatchVoid, responseInts},
    {RIL_REQUEST_STK_GET_PROFILE, dispatchVoid, responseString},
    {RIL_REQUEST_STK_SET_PROFILE, dispatchString, responseVoid},
    {RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND, dispatchString, responseString},
    {RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE, dispatchString, responseVoid},
    {RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM, dispatchInts, responseVoid},
    {RIL_REQUEST_EXPLICIT_CALL_TRANSFER, dispatchVoid, responseVoid},
    {RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, dispatchInts, responseVoid},
    {RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE, dispatchVoid, responseIntsGetPreferredNetworkType},
    {RIL_REQUEST_GET_NEIGHBORING_CELL_IDS, dispatchVoid, responseCellList},
    {RIL_REQUEST_SET_LOCATION_UPDATES, dispatchInts, responseVoid},
    {RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE, dispatchInts, responseVoid},
    {RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE, dispatchInts, responseVoid},
    {RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE, dispatchVoid, responseInts},
    {RIL_REQUEST_SET_TTY_MODE, dispatchInts, responseVoid},
    {RIL_REQUEST_QUERY_TTY_MODE, dispatchVoid, responseInts},
    {RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE, dispatchInts, responseVoid},
    {RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE, dispatchVoid, responseInts},
    {RIL_REQUEST_CDMA_FLASH, dispatchString, responseVoid},
    {RIL_REQUEST_CDMA_BURST_DTMF, dispatchStrings, responseVoid},
    {RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY, dispatchString, responseVoid},
    {RIL_REQUEST_CDMA_SEND_SMS, dispatchCdmaSms, responseSMS},
    {RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, dispatchCdmaSmsAck, responseVoid},
    {RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG, dispatchVoid, responseGsmBrSmsCnf},
    {RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG, dispatchGsmBrSmsCnf, responseVoid},
    {RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION, dispatchInts, responseVoid},
    {RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG, dispatchVoid, responseCdmaBrSmsCnf},
    {RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, dispatchCdmaBrSmsCnf, responseVoid},
    {RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, dispatchInts, responseVoid},
    {RIL_REQUEST_CDMA_SUBSCRIPTION, dispatchVoid, responseStrings},
    {RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM, dispatchRilCdmaSmsWriteArgs, responseInts},
    {RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM, dispatchInts, responseVoid},
    {RIL_REQUEST_DEVICE_IDENTITY, dispatchVoid, responseStrings},
    {RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE, dispatchVoid, responseVoid},
    {RIL_REQUEST_GET_SMSC_ADDRESS, dispatchVoid, responseString},
    {RIL_REQUEST_SET_SMSC_ADDRESS, dispatchString, responseVoid},
    {RIL_REQUEST_REPORT_SMS_MEMORY_STATUS, dispatchInts, responseVoid},
    {RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING, dispatchVoid, responseVoid},
    {RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE, dispatchCdmaSubscriptionSource, responseInts},
    {RIL_REQUEST_ISIM_AUTHENTICATION, dispatchString, responseString},
    {RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU, dispatchStrings, responseVoid},
    {RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS, dispatchString, responseSIM_IO},
    {RIL_REQUEST_VOICE_RADIO_TECH, dispatchVoiceRadioTech, responseInts},
    {RIL_REQUEST_GET_CELL_INFO_LIST, dispatchVoid, responseCellInfoList},
    {RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE, dispatchInts, responseVoid}, // 110
    {RIL_REQUEST_SET_INITIAL_ATTACH_APN, dispatchSetInitialAttachApn, responseVoid},
    {RIL_REQUEST_IMS_REGISTRATION_STATE, dispatchVoid, responseInts},
    {RIL_REQUEST_IMS_SEND_SMS, dispatchImsSms, responseSMS},
    {RIL_REQUEST_GET_DATA_CALL_PROFILE, dispatchInts, responseGetDataCallProfile},
    {RIL_REQUEST_SET_UICC_SUBSCRIPTION, dispatchUiccSubscripton, responseVoid},
    {RIL_REQUEST_SET_DATA_SUBSCRIPTION, dispatchVoid, responseVoid},

    /*???*/
    {RIL_REQUEST_GET_UICC_SUBSCRIPTION, dispatchVoid, responseUiccSubscription},
    {RIL_REQUEST_GET_DATA_SUBSCRIPTION, dispatchVoid, responseInts},
    {RIL_REQUEST_SET_TRANSMIT_POWER, dispatchVoid, responseVoid},

#if 0
    {RIL_REQUEST_SELECT_BAND, dispatchVoid, responseVoid},
    {RIL_REQUEST_GET_BAND, dispatchVoid, responseInts},
    {RIL_REQUEST_SET_FDY, dispatchVoid, responseVoid},
    {RIL_REQUEST_SET_COMCFG, dispatchVoid, responseVoid},
    {RIL_REQUEST_GET_COMCFG, dispatchVoid, responseInts},
    {RIL_REQUEST_SWITCH_MODEM, dispatchVoid, responseVoid},
#endif
    
    {10001, NULL, NULL},
    {RIL_REQUEST_GET_CELL_BROADCAST_CONFIG, dispatchVoid, responseCbSettings},/*???*/
    {10003, NULL, NULL},
    {10004, NULL, NULL},
    {RIL_REQUEST_SEND_ENCODED_USSD, dispatchVoid, responseVoid},
    {RIL_REQUEST_SET_PDA_MEMORY_STATUS, dispatchVoid, responseVoid},
    {RIL_REQUEST_GET_PHONEBOOK_STORAGE_INFO, dispatchVoid, responseInts},/*???*/
    {RIL_REQUEST_GET_PHONEBOOK_ENTRY, dispatchVoid, responseSIM_PB},/*???*/
    {RIL_REQUEST_ACCESS_PHONEBOOK_ENTRY, dispatchVoid, responseInts},/*???*/
    {RIL_REQUEST_DIAL_VIDEO_CALL, dispatchVoid, responseVoid},
    {RIL_REQUEST_CALL_DEFLECTION, dispatchVoid, responseVoid},
    {10012, NULL, NULL},
    {RIL_REQUEST_USIM_PB_CAPA, dispatchVoid, responseInts},/*???*/
    {RIL_REQUEST_LOCK_INFO, dispatchVoid, responseSIM_LockInfo},
    {10015, NULL, NULL},
    {RIL_REQUEST_DIAL_EMERGENCY, dispatchDial, responseVoid},
    {10017, NULL, NULL},
    {RIL_REQUEST_STK_SIM_INIT_EVENT, dispatchVoid, responseVoid},
    {RIL_REQUEST_GET_LINE_ID, dispatchVoid, responseInts},/*???*/
    {RIL_REQUEST_SET_LINE_ID, dispatchVoid, responseVoid},
    {RIL_REQUEST_GET_SERIAL_NUMBER, dispatchVoid, responseString},/*???*/
    {RIL_REQUEST_GET_MANUFACTURE_DATE_NUMBER, dispatchVoid, responseString},/*???*/
    {RIL_REQUEST_GET_BARCODE_NUMBER, dispatchVoid, responseString},/*???*/
    {RIL_REQUEST_UICC_GBA_AUTHENTICATE_BOOTSTRAP, dispatchVoid, responseBootstrap},
    {RIL_REQUEST_UICC_GBA_AUTHENTICATE_NAF, dispatchVoid, responseNaf},
    {10026, NULL, NULL},
    {10027, NULL, NULL},
    {10028, NULL, NULL},
    {10029, NULL, NULL},
    {10030, NULL, NULL},
    
    /*???*/
    {RIL_REQUEST_MODIFY_CALL_INITIATE, dispatchVoid, responseInts},
    {RIL_REQUEST_MODIFY_CALL_CONFIRM, dispatchVoid, responseVoid},
    {RIL_REQUEST_SAFE_MODE, dispatchVoid, responseVoid},
    {RIL_REQUEST_SET_VOICE_DOMAIN_PREF, dispatchVoid, responseVoid},
    
    
    {RIL_REQUEST_PS_ATTACH, dispatchVoid, responseStrings},/*???*/
    {RIL_REQUEST_PS_DETACH, dispatchVoid, responseString},/*???*/
    {RIL_REQUEST_ACTIVATE_DATA_CALL, dispatchVoid, responseStrings},/*???*/
    {RIL_REQUEST_CHANGE_SIM_PERSO, dispatchVoid, responseInts},/*???*/
    {RIL_REQUEST_ENTER_SIM_PERSO, dispatchVoid, responseInts},/*???*/
    {RIL_REQUEST_GET_TIME_INFO, dispatchVoid, responseString},/*???*/
    
    /*???*/
    {RIL_REQUEST_CDMA_SEND_SMS_EXPECT_MORE, dispatchVoid, responseSMS},/*???*/
    
    {RIL_REQUEST_OMADM_SETUP_SESSION, dispatchVoid, responseVoid},
    {RIL_REQUEST_OMADM_SERVER_START_SESSION, dispatchVoid, responseVoid},
    {RIL_REQUEST_OMADM_CLIENT_START_SESSION, dispatchVoid, responseVoid},
    {RIL_REQUEST_OMADM_SEND_DATA, dispatchVoid, responseVoid},
    {RIL_REQUEST_CDMA_GET_DATAPROFILE, dispatchVoid, responseInts},/*???*/
    {RIL_REQUEST_CDMA_SET_DATAPROFILE, dispatchVoid, responseVoid},
    {RIL_REQUEST_CDMA_GET_SYSTEMPROPERTIES, dispatchVoid, responseString},/*???*/
    {RIL_REQUEST_CDMA_SET_SYSTEMPROPERTIES, dispatchVoid, responseVoid},
    {RIL_REQUEST_SEND_SMS_COUNT, dispatchVoid, responseVoid},
    {RIL_REQUEST_SEND_SMS_MSG, dispatchVoid, responseVoid},
    {RIL_REQUEST_SEND_SMS_MSG_READ_STATUS, dispatchVoid, responseVoid},
    {RIL_REQUEST_MODEM_HANGUP, dispatchVoid, responseVoid},
    {RIL_REQUEST_SET_SIM_POWER, dispatchVoid, responseSimPowerDone},/*???*/
    {RIL_REQUEST_SET_PREFERRED_NETWORK_LIST, dispatchVoid, responseVoid},
    {RIL_REQUEST_GET_PREFERRED_NETWORK_LIST, dispatchVoid, responsePreferredNetworkList},
    {RIL_REQUEST_HANGUP_VT, dispatchVoid, responseVoid},
    
    {RIL_REQUEST_HOLD, dispatchVoid, responseVoid},
    {RIL_REQUEST_SET_LTE_BAND_MODE, dispatchVoid, responseVoid},
    {RIL_REQUEST_QUERY_LOCK_NETWORKS, dispatchVoid, responseNetworkInfos},
