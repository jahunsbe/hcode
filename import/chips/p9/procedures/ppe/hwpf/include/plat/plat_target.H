/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p9/procedures/ppe/hwpf/include/plat/plat_target.H $ */
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
/**
 * @file plat_target.H
 * @brief platform definitions for fapi2 targets
 */

#ifndef __FAPI2_PLAT_TARGET__
#define __FAPI2_PLAT_TARGET__

#include <stdint.h>

//
// Define what a platform handle looks like. For Hostboot,
// for example, this might be a void*. For the SBE, this
// will be a uint32_t ...
//
namespace fapi2
{
typedef union plat_target_handle
{
    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t chiplet_num       : 8;
        uint32_t type_target_num   : 8;
        uint32_t present           : 1;
        uint32_t functional        : 1;
        uint32_t _reserved_b1      : 1;
        uint32_t valid             : 1;
        uint32_t type              : 12;
#else
        uint32_t type              : 12;
        uint32_t valid             : 1;
        uint32_t _reserved_b1      : 1;
        uint32_t functional        : 1;
        uint32_t present           : 1;
        uint32_t type_target_num   : 8;
        uint32_t chiplet_num       : 8;
#endif
    } fields;
    // Union Constructor
    plat_target_handle(uint32_t i_value = 0): value(i_value) {}
} plat_target_handle_t;
};

#endif
