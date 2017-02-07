/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p9/procedures/ppe_closed/cme/stop_cme/p9_hcd_core_scomcust.c $ */
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

#include "p9_cme_stop.h"
#include "p9_cme_stop_exit_marks.h"
#include "p9_hcode_image_defines.H"

int
p9_hcd_core_scomcust(uint32_t core)
{
    /*
        CmeImageHeader_t* pCmeImgHdr  = (CmeImageHeader_t*)(CME_HEADER_IMAGE_OFFSET);
        CmeScomRestore*   pCmeScomRes = (CmeScomRestore*)(pCmeImgHdr->coreScomRestoreOffset);
        int i;

        for(i=0; pCmeScomRes->pad; i++, pCmeScomRes += sizeof(CmeScomRestore))
        {
            PK_TRACE("scom[%d] addr[%x] data[%016llx]",
                     i, pCmeScomRes->addr, pCmeScomRes->data);
            CME_PUTSCOM(pCmeScomRes->addr, core, pCmeScomRes->data);
        }
    */
    return 0;
}
