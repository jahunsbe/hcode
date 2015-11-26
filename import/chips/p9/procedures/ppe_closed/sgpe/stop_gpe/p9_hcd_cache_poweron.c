/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p9/procedures/ppe_closed/sgpe/stop_gpe/p9_hcd_cache_poweron.c $ */
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

#include "p9_sgpe_stop.h"
#include "p9_sgpe_stop_exit_marks.h"

int
p9_hcd_cache_poweron(uint8_t quad)
{
    int rc = SGPE_STOP_SUCCESS;
    uint64_t scom_data;

    // vdd_pfet_force_state == 00 (Nop)
    PK_TRACE("Make sure we are not forcing PFET for VDD off");
    GPE_GETSCOM(PPM_PFCS, QUAD_ADDR_BASE, quad, scom_data);

    if (scom_data & BITS64(0, 2))
    {
        return SGPE_STOP_ENTRY_VDD_PFET_NOT_IDLE;
    }

    // vdd_pfet_val/sel_override     = 0 (disbaled)
    // vdd_pfet_regulation_finger_en = 0 (controled by FSM)
    PK_TRACE("Prepare PFET Controls");
    GPE_PUTSCOM(PPM_PFCS_CLR, QUAD_ADDR_BASE, quad, BIT64(4) | BIT64(5) | BIT64(8));

    // vdd_pfet_force_state = 11 (Force Von)
    PK_TRACE("Power Off Core VDD");
    GPE_PUTSCOM(PPM_PFCS_OR, QUAD_ADDR_BASE, quad, BITS64(0, 2));

    PK_TRACE("Poll for power gate sequencer state: 0x8 (FSM Idle)");

    do
    {
        GPE_GETSCOM(PPM_PFCS, QUAD_ADDR_BASE, quad, scom_data);
    }
    while(!(scom_data & BIT64(42)));

    MARK_TRAP(SX_POWERON_DONE)

#if !EPM_P9_TUNNING
    PK_TRACE("Optional: Poll for vdd_pg_sel being: 0x0");

    do
    {
        GPE_GETSCOM(PPM_PFCS, QUAD_ADDR_BASE, quad, scom_data);
    }
    while(scom_data & BIT64(46));

    MARK_TRAP(SX_POWERON_PG_SEL)
#endif

    // vdd_pfet_force_state = 00 (Nop)
    PK_TRACE("Turn Off Force Von");
    GPE_PUTSCOM(PPM_PFCS_CLR, QUAD_ADDR_BASE, quad, BITS64(0, 2));

    return rc;
}