/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p9/common/pmlib/include/registers/pba_firmware_constants.h $ */
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
#define PBA_BARN_CMD_SCOPE_MASK SIXTYFOUR_BIT_CONSTANT(0xe000000000000000)
#define PBA_BARN_ADDR_MASK SIXTYFOUR_BIT_CONSTANT(0x0003fffffff00000)
#define PBA_BARMSKN_MASK_MASK SIXTYFOUR_BIT_CONSTANT(0x000001fffff00000)
#define PBA_BCUE_CTL_STOP SIXTYFOUR_BIT_CONSTANT(0x8000000000000000)
#define PBA_BCUE_CTL_START SIXTYFOUR_BIT_CONSTANT(0x4000000000000000)
#define PBA_BCDE_CTL_STOP SIXTYFOUR_BIT_CONSTANT(0x8000000000000000)
#define PBA_BCDE_CTL_START SIXTYFOUR_BIT_CONSTANT(0x4000000000000000)
#define PBA_FIR_OCI_APAR_ERR SIXTYFOUR_BIT_CONSTANT(0x8000000000000000)
#define PBA_FIR_PB_RDADRERR_FW SIXTYFOUR_BIT_CONSTANT(0x4000000000000000)
#define PBA_FIR_PB_RDDATATO_FW SIXTYFOUR_BIT_CONSTANT(0x2000000000000000)
#define PBA_FIR_PB_SUE_FW SIXTYFOUR_BIT_CONSTANT(0x1000000000000000)
#define PBA_FIR_PB_UE_FW SIXTYFOUR_BIT_CONSTANT(0x0800000000000000)
#define PBA_FIR_PB_CE_FW SIXTYFOUR_BIT_CONSTANT(0x0400000000000000)
#define PBA_FIR_OCI_SLAVE_INIT SIXTYFOUR_BIT_CONSTANT(0x0200000000000000)
#define PBA_FIR_OCI_WRPAR_ERR SIXTYFOUR_BIT_CONSTANT(0x0100000000000000)
#define PBA_FIR_OCI_REREQTO SIXTYFOUR_BIT_CONSTANT(0x0080000000000000)
#define PBA_FIR_PB_UNEXPCRESP SIXTYFOUR_BIT_CONSTANT(0x0040000000000000)
#define PBA_FIR_PB_UNEXPDATA SIXTYFOUR_BIT_CONSTANT(0x0020000000000000)
#define PBA_FIR_PB_PARITY_ERR SIXTYFOUR_BIT_CONSTANT(0x0010000000000000)
#define PBA_FIR_PB_WRADRERR_FW SIXTYFOUR_BIT_CONSTANT(0x0008000000000000)
#define PBA_FIR_PB_BADCRESP SIXTYFOUR_BIT_CONSTANT(0x0004000000000000)
#define PBA_FIR_PB_ACKDEAD_FW SIXTYFOUR_BIT_CONSTANT(0x0002000000000000)
#define PBA_FIR_PB_CRESPTO SIXTYFOUR_BIT_CONSTANT(0x0001000000000000)
#define PBA_FIR_BCUE_SETUP_ERR SIXTYFOUR_BIT_CONSTANT(0x0000800000000000)
#define PBA_FIR_BCUE_PB_ACK_DEAD SIXTYFOUR_BIT_CONSTANT(0x0000400000000000)
#define PBA_FIR_BCUE_PB_ADRERR SIXTYFOUR_BIT_CONSTANT(0x0000200000000000)
#define PBA_FIR_BCUE_OCI_DATAERR SIXTYFOUR_BIT_CONSTANT(0x0000100000000000)
#define PBA_FIR_BCDE_SETUP_ERR SIXTYFOUR_BIT_CONSTANT(0x0000080000000000)
#define PBA_FIR_BCDE_PB_ACK_DEAD SIXTYFOUR_BIT_CONSTANT(0x0000040000000000)
#define PBA_FIR_BCDE_PB_ADRERR SIXTYFOUR_BIT_CONSTANT(0x0000020000000000)
#define PBA_FIR_BCDE_RDDATATO_ERR SIXTYFOUR_BIT_CONSTANT(0x0000010000000000)
#define PBA_FIR_BCDE_SUE_ERR SIXTYFOUR_BIT_CONSTANT(0x0000008000000000)
#define PBA_FIR_BCDE_UE_ERR SIXTYFOUR_BIT_CONSTANT(0x0000004000000000)
#define PBA_FIR_BCDE_CE SIXTYFOUR_BIT_CONSTANT(0x0000002000000000)
#define PBA_FIR_BCDE_OCI_DATAERR SIXTYFOUR_BIT_CONSTANT(0x0000001000000000)
#define PBA_FIR_INTERNAL_ERR SIXTYFOUR_BIT_CONSTANT(0x0000000800000000)
#define PBA_FIR_ILLEGAL_CACHE_OP SIXTYFOUR_BIT_CONSTANT(0x0000000400000000)
#define PBA_FIR_OCI_BAD_REG_ADDR SIXTYFOUR_BIT_CONSTANT(0x0000000200000000)
#define PBA_FIR_AXPUSH_WRERR SIXTYFOUR_BIT_CONSTANT(0x0000000100000000)
#define PBA_FIR_AXRCV_DLO_ERR SIXTYFOUR_BIT_CONSTANT(0x0000000080000000)
#define PBA_FIR_AXRCV_DLO_TO SIXTYFOUR_BIT_CONSTANT(0x0000000040000000)
#define PBA_FIR_AXRCV_RSVDATA_TO SIXTYFOUR_BIT_CONSTANT(0x0000000020000000)
#define PBA_FIR_AXFLOW_ERR SIXTYFOUR_BIT_CONSTANT(0x0000000010000000)
#define PBA_FIR_AXSND_DHI_RTYTO SIXTYFOUR_BIT_CONSTANT(0x0000000008000000)
#define PBA_FIR_AXSND_DLO_RTYTO SIXTYFOUR_BIT_CONSTANT(0x0000000004000000)
#define PBA_FIR_AXSND_RSVTO SIXTYFOUR_BIT_CONSTANT(0x0000000002000000)
#define PBA_FIR_AXSND_RSVERR SIXTYFOUR_BIT_CONSTANT(0x0000000001000000)
#define PBA_FIR_PB_ACKDEAD_FW_WR SIXTYFOUR_BIT_CONSTANT(0x0000000000800000)
#define PBA_FIR_FIR_PARITY_ERR2 SIXTYFOUR_BIT_CONSTANT(0x0000000000080000)
#define PBA_FIR_FIR_PARITY_ERR SIXTYFOUR_BIT_CONSTANT(0x0000000000040000)
