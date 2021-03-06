/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p9/procedures/ppe/iota/iota_trace.h $            */
/*                                                                        */
/* OpenPOWER HCODE Project                                                */
/*                                                                        */
/* COPYRIGHT 2017,2018                                                    */
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
#if !defined(__IOTA_TRACE__)
#define __IOTA_TRACE__

#include "pk_app_cfg.h"
#include "pk_trace.h"

// Enable PK application tracing for latency measurments
#ifndef PK_TRACE_PERF_ENABLE
    #define PK_TRACE_PERF_ENABLE 0
#endif

/// Enable PK application tracing (enabled by default)
#ifndef PK_TRACE_ENABLE
    #define PK_TRACE_ENABLE 1
#endif

/// Enable PK ctrl  (enabled by default)
#ifndef PK_TRACE_CTRL_ENABLE
    #define PK_TRACE_CTRL_ENABLE 1
#endif

/// Enable PK crit  (disabled by default)
#ifndef PK_TRACE_CRIT_ENABLE
    #define PK_TRACE_CRIT_ENABLE 0
#endif

/// Enable PK ckpt  (disabled by default)
#ifndef PK_TRACE_CKPT_ENABLE
    #define PK_TRACE_CKPT_ENABLE 0
#endif

/// Enable Debug suppress  (disabled by default)
// a.k.a. enabled means turn off PK_TRACE(), but keep crit trace
#ifndef PK_TRACE_DBG_SUPPRESS
    #define PK_TRACE_DBG_SUPPRESS 0
#endif

/// Enable PK kernel tracing (disabled by default)
#ifndef PK_KERNEL_TRACE_ENABLE
    #define PK_KERNEL_TRACE_ENABLE 0
#endif

/// pk trace disabled implies no tracing at all
//   override any other trace settings
#if !PK_TRACE_ENABLE
    #undef PK_TRACE_DBG_SUPPRESS
    #undef PK_TRACE_CTRL_ENABLE
    #undef PK_TRACE_CRIT_ENABLE
    #undef PK_TRACE_CKPT_ENABLE

    #define PK_TRACE_DBG_SUPPRESS 1
    #define PK_TRACE_CTRL_ENABLE 0
    #define PK_TRACE_CRIT_ENABLE 0
    #define PK_TRACE_CKPT_ENABLE 0
#endif

// PK TRACE enabled implies all default tracing on.
// PK TRACE enabled & PK CKPT DEBUG disabled implies PK CRIT INFO and CTRL ERROR tracing only.
// PK TRACE enabled & PK CKPT DEBUG and CRIT INFO disabled implies PK CTRL ERROR tracing only.
// PK TRACE enabled & PK CRIT INFO disabled && PK DEBUG disabled && PK ERROR disabled implies
//          PK TRACE disabled
#if PK_TRACE_ENABLE &&  PK_TRACE_DBG_SUPPRESS && !PK_TRACE_CRIT_ENABLE && !PK_TRACE_CKPT_ENABLE && !PK_TRACE_CTRL_ENABLE
    #undef PK_TRACE_ENABLE
    #define PK_TRACE_ENABLE 0
#endif


//Application trace macros
#if !PK_TRACE_PERF_ENABLE
    #define PK_TRACE_PERF(...)
#else
    #define PK_TRACE_PERF(...) PKTRACE(__VA_ARGS__)
#endif

#if PK_TRACE_DBG_SUPPRESS
    #define PK_TRACE(...)
    #define PK_TRACE_BIN(str, bufp, buf_size)
#else
    #define PK_TRACE(...) PKTRACE(__VA_ARGS__)
    #define PK_TRACE_BIN(str, bufp, buf_size) PKTRACE_BIN(str, bufp, buf_size)
#endif

#if !PK_TRACE_CTRL_ENABLE
    #define PK_TRACE_ERR(...)
#else
    #define PK_TRACE_ERR(...) PKTRACE(__VA_ARGS__)
#endif

#if !PK_TRACE_CRIT_ENABLE
    #define PK_TRACE_INF(...)
#else
    #define PK_TRACE_INF(...) PKTRACE(__VA_ARGS__)
#endif

#if !PK_TRACE_CKPT_ENABLE
    #define PK_TRACE_DBG(...)
#else
    #define PK_TRACE_DBG(...) PKTRACE(__VA_ARGS__)
#endif

/// The following macros are helper macros for tracing.  They should not be called
/// directly.
#define VARG_COUNT_HELPER(_0, _1, _2, _3, _4, _5, _6, _7, N, ...) N
#define VARG_COUNT(...) VARG_COUNT_HELPER(, ##__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0)

/// Trace macros for C functions
#define HASH_ARG_COMBO(str, arg) \
    ((((uint32_t)trace_ppe_hash(str, PK_TRACE_HASH_PREFIX)) << 16) | ((uint32_t)(arg) & 0x0000ffff))

#define PKTRACE0(...) pk_trace_tiny() //will fail at compile time

#define PKTRACE1(str) \
    pk_trace_tiny((trace_ppe_hash(str, PK_TRACE_HASH_PREFIX) << 16))

#define PKTRACE2(str, parm0) \
    ((sizeof(parm0) <= 2)? \
     pk_trace_tiny(HASH_ARG_COMBO(str, parm0)): \
     pk_trace_big(HASH_ARG_COMBO(str, 1), ((uint64_t)parm0) << 32, 0))

#define PKTRACE3(str, parm0, parm1) \
    pk_trace_big(HASH_ARG_COMBO(str, 2), ((((uint64_t)parm0) << 32) | parm1), 0)

#define PKTRACE4(str, parm0, parm1, parm2) \
    pk_trace_big(HASH_ARG_COMBO(str, 3), ((((uint64_t)parm0) << 32) | parm1),\
                 ((uint64_t)parm2) << 32 )

#define PKTRACE5(str, parm0, parm1, parm2, parm3) \
    pk_trace_big(HASH_ARG_COMBO(str, 4), ((((uint64_t)parm0) << 32) | parm1),\
                 ((((uint64_t)parm2) << 32) | parm3) )

#define PKTRACE6(...) pk_trace_tiny() //will fail at compile time
#define PKTRACE7(...) pk_trace_tiny() //will fail at compile time

#define PKTRACE_HELPER2(count, ...) PKTRACE ## count (__VA_ARGS__)
#define PKTRACE_HELPER(count, ...) PKTRACE_HELPER2(count, __VA_ARGS__)

#if (PK_TRACE_SUPPORT)
#define PKTRACE(...) PKTRACE_HELPER(VARG_COUNT(__VA_ARGS__), __VA_ARGS__)
#define PKTRACE_BIN(str, bufp, buf_size) \
    pk_trace_binary(((buf_size < 255)? HASH_ARG_COMBO(str, buf_size): HASH_ARG_COMBO(str, 255)), bufp)
#else
#define PKTRACE(...)
#define PKTRACE_BIN(str, bufp, buf_size)
#endif //PK_TRACE_SUPPORT

//Trace function prototypes
void pk_trace_tiny(uint32_t i_parm);
void pk_trace_big(uint32_t i_hash_and_count,
                  uint64_t i_parm1, uint64_t i_parm2);
void pk_trace_binary(uint32_t i_hash_and_size, void* bufp);
void pk_trace_set_freq(uint32_t i_freq);





#endif
