/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p9/procedures/ppe_closed/sgpe/stop_gpe/p9_hcd_cache_scominit.c $ */
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

extern SgpeStopRecord G_sgpe_stop_record;

int
p9_hcd_cache_scominit(uint32_t quad, uint32_t ex)
{
    int rc = SGPE_STOP_SUCCESS;
    uint64_t   scom_data;
    ocb_qcsr_t qcsr;

    PK_TRACE("Set L3_LCO_TARGET_ID/VICTIMS via EX_L3_MODE_REG1[2-5,6-21]");

    // read partial good exes
    do
    {
        qcsr.value = in32(OCB_QCSR);
    }
    while (qcsr.fields.change_in_progress);

    if (ex & FST_EX_IN_QUAD)
    {
        GPE_GETSCOM(GPE_SCOM_ADDR_EX(EX_L3_MODE_REG1, quad, 0), scom_data);
        scom_data |= (quad << SHIFT32((5 - 1)));
        scom_data |= ((qcsr.value & BITS32(0, 12)) >> 6);
        GPE_PUTSCOM(GPE_SCOM_ADDR_EX(EX_L3_MODE_REG1, quad, 0), scom_data);
    }

    if (ex & SND_EX_IN_QUAD)
    {
        GPE_GETSCOM(GPE_SCOM_ADDR_EX(EX_L3_MODE_REG1, quad, 1), scom_data);
        scom_data |= ((quad << SHIFT32((5 - 1))) + 1);
        scom_data |= ((qcsr.value & BITS32(0, 12)) >> 6);
        GPE_PUTSCOM(GPE_SCOM_ADDR_EX(EX_L3_MODE_REG1, quad, 1), scom_data);
    }

    PK_TRACE("Enable DTS sampling via THERM_MODE_REG[5]");
    GPE_GETSCOM(GPE_SCOM_ADDR_QUAD(EQ_THERM_MODE_REG, quad), scom_data);
    scom_data |= BIT64(5);
    /// @todo set the sample pulse count (bit 6:9)
    /// enable the appropriate loops (needs investigation with the Perv team on the EC wiring).
    GPE_PUTSCOM(GPE_SCOM_ADDR_QUAD(EQ_THERM_MODE_REG, quad), scom_data);

    return rc;
}
