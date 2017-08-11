/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p9/procedures/ppe_closed/cme/pstate_cme/p9_cme_pstate.c $ */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file   p9_cme_pstate.c
/// \brief  CME and QCME codes enforcing the Power protocols for Pstate, DPLL
///         actuation, iVRM, resonant clocking, and VDM.
/// \owner  Rahul Batra Email: rbatra@us.ibm.com
///

#include "pk.h"
#include "ppe42_scom.h"

#include "cme_firmware_registers.h"
#include "cme_register_addresses.h"
#include "cppm_firmware_registers.h"
#include "cppm_register_addresses.h"
#include "ppm_firmware_registers.h"
#include "ppm_register_addresses.h"
#include "qppm_firmware_registers.h"
#include "qppm_register_addresses.h"

#include "ppehw_common.h"
#include "cmehw_common.h"
#include "cmehw_interrupts.h"
#include "p9_cme_pstate.h"
#include "p9_hcode_image_defines.H"

//
//Globals
//
cmeHeader_t* G_cmeHeader;
LocalPstateParmBlock* G_lppb;
extern CmePstateRecord G_cme_pstate_record;
extern CmeRecord G_cme_record;

const uint8_t G_vdm_threshold_table[13] =
{
    0x00, 0x01, 0x03, 0x02, 0x06, 0x07, 0x05, 0x04,
    0x0C, 0x0D, 0x0F, 0x0E, 0x0A
};

//
//send_pig_packet
//
int send_pig_packet(uint64_t data, uint32_t coreMask)
{
    int               rc = 0;
    uint64_t          data_tmp;

    // First make sure no interrupt request is currently granted
    do
    {
        // Read PPMPIG status
        CME_GETSCOM(PPM_PIG, coreMask, data_tmp);
    }
    while (((ppm_pig_t)data_tmp).fields.intr_granted);

    // Send PIG packet
    CME_PUTSCOM(PPM_PIG, coreMask, data);

    return rc;
}

void ippm_read(uint32_t addr, uint64_t* data)
{
    // G_cme_pstate_record.cmeMaskGoodCore MUST be set!
    uint64_t val;

    cppm_ippmcmd_t cppm_ippmcmd;
    cppm_ippmcmd.value = 0;
    cppm_ippmcmd.fields.qppm_reg = addr & 0x000000ff;
    cppm_ippmcmd.fields.qppm_rnw = 1;
    CME_PUTSCOM(CPPM_IPPMCMD, G_cme_pstate_record.cmeMaskGoodCore,
                cppm_ippmcmd.value);

    do
    {
        CME_GETSCOM(CPPM_IPPMSTAT, G_cme_pstate_record.cmeMaskGoodCore, val);
    } // Check the QPPM_ONGOING bit

    while(val & BIT64(0));

    // QPPM_STATUS, non-zero indicates an error
    if(val & BITS64(1, 2))
    {
        PK_PANIC(CME_PSTATE_IPPM_ACCESS_FAILED);
    }

    CME_GETSCOM(CPPM_IPPMRDATA, G_cme_pstate_record.cmeMaskGoodCore, val);

    *data = val;
}

void ippm_write(uint32_t addr, uint64_t data)
{
    // G_cme_pstate_record.cmeMaskGoodCore MUST be set!
    uint64_t val;

    CME_PUTSCOM(CPPM_IPPMWDATA, G_cme_pstate_record.cmeMaskGoodCore,
                data);
    cppm_ippmcmd_t cppm_ippmcmd;
    cppm_ippmcmd.value = 0;
    cppm_ippmcmd.fields.qppm_reg = addr & 0x000000ff;
    cppm_ippmcmd.fields.qppm_rnw = 0;
    CME_PUTSCOM(CPPM_IPPMCMD, G_cme_pstate_record.cmeMaskGoodCore,
                cppm_ippmcmd.value);

    do
    {
        CME_GETSCOM(CPPM_IPPMSTAT, G_cme_pstate_record.cmeMaskGoodCore, val);
    } // Check the QPPM_ONGOING bit

    while(val & BIT64(0));

    // QPPM_STATUS, non-zero indicates an error
    if(val & BITS64(1, 2))
    {
        PK_PANIC(CME_PSTATE_IPPM_ACCESS_FAILED);
    }
}

void intercme_msg_send(uint32_t msg, INTERCME_MSG_TYPE type)
{
    out32(CME_LCL_ICSR, (msg << 4) | type);

    PK_TRACE_DBG("imt send | msg=%08x", ((msg << 4) | type));

    // Block on ack from companion CME
    while(!(in32(CME_LCL_EISR) & BIT32(30))) {}

    out32(CME_LCL_EISR_CLR, BIT32(30));
}

void intercme_msg_recv(uint32_t* msg, INTERCME_MSG_TYPE type)
{
    // Poll for inter-cme communication from QM
    while(!(in32(CME_LCL_EISR) & BIT32(29))) {}

    *msg = in32(CME_LCL_ICRR);
    PK_TRACE_DBG("imt recv | msg=%08x", *msg);

    if(*msg & type)
    {
        // Shift out the type field, leaving only the message data
        *msg >>= 4;
    }
    else
    {
        PK_PANIC(CME_PSTATE_UNEXPECTED_INTERCME_MSG);
    }

    // Ack back to companion CME that msg was received
    out32(CME_LCL_ICCR_OR, BIT32(0));
    // Clear the ack
    out32(CME_LCL_ICCR_CLR, BIT32(0));
    out32(CME_LCL_EISR_CLR, BIT32(29));
}

void intercme_direct(INTERCME_DIRECT_INTF intf, INTERCME_DIRECT_TYPE type, uint32_t retry_enable)
{
    uint32_t addr_offset = 0;
    uint32_t orig_intf = 0;
    uint32_t poll_count = 0;

    orig_intf = intf;

    // Send intercme interrupt, this is the same whether notifying or acking
    out32(CME_LCL_ICCR_OR , BIT32(intf));
    out32(CME_LCL_ICCR_CLR, BIT32(intf));

    // Adjust the EI*R base address based on which intercme direct interface
    // is used since the bits are spread across both words in the EI*R registers
    if(intf == INTERCME_DIRECT_IN0)
    {
        // IN0: ICCR[5], EI*R[7]
        intf += 2;
    }
    else
    {
        // IN1: ICCR[6], EI*R[38], ie. second half EI*R[6]
        // IN2: ICCR[7], EI*R[39], ie. second half EI*R[7]
        addr_offset = 4;
    }

    if(type == INTERCME_DIRECT_NOTIFY)
    {
        uint32_t intercme_acked = 0;
#if SIMICS_TUNING == 1
        intercme_acked = 1;
#endif
        poll_count = 0;

        while(!intercme_acked)
        {
            // if the master CME is booted long before the slave, the slave won't see the original interrupt
            if (retry_enable && ((poll_count & 0x1FF) == 0x1FF))
            {
                // Send intercme interrupt, this is the same whether notifying or acking
                out32(CME_LCL_ICCR_OR , BIT32(orig_intf));
                out32(CME_LCL_ICCR_CLR, BIT32(orig_intf));
            }

            if(in32((CME_LCL_EISR + addr_offset)) & BIT32(intf))
            {
                intercme_acked = 1;
            }

            poll_count++;
        }
    }

    out32((CME_LCL_EISR_CLR + addr_offset), BIT32(intf)); // Clear the interrupt
}

#ifdef USE_CME_RESCLK_FEATURE
uint32_t p9_cme_resclk_get_index(uint32_t pstate)
{
    int32_t i = RESCLK_FREQ_REGIONS;

    // Walk the table backwards by decrementing the index and checking for
    // a value less than or equal to the requested pstate
    // If no match is found then the index will be zero meaning resonance gets
    // disabled
    while((pstate > G_lppb->resclk.resclk_freq[--i]) && (i > 0)) {}

    PK_TRACE_DBG("resclk_idx[i=%d]=%d", i, G_lppb->resclk.resclk_index[i]);
    return((uint32_t)G_lppb->resclk.resclk_index[i]);
}
#endif//USE_CME_RESCLK_FEATURE

void p9_cme_analog_control(uint32_t core_mask, ANALOG_CONTROL enable)
{
#ifdef USE_CME_RESCLK_FEATURE

    if(in32(CME_LCL_FLAGS) & BIT32(CME_FLAGS_RCLK_OPERABLE))
    {
        uint32_t pstate;
        uint32_t curr_idx;
        uint64_t val;

        if(enable)
        {
            PK_TRACE_INF("resclk | enabling resclks");

            if(core_mask == CME_MASK_C0)
            {
                // Use Core0 index since only updating that core
                curr_idx = G_cme_pstate_record.resclkData.core0_resclk_idx;
            }
            else
            {
                // Use Core1 index if a) only updating that core, or b) in the
                // case of both Cores since the indices will be the same
                curr_idx = G_cme_pstate_record.resclkData.core1_resclk_idx;
            }

            // 1) step CACCR to running pstate
            pstate = G_cme_pstate_record.quadPstate;
            p9_cme_resclk_update(core_mask, pstate, curr_idx);
            // 2) write CACCR[13:15]=0b111 to switch back to common control
            //    and leave clksync enabled
            CME_PUTSCOM(CPPM_CACCR_OR, core_mask, (BITS64(13, 3)));
            // 3) Clear out the CACCR resclk values
            CME_PUTSCOM(CPPM_CACCR_CLR, core_mask, BITS64(0, 13));

            p9_cme_pstate_pmsr_updt(G_cme_record.core_enabled);
        }
        else
        {
            PK_TRACE_INF("resclk | disabling resclks");

            // 1) copy QACCR[0:12] into CACCR[0:12], with CACCR[13:15]=0b000,
            //    to switch away from common control while leaving clksync
            //    disabled. QACCR will already be set to a value corresponding
            //    to the current quad Pstate
            ippm_read(QPPM_QACCR, &val);
            val &= BITS64(13, 51);
            CME_PUTSCOM(CPPM_CACCR, core_mask, val);
            curr_idx = p9_cme_resclk_get_index(G_cme_pstate_record.quadPstate);
            // 2) step CACCR to a value which disables resonance
            pstate = ANALOG_PSTATE_RESCLK_OFF;
            p9_cme_resclk_update(core_mask, pstate, curr_idx);
        }
    }

#endif//USE_CME_RESCLK_FEATURE

#ifdef USE_CME_VDM_FEATURE

    if(in32(CME_LCL_FLAGS) & BIT32(CME_FLAGS_VDM_OPERABLE))
    {
        if(enable)
        {
            PK_TRACE_INF("vdm | enabling vdms");
            // Clear Disable (Poweron is set earlier in Exit flow)
            CME_PUTSCOM(PPM_VDMCR_CLR, core_mask, BIT64(1));
        }
        else
        {
            PK_TRACE_INF("vdm | disabling vdms");
            // Clear Poweron and set Disable in one operation
            CME_PUTSCOM(PPM_VDMCR, core_mask, BIT64(1));
        }
    }

#endif//USE_CME_VDM_FEATURE
}

#ifdef USE_CME_VDM_FEATURE
uint32_t pstate_to_vpd_region(uint32_t pstate)
{
    if(pstate > G_lppb->operating_points[NOMINAL].pstate)
    {
        return REGION_POWERSAVE_NOMINAL;
    }
    else if(pstate > G_lppb->operating_points[TURBO].pstate)
    {
        return REGION_NOMINAL_TURBO;
    }
    else
    {
        return REGION_TURBO_ULTRA;
    }
}

uint32_t pstate_to_vid_compare(uint32_t pstate, uint32_t region)
{
    return((((uint32_t)G_lppb->PsVIDCompSlopes[region]
             * ((uint32_t)G_lppb->operating_points[region].pstate - pstate)
             + VDM_VID_COMP_ADJUST) >> VID_SLOPE_FP_SHIFT_12)
           + (uint32_t)G_lppb->vid_point_set[region]);
}

void calc_vdm_threshold_indices(uint32_t pstate, uint32_t region,
                                uint32_t indices[])
{
    static VDM_ROUNDING_ADJUST vdm_rounding_adjust[NUM_THRESHOLD_POINTS] =
    {
        VDM_OVERVOLT_ADJUST,
        VDM_SMALL_ADJUST,
        VDM_LARGE_ADJUST,
        VDM_XTREME_ADJUST
    };

    uint32_t i = 0;
    int32_t psdiff = (uint32_t)G_lppb->operating_points[region].pstate - pstate;

    for(i = 0; i < NUM_THRESHOLD_POINTS; ++i)
    {
        // Cast every math term into 32b for more efficient PPE maths
        indices[i] = (uint32_t)((int32_t)G_lppb->threshold_set[region][i]
                                + (((int32_t)G_lppb->PsVDMThreshSlopes[region][i] * psdiff
                                    // Apply the rounding adjust
                                    + (int32_t)vdm_rounding_adjust[i]) >> THRESH_SLOPE_FP_SHIFT));
    }

    // Check the interpolation result; since each threshold has a distinct round
    // adjust, the calculated index can be invalid relative to another threshold
    // index. Overvolt does not need to be checked and Small Droop will always
    // be either 0 or greater than 0 by definition.
    // Ensure that small <= large <= xtreme; where any can be == 0.
    indices[VDM_LARGE_IDX] = ((indices[VDM_LARGE_IDX] < indices[VDM_SMALL_IDX])
                              && (indices[VDM_LARGE_IDX] != 0))
                             ? indices[VDM_SMALL_IDX] : indices[VDM_LARGE_IDX];
    indices[VDM_XTREME_IDX] = ((indices[VDM_XTREME_IDX] < indices[VDM_LARGE_IDX])
                               && (indices[VDM_XTREME_IDX] != 0))
                              ? indices[VDM_LARGE_IDX] : indices[VDM_XTREME_IDX];
    indices[VDM_XTREME_IDX] = ((indices[VDM_XTREME_IDX] < indices[VDM_SMALL_IDX])
                               && (indices[VDM_LARGE_IDX]  == 0)
                               && (indices[VDM_XTREME_IDX] != 0))
                              ? indices[VDM_SMALL_IDX] : indices[VDM_XTREME_IDX];
}

void p9_cme_vdm_update(uint32_t pstate)
{
    // Static forces this array into .sbss instead of calling memset()
    static uint32_t new_idx[NUM_THRESHOLD_POINTS] = { 0 };
    uint32_t i = 0;
    // Set one bit per threshold starting at bit 31 (28,29,30,31)
    uint32_t not_done = BITS32(32 - NUM_THRESHOLD_POINTS, NUM_THRESHOLD_POINTS);
    uint64_t scom_data = 0;
    uint64_t base_scom_data = 0;
    uint32_t region = pstate_to_vpd_region(pstate);

    // Calculate the new index for each threshold
    calc_vdm_threshold_indices(pstate, region, new_idx);

    // Look-up the VID compare value using the Pstate
    base_scom_data |= (uint64_t)(pstate_to_vid_compare(pstate, region)
                                 & BITS32(24, 8)) << 56;

    // Step all thresholds in parallel until each reaches its new target.
    // Doing this in parallel minimizes the number of interppm scom writes.
    do
    {
        // Only keep the VID compare value
        scom_data = base_scom_data;

        // Loop over all 4 thresholds (overvolt, small, large, xtreme)
        for(i = 0; i < NUM_THRESHOLD_POINTS; ++i)
        {
            if(new_idx[i] != G_cme_pstate_record.vdmData.vdm_threshold_idx[i])
            {
                // Decrement or increment the current index, whichever is
                // required to reach the new/target index
                G_cme_pstate_record.vdmData.vdm_threshold_idx[i] +=
                    (G_cme_pstate_record.vdmData.vdm_threshold_idx[i] < new_idx[i])
                    ? 1 : -1;
            }
            else
            {
                // Clear the unique bit for each threshold as each threshold is stepped
                // to its new index
                not_done &= ~(0x1 << i);
            }

            // OR the new threshold greycode into the correct position
            scom_data |= (uint64_t)G_vdm_threshold_table[
                             G_cme_pstate_record.vdmData.vdm_threshold_idx[i]]
                         << (52 - (i * 4));
        }

        ippm_write(QPPM_VDMCFGR, scom_data);
    }
    while(not_done);
}
#endif//USE_CME_VDM_FEATURE

#ifdef USE_CME_RESCLK_FEATURE
void p9_cme_resclk_update(ANALOG_TARGET target, uint32_t pstate, uint32_t curr_idx)
{
    uint64_t base_val;
    uint64_t val;
    uint32_t next_idx = p9_cme_resclk_get_index(pstate);
    int32_t  step;

    PK_TRACE_DBG("resclk | target=%08x", (uint32_t)target);
    PK_TRACE_DBG("resclk | pstate=%d"  , pstate);
    PK_TRACE_DBG("resclk | curr_idx=%d", curr_idx);
    PK_TRACE_DBG("resclk | next_idx=%d", next_idx);

    // Determine the step polarity, step is not used if curr_idx == next_idx
    if(curr_idx < next_idx)
    {
        step = 1;
    }
    else
    {
        step = -1;
    }

    // Read out the resclk register that is currently in control
    if(target == ANALOG_COMMON)
    {
        ippm_read(QPPM_QACCR, &base_val);
    }
    else
    {
        CME_GETSCOM(CPPM_CACCR, target, base_val);
    }

    // Preserve only the resclk control bits
    base_val &= (BITS64(13, 51));

    while(curr_idx != next_idx)
    {
        curr_idx += step;
        val = (((uint64_t)G_lppb->resclk.steparray[curr_idx].value) << 48)
              | base_val;

        if(target == ANALOG_COMMON)
        {
            ippm_write(QPPM_QACCR, val);
        }
        else
        {
            CME_PUTSCOM(CPPM_CACCR, target, val);
        }

        // There is an attribute for step-delay which is currently not used,
        // this is where the delay would go.
    }

    // Update the resclk index variables
    if(target == ANALOG_COMMON)
    {
        G_cme_pstate_record.resclkData.common_resclk_idx = curr_idx;
    }
    else
    {
        if(target & ANALOG_CORE0)
        {
            G_cme_pstate_record.resclkData.core0_resclk_idx = curr_idx;
        }

        if(target & ANALOG_CORE1)
        {
            G_cme_pstate_record.resclkData.core1_resclk_idx = curr_idx;
        }
    }
}
#endif//USE_CME_RESCLK_FEATURE

void p9_cme_pstate_pmsr_updt(uint32_t coreMask)
{
    uint32_t cm;
    uint64_t pmsrData;

    //CORE0 mask is 0x2, and CORE1 mask is 0x1.
    //We AND with 0x1 to get the core number from mask.
    for (cm = 2; cm > 0; cm--)
    {
        if (cm & coreMask)
        {
            pmsrData = ((uint64_t)G_cme_pstate_record.globalPstate) << 56;
            pmsrData |= (((uint64_t)(G_cme_pstate_record.quadPstate)) << 48) ;
            pmsrData |= (((uint64_t)(G_cme_pstate_record.pmin)) << 40) ;
            pmsrData |= (((uint64_t)(G_cme_pstate_record.pmax)) << 32) ;

            if (G_cme_pstate_record.pmcrSeenErr & cm)
            {
                pmsrData |= BIT64(PMSR_INVALID_PMCR_VERSION);
            }

            out64(CME_LCL_PMSRS0 + ((cm & 0x1) << 5), pmsrData);
        }
    }
}
