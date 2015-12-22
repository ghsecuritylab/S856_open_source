/* Copyright (c) 2014-2016, Lenovo.
 * Created by zhanggb, 20140228
 */

#ifndef LENOVO_MDSS_UTILS_H
#define LENOVO_MDSS_UTILS_H

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/syscalls.h>

#include <linux/irq.h>
#include <asm/system.h>

#include "mdss_dsi.h"

extern void lcd_onoff_control(int enable);
int mdss_lcd_parse_file_cmds(struct dsi_panel_cmds *pcmds);
void mdss_dsi_panel_cabc_onoff(int onoff);
void mdss_dsi_panel_ce_onoff(int onoff);
void mdss_dsi_panel_dimming_onoff(int onoff);
int mdss_dsi_get_cabc_state(void);
int mdss_dsi_get_ce_state(void);
int mdss_dsi_get_dimming_state(void);
#endif
