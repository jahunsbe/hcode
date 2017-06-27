/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p9/common/pmlib/include/stop_sgpe_cme_api.h $    */
/*                                                                        */
/* OpenPOWER HCODE Project                                                */
/*                                                                        */
/* COPYRIGHT 2015,2017                                                    */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
#ifndef __STOP_SGPE_CME_API_H__
#define __STOP_SGPE_CME_API_H__


enum SGPE_STOP_IRQ_PAYLOAD_MASKS
{

    TYPE2_PAYLOAD_SUSPEND_OP_MASK          = 0x400,
    TYPE2_PAYLOAD_SUSPEND_EXIT_MASK        = 0x200,
    TYPE2_PAYLOAD_SUSPEND_ENTRY_MASK       = 0x100,
    TYPE2_PAYLOAD_SUSPEND_BOTH_MASK        = 0x300,
    TYPE2_PAYLOAD_SUSPEND_ACK_MASK         = 0x080,

    TYPE2_PAYLOAD_HARDWARE_WAKEUP          = 0x800,
    TYPE2_PAYLOAD_EXIT_EVENT               = 0xC00,
    TYPE2_PAYLOAD_STOP_LEVEL               = 0x00F,
    TYPE3_PAYLOAD_EXIT_EVENT               = 0xC00,
    TYPE6_PAYLOAD_EXIT_EVENT               = 0x00F
};


/// PIG TYPEs used by CME
enum CME_STOP_PIG_TYPES
{
    PIG_TYPE2                              = 2,
    PIG_TYPE3                              = 3
};

/// Numberical Doorbell Message IDs(used by CME check)
enum CME_DOORBELL_MESSAGE_IDS
{
    MSGID_DB1_UNSUSPEND_STOP_ENTRIES       = 0x01,
    MSGID_DB1_UNSUSPEND_STOP_EXITS         = 0x02,
    MSGID_DB1_UNSUSPEND_STOP_ENTRIES_EXITS = 0x03,
    MSGID_DB1_WAKEUP_GRANTED               = 0x04,
    MSGID_DB1_SUSPEND_STOP_ENTRIES         = 0x05,
    MSGID_DB1_SUSPEND_STOP_EXITS           = 0x06,
    MSGID_DB1_SUSPEND_STOP_ENTRIES_EXITS   = 0x07
};

#endif  /* __STOP_SGPE_CME_API_H__ */