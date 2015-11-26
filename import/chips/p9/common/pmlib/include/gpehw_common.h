/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p9/common/pmlib/include/gpehw_common.h $         */
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
#ifndef __GPEHW_COMMON_H__
#define __GPEHW_COMMON_H__

enum GPE_PIG_TYPES
{
    GPE_PIG_PSTATE_PHASE1       = 0,
    GPE_PIG_PSTATE_PHASE2       = 1,
    GPE_PIG_STOP_STATE_CHANGE   = 2,
    GPE_PIG_CME_REQUEST         = 3,
    GPE_PIG_CME_ACK             = 4,
    GPE_PIG_CME_ERROR           = 5,
    GPE_PIG_QUAD_PPM_MSG        = 6,
    GPE_PIG_QUAD_PPM_ERROR      = 7
};


enum GPE_CHIPLET_CONFIGS
{
    CORES_PER_EX                 = 2,
    EXES_PER_QUAD                = 2,
    CORES_PER_QUAD               = 4,
    MAX_QUADS                    = 6,
    MAX_EXES                     = 12,
    MAX_CORES                    = 24
};

enum GPE_CHIPLET_MASKS
{
    ONE_QUAD_IN_CHIP             = 0x20,
    ALL_QUADS_IN_CHIP            = 0x3F,
    ONE_CORE_IN_QUAD             = 0x8,
    ALL_CORES_IN_QUAD            = 0xF,
    FST_CORE_IN_EX               = 0x2,
    SND_CORE_IN_EX               = 0x1,
    BOTH_CORES_IN_EX             = 0x3,
    FST_EX_IN_QUAD               = 0x2,
    SND_EX_IN_QUAD               = 0x1,
    BOTH_EX_IN_QUAD              = 0x3,
    FST_2CORES_IN_QUAD           = 0xC,
    SND_2CORES_IN_QUAD           = 0x3
};


enum GPE_SCOM_ADDRESS_PARAMETERS
{
    QUAD_ADDR_OFFSET            = 0x01000000,
    QUAD_ADDR_BASE              = 0x10000000,
    CORE_ADDR_OFFSET            = 0x01000000,
    CORE_ADDR_BASE              = 0x20000000,
    CME_ADDR_OFFSET             = 0x00000400,
    CME_ADDR_OFFSET_EX0         = 0x00000400,
    CME_ADDR_OFFSET_EX1         = 0x00000800,
    CME_ADDR_BASE               = 0x10012000
};


/// GPE SCOM
#define GPE_SCOM_ADDR(addr, base, offset) (addr | base | (offset << 24))
#define GPE_SCOM_ADDR_CORE(addr, core) GPE_SCOM_ADDR(addr, CORE_ADDR_BASE, core)
#define GPE_SCOM_ADDR_QUAD(addr, quad) GPE_SCOM_ADDR(addr, QUAD_ADDR_BASE, quad)

#define GPE_GETSCOM(addr, base, offset, data)                          \
    rc = getscom(0, GPE_SCOM_ADDR(addr, base, offset), &data);         \
    if (rc) {                                                          \
        PK_TRACE("getscom@%d failed w/rc=0x%08x",                      \
                 GPE_SCOM_ADDR(addr, base, offset), rc);               \
        pk_halt();                                                     \
    }

#define GPE_PUTSCOM(addr, base, offset, data)                                \
    rc = putscom(0, GPE_SCOM_ADDR(addr, base, offset), data);          \
    if (rc) {                                                          \
        PK_TRACE("putscom@%d failed w/rc=0x%08x",                      \
                 GPE_SCOM_ADDR(addr, base, offset), rc);               \
        pk_halt();                                                     \
    }


#endif  /* __GPEHW_COMMON_H__ */