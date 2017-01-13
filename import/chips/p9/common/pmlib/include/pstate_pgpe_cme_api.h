/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p9/common/pmlib/include/pstate_pgpe_cme_api.h $  */
/*                                                                        */
/* OpenPOWER HCODE Project                                                */
/*                                                                        */
/* COPYRIGHT 2016,2017                                                    */
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
#ifndef __PSTATE_PGPE_CME_API_H__
#define __PSTATE_PGPE_CME_API_H__

#define DPLL_REFCLK_DIVIDER         16667
#define PIG_PAYLOAD_PS_PHASE1_MASK  0x03FF000000000000
#define PIG_PAYLOAD_PS_PHASE2_MASK  0x000003FF00000000
#define PIG_PAYLOAD_MASK            0x0fff000000000000
#define PIG_INTR_FIELD_MASK         0x7000000000000000
#define PIG_INTR_GRANTED_MASK       0x0000000080000000

// Type1 specific defines
#define PIG_PAYLOAD_PSTATE_MASK     0x03ff000000000000
#define PIG_PAYLOAD_PSTATE_SEQ_INCR 0x0400000000000000  // Seq increment value

// PIR defines
#define PIR_INSTANCE_EVEN_ODD_MASK  (uint32_t)(0x00000001)
#define PIR_INSTANCE_NUM_MASK       (uint32_t)(0x0000000F)

//Bit0 WR: 0
//Bit1 Multicast: 1
//Bit 2:4 Multicast Type: 101
//Bit 5:7 Mutlicast Grp: 001
#define PCB_MUTLICAST_GRP1          0x69000000

#define QUAD_FROM_CORE(c) \
    ((c&0x1C) >> 2)

#define QUAD_FROM_CME_INSTANCE_NUM(num) \
    ((num&0xE) >> 1)

//
// CME<->PGPE API
//
enum MESSAGE_ID_DB0
{
    MSGID_DB0_INVALID                   = 0,
    MSGID_DB0_RESERVED                  = 1,
    MSGID_DB0_GLOBAL_ACTUAL_BROADCAST   = 2,
    MSGID_DB0_START_PSTATE_BROADCAST    = 3,
    MSGID_DB0_STOP_PSTATE_BROADCAST     = 4
};

enum MESSAGEID_PCB_TYPE4_ACK_TYPES
{
    MSGID_PCB_TYPE4_ACK_ERROR                    = 0,
    MSGID_PCB_TYPE4_ACK_PSTATE_PROTO_ACK         = 1,
    MSGID_PCB_TYPE4_ACK_PSTATE_SUSPENDED         = 2,
};


//
//PGPE-CME Doorbell0(Global Actual Broadcast)
//
typedef union pgpe_db0_glb_bcast_t
{
    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t  msg_id : 8;
        uint64_t  global_actual : 8;
        uint64_t  quad0_ps: 8;
        uint64_t  quad1_ps: 8;
        uint64_t  quad2_ps: 8;
        uint64_t  quad3_ps: 8;
        uint64_t  quad4_ps: 8;
        uint64_t  quad5_ps: 8;
#else
        uint64_t  quad5_ps: 8;
        uint64_t  quad4_ps: 8;
        uint64_t  quad3_ps: 8;
        uint64_t  quad2_ps: 8;
        uint64_t  quad1_ps: 8;
        uint64_t  quad0_ps: 8;
        uint64_t  global_actual : 8;
        uint64_t  msg_id : 8;
#endif
    } fields;
} pgpe_db0_glb_bcast_t;

//
//PGPE-CME Doorbell0(Start Pstate Broaadcast)
//
typedef union pgpe_db0_start_ps_bcast
{
    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t msg_id : 8;
        uint64_t global_actual : 8;
        uint64_t quad0_ps : 8;
        uint64_t quad1_ps : 8;
        uint64_t quad2_ps : 8;
        uint64_t quad3_ps : 8;
        uint64_t quad4_ps : 8;
        uint64_t quad5_ps : 8;
#else
        uint64_t quad5_ps : 8;
        uint64_t quad4_ps : 8;
        uint64_t quad3_ps : 8;
        uint64_t quad2_ps : 8;
        uint64_t quad1_ps : 8;
        uint64_t quad0_ps : 8;
        uint64_t global_actual : 8;
        uint64_t msg_id : 8;
#endif
    } fields;
} pgpe_db0_start_ps_bcast_t;

//
//PGPE-CME Doorbell0(Pstate Stop Broaadcast)
//
typedef union pgpe_db0_stop_ps_bcast
{
    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t msg_id : 8;
        uint64_t reserved : 56;
#else
        uint64_t reserved : 56;
        uint64_t msg_id : 8;
#endif
    } fields;
} pgpe_db0_stop_ps_bcast_t;

#endif //__PSTATE_PGPE_CME_API_H__