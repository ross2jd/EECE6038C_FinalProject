/* 
 * File:   config.h
 * Author: cfrosty13
 *
 * Created on January 12, 2014, 6:40 PM
 */

#ifndef CONFIG_H
#define	CONFIG_H

#include "xc.h"

_CONFIG1(
        JTAGEN_OFF      // disable JTAG interface
        & GCP_OFF       // disable general code protection
        & GWRP_OFF      // disable flash write protection
        & ICS_PGx2      // ICSP interface (2=default)
        & FWDTEN_OFF    // disable watchdog timer
        )

_CONFIG2(
        IESO_OFF        // two speed start up disabled
        & FCKSM_CSDCMD  // disable clk-switching/monitor
        & FNOSC_PRIPLL  // primary osicllator: enable PLL
        & POSCMOD_XT    // primary osicllator: XT mode
        )

#endif	/* CONFIG_H */

