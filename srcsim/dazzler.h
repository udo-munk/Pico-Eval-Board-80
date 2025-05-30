/*
 * Emulation of Cromemco Dazzler on the Pico-Eval-Board LCD
 *
 * Copyright (C) 2015-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

#ifndef DAZZLER_INC
#define DAZZLER_INC

#include "sim.h"
#include "simdefs.h"

extern void dazzler_ctl_out(BYTE data), dazzler_format_out(BYTE data);
extern void dazzler_draw(void);
extern BYTE dazzler_flags_in(void);
extern volatile bool first_flag;
extern int dazzler_state;

#endif /* !DAZZLER_INC */
