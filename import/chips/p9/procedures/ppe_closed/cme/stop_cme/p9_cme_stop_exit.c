/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p9/procedures/ppe_closed/cme/stop_cme/p9_cme_stop_exit.c $ */
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
#include "p9_cme_copy_scan_ring.h"

extern CmeStopRecord G_cme_stop_record;
#if HW386841_PLS_SRR1_DLS_STOP1_FIX
CoreThreadsInfo G_dsl[MAX_CORES_PER_CME] = {0, 0};
#endif

int
p9_cme_stop_exit()
{
    int          d2u4_flag         = 0;
    int          catchup_ongoing_a = 0;
    int          catchup_ongoing_b = 0;
    uint8_t      target_level;
    uint8_t      deeper_level      = 0;
    uint32_t     deeper_core       = 0;
    uint32_t     wakeup;
    uint32_t     core;
#if !SPWU_AUTO
    uint32_t     spwu_stop;
    uint32_t     spwu_wake;
#endif
#if !SKIP_EXIT_CATCHUP
    uint8_t      catchup_level;
    uint32_t     core_catchup;
#endif
#if HW386841_PLS_SRR1_DLS_STOP1_FIX
    uint32_t     core_stop1        = 0;
    uint8_t      dsl;
    uint8_t      thread;
    uint8_t      G_srr1[MAX_CORES_PER_CME][MAX_THREADS_PER_CORE] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
    cme_scom_pscrs00_t pscrs;
#endif
    ppm_sshsrc_t hist;
    uint64_t     data64;

    //--------------------------------------------------------------------------
    // BEGIN OF STOP EXIT
    //--------------------------------------------------------------------------

    // extract wakeup signals, clear status, and mask wakeup interrupts
    wakeup = (in32(CME_LCL_EISR) >> SHIFT32(17)) & 0x3F;
    core   = ((wakeup >> 4) | (wakeup >> 2) | wakeup) & CME_MASK_BC;

    if (core & CME_MASK_C0)
    {
        CME_GETSCOM(CPPM_CPMMR, CME_MASK_C0, CME_SCOM_AND, data64);

        if (data64 & BIT64(13))
        {
            core = core - CME_MASK_C0;
        }
    }

    if (core & CME_MASK_C1)
    {
        CME_GETSCOM(CPPM_CPMMR, CME_MASK_C1, CME_SCOM_AND, data64);

        if (data64 & BIT64(13))
        {
            core = core - CME_MASK_C1;
        }
    }

    // override with partial good core mask
    // also ignore wakeup to running cores
    core = core & G_cme_stop_record.core_enabled &
           (~G_cme_stop_record.core_running);

    PK_TRACE("X0: Core Select[%d] Wakeup[%x] Actual Stop Levels[%d %d]",
             core, wakeup,
             G_cme_stop_record.act_level_c0,
             G_cme_stop_record.act_level_c1);

#if !SPWU_AUTO
    spwu_stop = (wakeup >> 2) & G_cme_stop_record.core_enabled;
    spwu_wake = spwu_stop &   G_cme_stop_record.core_running;
    spwu_stop = spwu_stop & (~G_cme_stop_record.core_running);

    if (spwu_wake)
    {
        PK_TRACE("X0: setting spwu done");
        out32(CME_LCL_SICR_OR,  spwu_wake << SHIFT32(17));
    }

#endif

    // Code Error: function should never be entered without wakeup source active
    if (!core)
    {
#if !SPWU_AUTO

        if (!spwu_wake)
        {
#endif
            PK_TRACE("Error: no wakeup fired");
            pk_halt();
#if !SPWU_AUTO
        }
        else
        {
            return CME_STOP_SUCCESS;
        }

#endif
    }

    //==================================
    MARK_TAG(BEGINSCOPE_STOP_EXIT, core)
    //==================================

#if HW386841_PLS_SRR1_DLS_STOP1_FIX

    if ((core & CME_MASK_C0) && G_cme_stop_record.act_level_c0 == STOP_LEVEL_1)
    {
        core_stop1 |= CME_MASK_C0;
    }

    if ((core & CME_MASK_C1) && G_cme_stop_record.act_level_c1 == STOP_LEVEL_1)
    {
        core_stop1 |= CME_MASK_C1;
    }

    core = core & ~core_stop1;

if (!core) goto STOP1_EXIT:
#endif

        // set target_level to STOP level for c0
        // unless c1(also or only) wants to wakeup
        target_level = deeper_level =
                           (core == CME_MASK_C0) ? G_cme_stop_record.act_level_c0 :
                           G_cme_stop_record.act_level_c1;

    // If both cores want to wakeup but are in different STOP levels,
    // set deeper_level to the deeper level targeted by deeper core
    if ((core == CME_MASK_BC) &&
        (G_cme_stop_record.act_level_c0 != G_cme_stop_record.act_level_c1))
    {
        // Assume C0 is deeper, target_level is already set to C1
        deeper_level = G_cme_stop_record.act_level_c0;
        deeper_core  = CME_MASK_C0;

        // Otherwise correct assumption on which one is in lighter level
        if (G_cme_stop_record.act_level_c0 < G_cme_stop_record.act_level_c1)
        {
            target_level = G_cme_stop_record.act_level_c0;
            deeper_level = G_cme_stop_record.act_level_c1;
            deeper_core  = CME_MASK_C1;
        }
    }

    PK_TRACE("X0: target_lv[%d], deeper_lv[%d], deeper_c[%d]",
             target_level, deeper_level, deeper_core);

    // ensure PCB Mux grant is present for all cores that wants to wakeup
    // only stop 11 needs to request for new grant
    out32(CME_LCL_SICR_OR,  (core << SHIFT32(11)));

    // chtm purge done
    out32(CME_LCL_EISR_CLR, (core << SHIFT32(25)));

    PK_TRACE("X0: Update STOP History: In Transition of Exit");
    CME_STOP_UPDATE_HISTORY(core,
                            STOP_CORE_IS_GATED,
                            STOP_TRANS_EXIT,
                            STOP_LEVEL_0,
                            STOP_LEVEL_0,
                            STOP_REQ_DISABLE,
                            STOP_ACT_DISABLE);

    PK_TRACE("X0: Check and Request PCB Mux");

    while((core & (in32(CME_LCL_SISR) >> SHIFT32(11))) != core);

    //--------------------------------------------------------------------------
    // STOP LEVEL 2 - single core express
    //--------------------------------------------------------------------------

    if (deeper_core)
    {
        core = CME_MASK_BC - deeper_core;

        //============================
        MARK_TAG(SX_STARTCLOCKS, core)
        //============================

        // do this again here for stop2 in addition to chiplet_reset
        // Note IPL doesnt need to do this twice
        PK_TRACE("4S2: Set Core Glitchless Mux to DPLL");
        CME_PUTSCOM(C_PPM_CGCR, core, BIT64(3));

        PK_TRACE("X9: Start Core Clock");
        p9_hcd_core_startclocks(core);

        // Clear CPPM PECE Shadow
        CME_PUTSCOM(CPPM_PECES, core, 0);

        core = deeper_core;
        deeper_core = 0;
    }

    //--------------------------------------------------------------------------
    // STOP LEVEL 4
    //--------------------------------------------------------------------------

    if (deeper_level >= STOP_LEVEL_4)
    {
        PK_TRACE("STOP Level 4+ Sequence");

        // if deeper_core is set, then core must be 0b11
        if (deeper_core && target_level < STOP_LEVEL_4)
        {
            d2u4_flag = 1;
            core      = deeper_core;
        }

        do   //catchup loop
        {

#if !SKIP_BCE_SCOM_RESTORE
            PK_TRACE("BCE Runtime Kickoff: copy scom restore");
            //right now a blocking call. Need to confirm this.
            instance_scom_restore();
#endif

            //========================
            MARK_TAG(SX_POWERON, core)
            //========================

            PK_TRACE("X1: Core Poweron");
            p9_hcd_core_poweron(core);

            //=========================
            MARK_TRAP(SX_CHIPLET_RESET)
            //=========================

            PK_TRACE("X2: Core Chiplet Reset");
            p9_hcd_core_chiplet_reset(core);

#if !STOP_PRIME
#if !SKIP_EXIT_CATCHUP

            if (catchup_ongoing_a && (!catchup_ongoing_b))
            {
                core = CME_MASK_BC;
                catchup_ongoing_a = 0;
            }
            else if ((core != CME_MASK_BC) && (!deeper_core) && (!catchup_ongoing_b))
            {
                wakeup = (in32(CME_LCL_EISR) >> SHIFT32(17)) & 0x3F;
                core_catchup = (~core) &
                               ((wakeup >> 4) | (wakeup >> 2) | wakeup);
                core_catchup = core_catchup & G_cme_stop_record.core_enabled &
                               (~G_cme_stop_record.core_running);

                if (core_catchup & CME_MASK_C0)
                {
                    CME_GETSCOM(CPPM_CPMMR, CME_MASK_C0, CME_SCOM_AND, data64);

                    if (data64 & BIT64(13))
                    {
                        core_catchup = core_catchup - CME_MASK_C0;
                    }
                }

                if (core_catchup & CME_MASK_C1)
                {
                    CME_GETSCOM(CPPM_CPMMR, CME_MASK_C1, CME_SCOM_AND, data64);

                    if (data64 & BIT64(13))
                    {
                        core_catchup = core_catchup - CME_MASK_C1;
                    }
                }

                if (core_catchup)
                {
                    //==================================
                    MARK_TAG(SX_CATCHUP_B, core_catchup)
                    //==================================

                    // pcbmux grant
                    out32(CME_LCL_SICR_OR,  (core_catchup << SHIFT32(11)));
                    // chtm purge done
                    out32(CME_LCL_EISR_CLR, (core_catchup << SHIFT32(25)));

                    CME_STOP_UPDATE_HISTORY(core_catchup,
                                            STOP_CORE_IS_GATED,
                                            STOP_TRANS_EXIT,
                                            STOP_LEVEL_0,
                                            STOP_LEVEL_0,
                                            STOP_REQ_DISABLE,
                                            STOP_ACT_DISABLE);

                    catchup_level = (core_catchup & CME_MASK_C0) ?
                                    G_cme_stop_record.act_level_c0 :
                                    G_cme_stop_record.act_level_c1 ;

                    PK_TRACE("core[%d] running[%d] core_catchup[%d] catchup_level[%d]",
                             core, G_cme_stop_record.core_running,
                             core_catchup, catchup_level);

                    while((core_catchup & (in32(CME_LCL_SISR) >>
                                           SHIFT32(11))) != core_catchup);

                    if (catchup_level < STOP_LEVEL_4)
                    {
                        deeper_core  = core;
                        d2u4_flag    = 1;
                    }
                    else
                    {
                        core = core_catchup;
                        catchup_ongoing_a = 1;
                        continue;
                    }
                }
            }

#endif

            //==============================
            MARK_TAG(SX_CHIPLET_INITS, core)
            //==============================
#if !SKIP_INITF
            PK_TRACE("X3: Core GPTR Time Initf");
            p9_hcd_core_gptr_time_initf(core);
#endif

            PK_TRACE("X5: Core Chiplet Init");
            p9_hcd_core_chiplet_init(core);

#if !SKIP_INITF
            PK_TRACE("X6: Core Repair Initf");
            p9_hcd_core_repair_initf(core);
#endif

#if !SKIP_EXIT_CATCHUP

            if (catchup_ongoing_b)
            {
                core = CME_MASK_BC;
                catchup_ongoing_b = 0;
            }
            else if ((core != CME_MASK_BC) && (!deeper_core))
            {
                wakeup = (in32(CME_LCL_EISR) >> SHIFT32(17)) & 0x3F;
                core_catchup = (~core) &
                               ((wakeup >> 4) | (wakeup >> 2) | wakeup);
                core_catchup = core_catchup & G_cme_stop_record.core_enabled &
                               (~G_cme_stop_record.core_running);

                if (core_catchup & CME_MASK_C0)
                {
                    CME_GETSCOM(CPPM_CPMMR, CME_MASK_C0, CME_SCOM_AND, data64);

                    if (data64 & BIT64(13))
                    {
                        core_catchup = core_catchup - CME_MASK_C0;
                    }
                }

                if (core_catchup & CME_MASK_C1)
                {
                    CME_GETSCOM(CPPM_CPMMR, CME_MASK_C1, CME_SCOM_AND, data64);

                    if (data64 & BIT64(13))
                    {
                        core_catchup = core_catchup - CME_MASK_C1;
                    }
                }

                if (core_catchup)
                {
                    //==================================
                    MARK_TAG(SX_CATCHUP_B, core_catchup)
                    //==================================

                    // pcbmux grant
                    out32(CME_LCL_SICR_OR,  (core_catchup << SHIFT32(11)));
                    // chtm purge done
                    out32(CME_LCL_EISR_CLR, (core_catchup << SHIFT32(25)));

                    CME_STOP_UPDATE_HISTORY(core_catchup,
                                            STOP_CORE_IS_GATED,
                                            STOP_TRANS_EXIT,
                                            STOP_LEVEL_0,
                                            STOP_LEVEL_0,
                                            STOP_REQ_DISABLE,
                                            STOP_ACT_DISABLE);

                    catchup_level = (core_catchup & CME_MASK_C0) ?
                                    G_cme_stop_record.act_level_c0 :
                                    G_cme_stop_record.act_level_c1 ;

                    PK_TRACE("core[%d] running[%d] core_catchup[%d] catchup_level[%d]",
                             core, G_cme_stop_record.core_running,
                             core_catchup, catchup_level);

                    while((core_catchup & (in32(CME_LCL_SISR) >>
                                           SHIFT32(11))) != core_catchup);

                    if (catchup_level < STOP_LEVEL_4)
                    {
                        deeper_core  = core;
                        d2u4_flag    = 1;
                    }
                    else
                    {
                        core = core_catchup;
                        catchup_ongoing_b = 1;
                        continue;
                    }
                }
            }

#endif

            //===========================
            MARK_TAG(SX_ARRAY_INIT, core)
            //===========================
#if !SKIP_ARRAYINIT
            PK_TRACE("X7: Core Array Init");
            p9_hcd_core_arrayinit(core);
#endif

            //=====================
            MARK_TRAP(SX_FUNC_INIT)
            //=====================
#if !SKIP_INITF
            PK_TRACE("X8: Core Func Scan");
            p9_hcd_core_initf(core);
#endif
#endif

        }
        while(catchup_ongoing_a || catchup_ongoing_b);

        if (d2u4_flag)
        {
            core = CME_MASK_BC;
        }
    }

    //--------------------------------------------------------------------------
    // STOP LEVEL 3
    //--------------------------------------------------------------------------

    if (deeper_level == STOP_LEVEL_3 || target_level == STOP_LEVEL_3)
    {
        //======================
        MARK_TAG(SX_STOP3, core)
        //======================

        PK_TRACE("STOP Level 3 Sequence");
        //Return to full voltage
        //disable ivrm?
    }

    //--------------------------------------------------------------------------
    // STOP LEVEL 2
    //--------------------------------------------------------------------------

    //============================
    MARK_TAG(SX_STARTCLOCKS, core)
    //============================

    // do this again here for stop2 in addition to chiplet_reset
    // Note IPL doesnt need to do this twice
    PK_TRACE("4S2: Set Core Glitchless Mux to DPLL");
    CME_PUTSCOM(C_PPM_CGCR, core, BIT64(3));

    PK_TRACE("X9: Start Core Clock");
    p9_hcd_core_startclocks(core);

    // Clear CPPM PECE Shadow
    CME_PUTSCOM(CPPM_PECES, core, 0);

    //--------------------------------------------------------------------------
    // STOP LEVEL 4
    //--------------------------------------------------------------------------

    if (deeper_level >= STOP_LEVEL_4)
    {
        PK_TRACE("STOP Level 4+ Sequence");

        if (d2u4_flag)
        {
            core = deeper_core;
        }

#if !STOP_PRIME
        //===========================
        MARK_TAG(SX_SCOM_INITS, core)
        //===========================

        PK_TRACE("X10: Image Hardcoded Scoms");
        p9_hcd_core_scominit(core);

        //==========================
        MARK_TAG(SX_BCE_CHECK, core)
        //==========================

#if !SKIP_BCE_SCOM_RESTORE
        PK_TRACE("BCE Runtime Check");

        if( BLOCK_COPY_SUCCESS != isScanRingCopyDone() )
        {
            PK_TRACE("BCE: Copy of scom restore failed");
            pk_halt();
        }

        PK_TRACE("X11: XIP Customized Scoms");
        p9_hcd_core_scomcust(core);
#endif

        //==============================
        MARK_TAG(SX_RUNTIME_INITS, core)
        //==============================

        // todo
        //PK_TRACE("X12: RAS Runtime Scom");
        //p9_hcd_core_ras_runtime_scom(core);

        // todo
        //PK_TRACE("X13: OCC Runtime Scom");
        //p9_hcd_core_occ_runtime_scom(core);
#endif

        //=============================
        MARK_TAG(SX_SELF_RESTORE, core)
        //=============================

        PK_TRACE("X14: Core Self Restore");

#if !SKIP_SELF_RESTORE
        PK_TRACE("Raise block interrupt to PC");
        out32(CME_LCL_SICR_OR, core << SHIFT32(3));

        PK_TRACE("Now Wakeup the Core(pm_exit=1)");
        out32(CME_LCL_SICR_OR, core << SHIFT32(5));

        PK_TRACE("Polling for Core Wakeup(pm_active=0)");

        while((in32(CME_LCL_EINR)) & (core << SHIFT32(21)));

#if !SKIP_RAM_HRMOR
        PK_TRACE("Activate Thread0 for RAM");
        CME_PUTSCOM(THREAD_INFO, core, BIT64(18));

        PK_TRACE("Enable RAM mode");
        CME_PUTSCOM(RAM_MODEREG, core, BIT64(0));

        PK_TRACE("Set SPR mode to LT0-7");
        CME_PUTSCOM(SPR_MODE, core, BITS64(20, 8));

        if (core & CME_MASK_C0)
        {
            PK_TRACE("Set SPRC to Scratch0 for core0");
            CME_PUTSCOM(SCOM_SPRC, CME_MASK_C0, 0);

            PK_TRACE("Load Scratch0 with HOMER+2MB");
#if EPM_P9_TUNING
            CME_PUTSCOM(SCRACTH0, CME_MASK_C0, 0x200000);
#else
            CME_PUTSCOM(SCRACTH0, CME_MASK_C0, 0xA200000);
            //CME_PUTSCOM(SCRACTH0, CME_MASK_C0, in64(SELF_RESTORE_ADDR_FETCH));
#endif
        }

        if (core & CME_MASK_C1)
        {
            PK_TRACE("Set SPRC to Scratch1 for core1");
            CME_PUTSCOM(SCOM_SPRC, CME_MASK_C1, BIT64(60));

            PK_TRACE("Load Scratch1 with HOMER+2MB");
#if EPM_P9_TUNING
            CME_PUTSCOM(SCRACTH1, CME_MASK_C1, 0x200000);
#else
            CME_PUTSCOM(SCRACTH1, CME_MASK_C1, 0xA200000);
            //CME_PUTSCOM(SCRACTH1, CME_MASK_C1, in64(SELF_RESTORE_ADDR_FETCH));
#endif
        }

        PK_TRACE("RAM: mfspr sprd , gpr0");
        CME_PUTSCOM(RAM_CTRL, core, RAM_MFSPR_SPRD_GPR0);

        PK_TRACE("RAM: mtspr hrmor, gpr0");
        CME_PUTSCOM(RAM_CTRL, core, RAM_MTSPR_HRMOR_GPR0);

        PK_TRACE("Disable Thread0 for RAM");
        CME_PUTSCOM(THREAD_INFO, core, 0);

        PK_TRACE("Disable RAM mode");
        CME_PUTSCOM(RAM_MODEREG, core, 0);
#endif

        PK_TRACE("S-Reset all threads");
        CME_PUTSCOM(DIRECT_CONTROLS, core,
                    BIT64(4) | BIT64(12) | BIT64(20) | BIT64(28));

        //==========================
        MARK_TRAP(SX_SRESET_THREADS)
        //==========================

        PK_TRACE("Allow threads to run(pm_exit=0) core: %d data:0x%08x ", core, (core << SHIFT32(5)));
        out32(CME_LCL_SICR_CLR, core << SHIFT32(5));

        PK_TRACE("Poll for Core stop again(pm_active=1)");

        while((~(in32(CME_LCL_EINR))) & (core << SHIFT32(21)))
        {
            if (in32_sh(CME_LCL_SISR) & (core << SHIFT32(1)))
            {
                PK_TRACE("Error: Core Special Attention Detected");
                pk_halt();
            }
        }

        PK_TRACE("Drop block interrupt to PC");
        out32(CME_LCL_SICR_CLR, core << SHIFT32(3));
#endif

        if (d2u4_flag)
        {
            core = CME_MASK_BC;
        }
    }

    //=========================
    MARK_TRAP(SX_ENABLE_ANALOG)
    //=========================

    //--------------------------------------------------------------------------
    // END OF STOP EXIT
    //--------------------------------------------------------------------------

#if HW386841_PLS_SRR1_CME_DLS_FIX
STOP1_EXIT:
    core = core | core_stop1;
#endif

    PK_TRACE("Restore PSSCR.PLS+SRR1 back to actual level");

#if HW386841_PLS_SRR1_CME_DLS_FIX
    core = core | core_stop1;

    if (core & CME_MASK_C0)
    {
        CME_GETSCOM(PPM_SSHSRC, CME_MASK_C0, CME_SCOM_AND, hist.value);

        for (thread = 0; thread < MAX_THREADS_PER_CORE; thread++)
        {
            pscrs.value = in32((CME_LCL_PSCRS00 + (thread << 2)));
            dsl = (G_dsl[0].vector & BITS16((thread << 2), 4)) >>
                  SHIFT16((thread << 2) + 3);
            PK_TRACE("PSCRS1%d %x, old dsl %d", thread, pscrs.value, dsl);

            dsl = MIN(pscrs.field.esl_a_n ? pscrs.fields.rl_a_n : 0,
                      MAX(hist.fields.act_stop_level, dsl));
            G_dsl[0].vector = ((G_dsl[0].vector & ~BITS16((thread << 2), 4)) |
                               (dsl << SHIFT16((thread << 2) + 3)));
            PK_TRACE("new dsl %d", dsl);

            if (dsl >= STOP_LEVEL_8)
            {
                G_srr1[0][thread] = MOST_STATE_LOSS;
            }
            else if (dsl >= STOP_LEVEL_4)
            {
                G_srr1[0][thread] = SOME_STATE_LOSS_BUT_NOT_TIMEBASE;
            }
            else
                G_srr1[0][thread] =
                    pscrs.field.esl_a_n ? SOME_STATE_LOSS_BUT_NOT_TIMEBASE : NO_STATE_LOSS;
        }

        CME_PUTSCOM(DIRECT_CONTROLS, CME_MASK_C0, CME_STOP_UPDATE_DLS(G_dsl[0], G_srr1[0]));
        PK_TRACE("DLS   C0: %x %x %x %x",
                 G_dsl[0].threads.t0, G_dsl[0].threads.t1,
                 G_dsl[0].threads.t2, G_dsl[0].threads.t3);
        PK_TRACE("SRR1  C0: %x %x %x %x",
                 G_srr1[0][0], G_srr1[0][1], G_srr1[0][2], G_srr1[0][3]);
    }

    if (core & CME_MASK_C1)
    {
        CME_GETSCOM(PPM_SSHSRC, CME_MASK_C1, CME_SCOM_AND, hist.value);

        for (thread = 0; thread < MAX_THREADS_PER_CORE; thread++)
        {
            pscrs.value = in32((CME_LCL_PSCRS10 + (thread << 2)));
            dsl = (G_dsl[1].vector & BITS16((thread << 2), 4)) >>
                  SHIFT16((thread << 2) + 3);
            PK_TRACE("PSCRS1%d %x, old dsl %d", thread, pscrs.value, dsl);

            dsl = MIN(pscrs.field.esl_a_n ? pscrs.fields.rl_a_n : 0,
                      MAX(hist.fields.act_stop_level, dsl));
            G_dsl[1].vector = ((G_dsl[1].vector & ~BITS16((thread << 2), 4)) |
                               (dsl << SHIFT16((thread << 2) + 3)));
            PK_TRACE("new dsl %d", dsl);

            if (G_dsl[1][thread] >= STOP_LEVEL_8)
            {
                G_srr1[1][thread] = MOST_STATE_LOSS;
            }
            else if (G_dsl[1][thread] >= STOP_LEVEL_4)
            {
                G_srr1[1][thread] = SOME_STATE_LOSS_BUT_NOT_TIMEBASE;
            }
            else
                G_srr1[1][thread] =
                    pscrs.field.esl_a_n ? SOME_STATE_LOSS_BUT_NOT_TIMEBASE : NO_STATE_LOSS;
        }

        CME_PUTSCOM(DIRECT_CONTROLS, CME_MASK_C1, CME_STOP_UPDATE_DLS(G_dsl[1], G_srr1[1]));
        PK_TRACE("DLS   C1: %x %x %x %x",
                 G_dsl[1].threads.t0, G_dsl[1].threads.t1,
                 G_dsl[1].threads.t2, G_dsl[1].threads.t3);
        PK_TRACE("SRR1  C0: %x %x %x %x",
                 G_srr1[1][0], G_srr1[1][1], G_srr1[1][2], G_srr1[1][3]);
    }

#else

    if (core & CME_MASK_C0)
    {
        CME_GETSCOM(PPM_SSHSRC, CME_MASK_C0, CME_SCOM_AND, hist.value);

        if (hist.fields.act_stop_level >= STOP_LEVEL_8)
        {
            CME_PUTSCOM(DIRECT_CONTROLS, CME_MASK_C0, CME_STOP_UPDATE_PLS_SRR1(
                            hist.fields.act_stop_level,
                            MOST_STATE_LOSS, MOST_STATE_LOSS,
                            MOST_STATE_LOSS, MOST_STATE_LOSS));
        }
        else if (hist.fields.act_stop_level >= STOP_LEVEL_4)
        {
            CME_PUTSCOM(DIRECT_CONTROLS, CME_MASK_C0, CME_STOP_UPDATE_PLS_SRR1(
                            hist.fields.act_stop_level,
                            SOME_STATE_LOSS_BUT_NOT_TIMEBASE,
                            SOME_STATE_LOSS_BUT_NOT_TIMEBASE,
                            SOME_STATE_LOSS_BUT_NOT_TIMEBASE,
                            SOME_STATE_LOSS_BUT_NOT_TIMEBASE));
        }
        else
        {
            CME_PUTSCOM(DIRECT_CONTROLS, CME_MASK_C0, CME_STOP_UPDATE_PLS_SRR1(
                            hist.fields.act_stop_level,
                            ((in32(CME_LCL_PSCRS00) & BIT32(2)) ?
                             SOME_STATE_LOSS_BUT_NOT_TIMEBASE : NO_STATE_LOSS),
                            ((in32(CME_LCL_PSCRS10) & BIT32(2)) ?
                             SOME_STATE_LOSS_BUT_NOT_TIMEBASE : NO_STATE_LOSS),
                            ((in32(CME_LCL_PSCRS20) & BIT32(2)) ?
                             SOME_STATE_LOSS_BUT_NOT_TIMEBASE : NO_STATE_LOSS),
                            ((in32(CME_LCL_PSCRS30) & BIT32(2)) ?
                             SOME_STATE_LOSS_BUT_NOT_TIMEBASE : NO_STATE_LOSS)));
        }
    }

    if (core & CME_MASK_C1)
    {
        CME_GETSCOM(PPM_SSHSRC, CME_MASK_C1, CME_SCOM_AND, hist.value);

        if (hist.fields.act_stop_level >= STOP_LEVEL_8)
        {
            CME_PUTSCOM(DIRECT_CONTROLS, CME_MASK_C1, CME_STOP_UPDATE_PLS_SRR1(
                            hist.fields.act_stop_level,
                            MOST_STATE_LOSS, MOST_STATE_LOSS,
                            MOST_STATE_LOSS, MOST_STATE_LOSS));
        }
        else if(hist.fields.act_stop_level >= STOP_LEVEL_4)
        {
            CME_PUTSCOM(DIRECT_CONTROLS, CME_MASK_C1, CME_STOP_UPDATE_PLS_SRR1(
                            hist.fields.act_stop_level,
                            SOME_STATE_LOSS_BUT_NOT_TIMEBASE,
                            SOME_STATE_LOSS_BUT_NOT_TIMEBASE,
                            SOME_STATE_LOSS_BUT_NOT_TIMEBASE,
                            SOME_STATE_LOSS_BUT_NOT_TIMEBASE));
        }
        else
        {
            CME_PUTSCOM(DIRECT_CONTROLS, CME_MASK_C1, CME_STOP_UPDATE_PLS_SRR1(
                            hist.fields.act_stop_level,
                            ((in32(CME_LCL_PSCRS01) & BIT32(2)) ?
                             SOME_STATE_LOSS_BUT_NOT_TIMEBASE : NO_STATE_LOSS),
                            ((in32(CME_LCL_PSCRS11) & BIT32(2)) ?
                             SOME_STATE_LOSS_BUT_NOT_TIMEBASE : NO_STATE_LOSS),
                            ((in32(CME_LCL_PSCRS21) & BIT32(2)) ?
                             SOME_STATE_LOSS_BUT_NOT_TIMEBASE : NO_STATE_LOSS),
                            ((in32(CME_LCL_PSCRS31) & BIT32(2)) ?
                             SOME_STATE_LOSS_BUT_NOT_TIMEBASE : NO_STATE_LOSS)));
        }
    }

#endif

    PK_TRACE("XF: Now Wakeup the Core(pm_exit=1)");
    out32(CME_LCL_SICR_OR, core << SHIFT32(5));

    PK_TRACE("XF: Polling for Core Waking up(pm_active=0)");

    while((in32(CME_LCL_EINR)) & (core << SHIFT32(21)));

    PK_TRACE("XF: Release PCB Mux back to Core");
    out32(CME_LCL_SICR_CLR, core << SHIFT32(11));

    while((core & ~(in32(CME_LCL_SISR) >> SHIFT32(11))) != core);

    PK_TRACE("XF: Update STOP History: STOP Exit Completed");
    CME_STOP_UPDATE_HISTORY(core,
                            STOP_CORE_READY_RUN,
                            STOP_TRANS_COMPLETE,
                            STOP_LEVEL_0,
                            STOP_LEVEL_0,
                            STOP_REQ_DISABLE,
                            STOP_ACT_DISABLE)

    if (core & CME_MASK_C0)
    {
        G_cme_stop_record.req_level_c0 = 0;
        G_cme_stop_record.act_level_c0 = 0;
    }

    if (core & CME_MASK_C1)
    {
        G_cme_stop_record.req_level_c1 = 0;
        G_cme_stop_record.act_level_c1 = 0;
    }

    G_cme_stop_record.core_running |=  core;
    G_cme_stop_record.core_stopgpe &= ~core;

    // HALTED_STOP_OVERRIDE_DISABLE = 0
    out32(CME_LCL_LMCR_CLR, (core << SHIFT32(15)));

    PK_TRACE("XF: Drop pm_exit to allow core to run if spwu is not present");
#if SPWU_AUTO

    // clear auto spwu disable, enable auto spwu
    out32(CME_LCL_LMCR_CLR, core << SHIFT32(13));

    //clear pm_exit
    out32(CME_LCL_SICR_CLR, core << SHIFT32(5));

#else

    if (spwu_stop)
    {
        // done = spwu + !pm_active + !core_chiplet_fence + !pcbmux_req + !pcbmux_grant
        // chiplet fence forces pm_active to zero
        PK_TRACE("XF: setting spwu done");
        out32(CME_LCL_SICR_OR,  spwu_stop << SHIFT32(17));
    }

    if ((core = (core & (~spwu_stop))))
    {
        PK_TRACE("XF: drop pm_exit for not spwu core");
        out32(CME_LCL_SICR_CLR, core << SHIFT32(5));
    }

#endif

    //===========================
    MARK_TRAP(ENDSCOPE_STOP_EXIT)
    //===========================

    return CME_STOP_SUCCESS;
}
