/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/qpnp/pin.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/qpnp/pwm.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/msm_tsens.h>


#include "mdss_fb.h"
#include "mdss_dsi.h"
#include "mdss_mdp.h"
#ifdef CONFIG_F_SKYDISP_SMARTDIMMING
#include "ef63_display.h"
#include <mach/msm_smsm.h>
#endif
extern struct msm_fb_data_type * mfdmsm_fb_get_mfd(void);

#define DT_CMD_HDR 6

DEFINE_LED_TRIGGER(bl_led_trigger);

void mdss_dsi_panel_pwm_cfg(struct mdss_dsi_ctrl_pdata *ctrl)
{
	ctrl->pwm_bl = pwm_request(ctrl->pwm_lpg_chan, "lcd-bklt");
	if (ctrl->pwm_bl == NULL || IS_ERR(ctrl->pwm_bl)) {
		pr_err("%s: Error: lpg_chan=%d pwm request failed",
				__func__, ctrl->pwm_lpg_chan);
	}
}

static void mdss_dsi_panel_bklt_pwm(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	int ret;
	u32 duty;

	if (ctrl->pwm_bl == NULL) {
		pr_err("%s: no PWM\n", __func__);
		return;
	}

	if (level == 0) {
		if (ctrl->pwm_enabled)
			pwm_disable(ctrl->pwm_bl);
		ctrl->pwm_enabled = 0;
		return;
	}

	duty = level * ctrl->pwm_period;
	duty /= ctrl->bklt_max;

	pr_debug("%s: bklt_ctrl=%d pwm_period=%d pwm_gpio=%d pwm_lpg_chan=%d\n",
			__func__, ctrl->bklt_ctrl, ctrl->pwm_period,
				ctrl->pwm_pmic_gpio, ctrl->pwm_lpg_chan);

	pr_debug("%s: ndx=%d level=%d duty=%d\n", __func__,
					ctrl->ndx, level, duty);

	if (ctrl->pwm_enabled) {
		pwm_disable(ctrl->pwm_bl);
		ctrl->pwm_enabled = 0;
	}

	ret = pwm_config_us(ctrl->pwm_bl, duty, ctrl->pwm_period);
	if (ret) {
		pr_err("%s: pwm_config_us() failed err=%d.\n", __func__, ret);
		return;
	}

	ret = pwm_enable(ctrl->pwm_bl);
	if (ret)
		pr_err("%s: pwm_enable() failed err=%d\n", __func__, ret);
	ctrl->pwm_enabled = 1;
}

static char dcs_cmd[2] = {0x54, 0x00}; /* DTYPE_DCS_READ */
static struct dsi_cmd_desc dcs_read_cmd = {
	{DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(dcs_cmd)},
	dcs_cmd
};
#ifdef CONFIG_F_SKYDISP_SMARTDIMMING
u32 mdss_dsi_panel_cmd_read(struct mdss_dsi_ctrl_pdata *ctrl, char cmd0,
		char cmd1, void (*fxn)(int,char *), char *rbuf, int len)
{
	struct dcs_cmd_req cmdreq;

	dcs_cmd[0] = cmd0;
	dcs_cmd[1] = cmd1;
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &dcs_read_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_RX | CMD_REQ_COMMIT;
	cmdreq.rlen = len;
	cmdreq.rbuf = rbuf;
	cmdreq.cb = fxn; /* call back */
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
	/*
	 * blocked here, until call back called
	 */

	return 0;
}
#else
u32 mdss_dsi_panel_cmd_read(struct mdss_dsi_ctrl_pdata *ctrl, char cmd0,
		char cmd1, void (*fxn)(int), char *rbuf, int len)
{
	struct dcs_cmd_req cmdreq;

	dcs_cmd[0] = cmd0;
	dcs_cmd[1] = cmd1;
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &dcs_read_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_RX | CMD_REQ_COMMIT;
	cmdreq.rlen = len;
	cmdreq.rbuf = rbuf;
	cmdreq.cb = fxn; /* call back */
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
	/*
	 * blocked here, until call back called
	 */

	return 0;
}

#endif
static void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
			struct dsi_panel_cmds *pcmds)
{
	struct dcs_cmd_req cmdreq;

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = pcmds->cmds;
	cmdreq.cmds_cnt = pcmds->cmd_cnt;
	cmdreq.flags = CMD_REQ_COMMIT;

	/*Panel ON/Off commands should be sent in DSI Low Power Mode*/
	if (pcmds->link_state == DSI_LP_MODE)
		cmdreq.flags  |= CMD_REQ_LP_MODE;

	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}
#if defined (CONFIG_F_SKYDISP_EF56_SS) || defined (CONFIG_F_SKYDISP_EF59_SS) || defined (CONFIG_F_SKYDISP_EF60_SS)
static char led_pwm1[3] = {0x51, 0x00, 0x00};	/* DTYPE_DCS_LWRITE */
static struct dsi_cmd_desc backlight_cmd = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(led_pwm1)},
	led_pwm1
};

#ifdef CONFIG_F_SKYDISP_SHARP_DIMMING_CTRL
static char dimming_ctl[2] = {0x53, 0x00};	/* DTYPE_DCS_LWRITE */
static struct dsi_cmd_desc dimming_ctrl_cmd = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(dimming_ctl)},
	dimming_ctl
};

void dimming_onoff_ctrl(struct mdss_dsi_ctrl_pdata *ctrl, int state)
{
	struct dcs_cmd_req cmdreq;

	if(ctrl->dimming_state_check == state)
		return;
	
	if(state == 0)
		dimming_ctl[1] = 0x24;  // dimming_off
	else
		dimming_ctl[1] = 0x2c;  // dimming_on

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &dimming_ctrl_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	usleep_range(1000,1000);
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
	ctrl->dimming_state_check = state;
	
	pr_info("sharp dimming %s \n",state ? "on" : "off" );
}
#endif

void mdss_dsi_panel_bklt_dcs(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	struct dcs_cmd_req cmdreq;

	pr_err("[LCD_ON_SPEED]%s: +level=%d\n", __func__, level);
#if 0//defined (CONFIG_F_SKYDISP_EF56_SS)
//PWM 22khz
	led_pwm1[1] = (unsigned char)(level >>8) & 0x0F; 
       led_pwm1[2] = (unsigned char)level & 0xFF;
#else
//PWM 10khz
       led_pwm1[2] = (unsigned char)level & 0xFF;
#endif

#ifdef CONFIG_F_SKYDISP_SHARP_DIMMING_CTRL
	pr_err("[LCD_ON_SPEED]%s: +dimming_onoff_ctrl\n", __func__);
	if(ctrl->onoff_state)
		dimming_onoff_ctrl(ctrl, 0);
	else
		dimming_onoff_ctrl(ctrl, 1);			
	ctrl->onoff_state =0;
	pr_err("[LCD_ON_SPEED]%s: -dimming_onoff_ctrl\n", __func__);		
#endif

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &backlight_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

       usleep_range(1000,1000);
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
	pr_err("[LCD_ON_SPEED]%s: -level=%d\n", __func__, level);	
}
#elif defined(CONFIG_F_SKYDISP_EF63_SS)
static void mdss_dsi_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
				struct dcs_cmd_req * cmdreq,struct dsi_cmd_desc * cmds)
{


	memset(cmdreq, 0, sizeof(cmdreq));
	cmdreq->cmds = cmds;
	cmdreq->cmds_cnt = 1;
	cmdreq->flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq->rlen = 0;
	cmdreq->cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, cmdreq);
}

void mdss_dsi_panel_bklt_dcs(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{

	int i;
	struct dcs_cmd_req cmdreq;
	struct tsens_device tsens_dev;
#ifdef CONFIG_F_SKYDISP_ELVSS_WORK
	int index;
	int  ret = 0;
	long temp,average_temp = 0;
#endif	
	if(ctrl->offline_charger == true){
		
		backlight_cmd.payload = oled_gamma;
		locking_cmd.payload = locking;
		aor_cmd.payload = aor_dim[20];
		elvss_cmd.payload = elvss_set[20];//20

	}
	else{
		backlight_cmd.payload = oled_gamma;
		locking_cmd.payload = locking;
		aor_cmd.payload = aor_dim[level];
		elvss_cmd.payload = elvss_set[level];
		
		oled_gamma[0] = 0xca;	
		oled_gamma[1] = (0x0100 & ctrl->gamma_set.gamma_table[level][0]) >> 8 ;
		oled_gamma[2] =0x0ff & ctrl->gamma_set.gamma_table[level][0] ;	
		oled_gamma[3] =  (0x0100 & ctrl->gamma_set.gamma_table[level][1]) >> 8 ;	
		oled_gamma[4] = 0x0ff & ctrl->gamma_set.gamma_table[level][1] ;
		oled_gamma[5] = (0x0100 & ctrl->gamma_set.gamma_table[level][2]) >> 8 ;
		oled_gamma[6] = 0x0ff & ctrl->gamma_set.gamma_table[level][2] ;

			
		for(i = 7; i <34;i++ ){
			if(ctrl->gamma_set.gamma_table[level][i-4] < 255){
				oled_gamma[i] = ctrl->gamma_set.gamma_table[level][i-4] ;
			}
			else{
				oled_gamma[i] =0xff; 
			}
		}
#ifdef CONFIG_F_SKYDISP_ELVSS_WORK
		for(index = 1; index <=10; index++){
			tsens_dev.sensor_num = index;
			ret = tsens_get_temp(&tsens_dev, &temp);
			if (ret) {
				pr_err(" Unable to read TSENS sensor %d\n", tsens_dev.sensor_num);
				return;
	}
			//printk("elvss_temp_work %ld\n",temp);
			average_temp += temp;
		}
		average_temp = average_temp /10;
		if(average_temp > 127)
			average_temp = 127;
		if(average_temp  < -127)
			average_temp = -127;
		
		if(average_temp <= 127 && average_temp >= 0){
			temp_set[1] = 0x7f & (unsigned char)average_temp;
		}
		if(average_temp >= -127 && average_temp <= -1){
			temp_set[1] = 0x80 | (~(unsigned char)average_temp);
		}
		temp_cmd.payload = temp_set;
		
#endif
	}
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, false);
	mdss_set_tx_power_mode(0 , &ctrl->panel_data );
	
	mdss_dsi_cmds_send(ctrl,&cmdreq,&backlight_cmd);
	mdss_dsi_cmds_send(ctrl,&cmdreq,&aor_cmd);
	mdss_dsi_cmds_send(ctrl,&cmdreq,&elvss_cmd);
#ifdef CONFIG_F_SKYDISP_ELVSS_WORK
	mdss_dsi_cmds_send(ctrl,&cmdreq,&temp_cmd);
#endif
	mdss_dsi_cmds_send(ctrl,&cmdreq,&locking_cmd);
	
	mdss_set_tx_power_mode(1 ,&ctrl->panel_data);
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF, false);

	pr_err(" %d backlight level = %d AOR = 0x%x ELVSS = 0x%x TEMP = 0x%x\n",level,gamma_level[level],aor_dim[level][4],elvss_set[level][2],temp_set[1]);	
#ifdef F_WA_WATCHDOG_DURING_BOOTUP
	ctrl->octa_blck_set =1;
#endif
	
//TODO
}
#else
static char led_pwm1[2] = {0x51, 0x0};	/* DTYPE_DCS_WRITE1 */
static struct dsi_cmd_desc backlight_cmd = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(led_pwm1)},
	led_pwm1
};

static void mdss_dsi_panel_bklt_dcs(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	struct dcs_cmd_req cmdreq;

	pr_debug("%s: level=%d\n", __func__, level);

	led_pwm1[1] = (unsigned char)level;

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &backlight_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}
#endif
void mdss_dsi_panel_reset(struct mdss_panel_data *pdata, int enable)
{
#if defined (CONFIG_F_SKYDISP_EF56_SS) || defined (CONFIG_F_SKYDISP_EF59_SS) ||defined (CONFIG_F_SKYDISP_EF60_SS) ||(defined (CONFIG_F_SKYDISP_EF63_SS) && (CONFIG_BOARD_VER <= CONFIG_PT20)) 
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (!gpio_is_valid(ctrl_pdata->bl_en_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
	}

	if (!gpio_is_valid(ctrl_pdata->rst_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return;
	}
	if (!gpio_is_valid(ctrl_pdata->lcd_vcip_reg_en_gpio)) {
		pr_debug("%s:%d, lcd vcip line not configured\n",
			   __func__, __LINE__);
		return;
	}
	pr_debug("%s: enable = %d\n", __func__, enable);
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (enable) {
		if (gpio_is_valid(ctrl_pdata->lcd_vcip_reg_en_gpio))
			gpio_set_value((ctrl_pdata->lcd_vcip_reg_en_gpio), 1);
              	usleep_range(1000,1000);
#if defined (CONFIG_F_SKYDISP_EF56_SS)			  
		if (gpio_is_valid(ctrl_pdata->lcd_vcin_reg_en_gpio))
			gpio_set_value((ctrl_pdata->lcd_vcin_reg_en_gpio), 1);
              	usleep_range(2000,2000);	
#endif			  
			gpio_set_value((ctrl_pdata->rst_gpio),0);
			usleep_range(12000,15000);
			gpio_set_value((ctrl_pdata->rst_gpio),1);
			usleep_range(12000,15000);			
		if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
			pr_debug("%s: Panel Not properly turned OFF\n",
						__func__);
			ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
			pr_debug("%s: Reset panel done\n", __func__);
		}
	} else {
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		msleep(5);
#if defined (CONFIG_F_SKYDISP_EF56_SS)		
		if (gpio_is_valid(ctrl_pdata->lcd_vcin_reg_en_gpio))
			gpio_set_value((ctrl_pdata->lcd_vcin_reg_en_gpio), 0);	
		msleep(3);
#endif
		if (gpio_is_valid(ctrl_pdata->lcd_vcip_reg_en_gpio))
			gpio_set_value((ctrl_pdata->lcd_vcip_reg_en_gpio), 0);
		msleep(100);
	}
#elif (defined (CONFIG_F_SKYDISP_EF63_SS) && (CONFIG_BOARD_VER >= CONFIG_WS10))
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (!gpio_is_valid(ctrl_pdata->octa_rst_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return;
	}

	pr_debug("%s: enable = %d\n", __func__, enable);
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (enable) {
#if 0		
		gpio_set_value((ctrl->octa_rst_gpio), 1);
		msleep(10);
		gpio_set_value((ctrl->octa_rst_gpio), 0);
              msleep(10); 
		gpio_set_value((ctrl->octa_rst_gpio), 1);
		msleep(10);
#endif		
		if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
			pr_debug("%s: Panel Not properly turned OFF\n",
						__func__);
			ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
			pr_debug("%s: Reset panel done\n", __func__);
		}
	} else {
		gpio_set_value((ctrl_pdata->octa_rst_gpio), 0);
	}
#else  //QUALCOMM default
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;
	int i;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (!gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
	}

	if (!gpio_is_valid(ctrl_pdata->rst_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return;
	}

	pr_debug("%s: enable = %d\n", __func__, enable);
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (enable) {
		if (gpio_is_valid(ctrl_pdata->disp_en_gpio))
			gpio_set_value((ctrl_pdata->disp_en_gpio), 1);

		for (i = 0; i < pdata->panel_info.rst_seq_len; ++i) {
			gpio_set_value((ctrl_pdata->rst_gpio),
				pdata->panel_info.rst_seq[i]);
			if (pdata->panel_info.rst_seq[++i])
				usleep(pdata->panel_info.rst_seq[i] * 1000);
		}

		if (gpio_is_valid(ctrl_pdata->mode_gpio)) {
			if (pinfo->mode_gpio_state == MODE_GPIO_HIGH)
				gpio_set_value((ctrl_pdata->mode_gpio), 1);
			else if (pinfo->mode_gpio_state == MODE_GPIO_LOW)
				gpio_set_value((ctrl_pdata->mode_gpio), 0);
		}
		if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
			pr_debug("%s: Panel Not properly turned OFF\n",
						__func__);
			ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
			pr_debug("%s: Reset panel done\n", __func__);
		}
	} else {
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		if (gpio_is_valid(ctrl_pdata->disp_en_gpio))
			gpio_set_value((ctrl_pdata->disp_en_gpio), 0);
	}
#endif	
}

static char caset[] = {0x2a, 0x00, 0x00, 0x03, 0x00};	/* DTYPE_DCS_LWRITE */
static char paset[] = {0x2b, 0x00, 0x00, 0x05, 0x00};	/* DTYPE_DCS_LWRITE */

static struct dsi_cmd_desc partial_update_enable_cmd[] = {
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(caset)}, caset},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(paset)}, paset},
};

static int mdss_dsi_panel_partial_update(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	struct dcs_cmd_req cmdreq;
	int rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	mipi  = &pdata->panel_info.mipi;

	pr_debug("%s: ctrl=%p ndx=%d\n", __func__, ctrl, ctrl->ndx);

	caset[1] = (((pdata->panel_info.roi_x) & 0xFF00) >> 8);
	caset[2] = (((pdata->panel_info.roi_x) & 0xFF));
	caset[3] = (((pdata->panel_info.roi_x - 1 + pdata->panel_info.roi_w)
								& 0xFF00) >> 8);
	caset[4] = (((pdata->panel_info.roi_x - 1 + pdata->panel_info.roi_w)
								& 0xFF));
	partial_update_enable_cmd[0].payload = caset;

	paset[1] = (((pdata->panel_info.roi_y) & 0xFF00) >> 8);
	paset[2] = (((pdata->panel_info.roi_y) & 0xFF));
	paset[3] = (((pdata->panel_info.roi_y - 1 + pdata->panel_info.roi_h)
								& 0xFF00) >> 8);
	paset[4] = (((pdata->panel_info.roi_y - 1 + pdata->panel_info.roi_h)
								& 0xFF));
	partial_update_enable_cmd[1].payload = paset;

	pr_debug("%s: enabling partial update\n", __func__);
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = partial_update_enable_cmd;
	cmdreq.cmds_cnt = 2;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);

	return rc;
}
#ifdef CONFIG_F_SKYDISP_CABC_CONTROL
void cabc_control(struct mdss_panel_data *pdata, int state)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,panel_data);
#if defined (CONFIG_F_SKYDISP_EF56_SS) ||defined (CONFIG_F_SKYDISP_EF60_SS)
	if(state == 1){
		*(ctrl_pdata->cabc_cmds.cmds->payload + 1) = 0x01;	
		ctrl_pdata->on_cmds.buf[207] = 0x01;
	}else if(state == 0){
		*(ctrl_pdata->cabc_cmds.cmds->payload + 1) = 0x00;
		ctrl_pdata->on_cmds.buf[207] = 0x00;
	}else { //for err
		*(ctrl_pdata->cabc_cmds.cmds->payload+ 1) = 0x01;
		ctrl_pdata->on_cmds.buf[207] = 0x01;
	}	
#else
	if(state == 1){
		*(ctrl_pdata->cabc_cmds.cmds->payload + 1) = 0x03;	
		ctrl_pdata->on_cmds.buf[178] = 0x03;
	}else if(state == 0){
		*(ctrl_pdata->cabc_cmds.cmds->payload + 1) = 0x00;
		ctrl_pdata->on_cmds.buf[178] = 0x00;
	}else { //for err
		*(ctrl_pdata->cabc_cmds.cmds->payload+ 1) = 0x03;
		ctrl_pdata->on_cmds.buf[178] = 0x03;
	}	
#endif
	mdss_dsi_panel_cmds_send(ctrl_pdata, &ctrl_pdata->cabc_cmds);
}
#endif


#ifdef CONFIG_F_SKYDISP_HBM_FOR_AMOLED
void hbm_control(struct mdss_panel_data *pdata, int state)
{

	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct dcs_cmd_req cmdreq;

	char hbm_data[2] = {0x53, 0xD0};
	char acl_data[2] = {0x55, 0x00};	
	struct dsi_cmd_desc hbm_data_cmd = {{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(hbm_data)},hbm_data};
	struct dsi_cmd_desc acl_data_cmd = {{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(acl_data)},acl_data};

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if(pdata->hbm_flag == state)
		return;
	
	if(state == 0){
		hbm_data[1] = 0x00;
		acl_data[1] = 0x01; 
	}
	else
	{//off
		hbm_data[1] = 0xD0;
		acl_data[1] = 0x02;
	}

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &hbm_data_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, false);
	mdss_set_tx_power_mode(0 , pdata );
	
	mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
	
	cmdreq.cmds = &acl_data_cmd;
	mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
	
	mdss_set_tx_power_mode(1 , pdata );
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF, false);
	pdata->hbm_flag = state;
	pr_info("Oled hbm %s \n",state ? "on" : "off" );

}

#endif
#ifdef CONFIG_F_SKYDISP_SMARTDIMMING
void panel_gamma_sort(struct mdss_dsi_ctrl_pdata *pdata)
{
	int i;
	int index;
	int cnt = 27;
	int temp_data = 0;
	if(pdata == NULL)
		return;

	
	for(index = 0; index < GAMMA_TABLE_SIZE; index++){
		for(i = 0; i < 15;){
			temp_data = pdata->gamma_set.gamma_table[index][i];
			pdata->gamma_set.gamma_table[index][i] = pdata->gamma_set.gamma_table[index][cnt];
			pdata->gamma_set.gamma_table[index][cnt] = temp_data;

			temp_data= pdata->gamma_set.gamma_table[index][i+1];
			pdata->gamma_set.gamma_table[index][i+1] = pdata->gamma_set.gamma_table[index][cnt+1];
			pdata->gamma_set.gamma_table[index][cnt+1] = temp_data;

			temp_data= pdata->gamma_set.gamma_table[index][i+2];
			pdata->gamma_set.gamma_table[index][i+2] = pdata->gamma_set.gamma_table[index][cnt+2];
			pdata->gamma_set.gamma_table[index][cnt+2] = temp_data;
			i += 3; 
			cnt -= 3;
		}
		cnt =27;
	}
#if 0
	for( i = 0; i < 32;i++)
		for( index = 0; index< 30;){
			printk("R : 0x%x  G :0x%x B : 0x%x\n",pdata->gamma_set.gamma_table[i][index],pdata->gamma_set.gamma_table[i][index+1],pdata->gamma_set.gamma_table[i][index+2]);
		index+=3;
		}
#endif
}
void mtp_read(int data,char * read_buf)
{

	struct msm_fb_data_type *mfd = mfdmsm_fb_get_mfd();
	struct mdss_panel_info *panel_info = mfd->panel_info;
	struct mdss_panel_data * pdata =NULL;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	int index = 0;
	int temp_index = V203;
	
	pdata = container_of(panel_info, struct mdss_panel_data,
				panel_info);
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	for(index = 0; index < data; index++){
		//printk("OLED Mtp Value[%d] = %d\n",ctrl_pdata->mtp_cnt,*read_buf);
		ctrl_pdata->panel_read_mtp.mtp_data_RGB[ctrl_pdata->mtp_cnt] = *(read_buf);
		(read_buf)++;
		ctrl_pdata->mtp_cnt++;
	}
	
	if(ctrl_pdata->mtp_cnt == MTP_READ_MAX)
	{
		ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][V255] = ((ctrl_pdata->panel_read_mtp.mtp_data_RGB[0] & 0x01) <<8) + ctrl_pdata->panel_read_mtp.mtp_data_RGB[1];
		ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][V255] = ((ctrl_pdata->panel_read_mtp.mtp_data_RGB[2] & 0x01) << 8) + ctrl_pdata->panel_read_mtp.mtp_data_RGB[3];
		ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][V255] = ((ctrl_pdata->panel_read_mtp.mtp_data_RGB[4] & 0x01) << 8)+ ctrl_pdata->panel_read_mtp.mtp_data_RGB[5];

		if(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][V255] > 255)
			ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][V255] = -(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][V255] -256);
		if(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][V255] > 255)
			ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][V255] = -(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][V255] -256);
		if(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][V255] > 255)
			ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][V255] = -(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][V255] -256);

		for(index = 6; index < MTP_READ_MAX; )
		{
			ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][temp_index] = ctrl_pdata->panel_read_mtp.mtp_data_RGB[index];
			ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][temp_index] = ctrl_pdata->panel_read_mtp.mtp_data_RGB[index + 1];
			ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][temp_index] = ctrl_pdata->panel_read_mtp.mtp_data_RGB[index + 2];
			
			if(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][temp_index] > 127)
				ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][temp_index] = -(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][temp_index] - 128);
			if(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][temp_index] > 127)
				ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][temp_index] = -(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][temp_index] - 128);
			if(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][temp_index] > 127)
				ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][temp_index] = -(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][temp_index] - 128);
			
			index += 3;
			temp_index--;
		}	

		//gamma_add2_mtp
		for(index = 0; index < V255_MAX; index++){
			for(temp_index = 0; temp_index < RGB_MAX; temp_index++){
				ctrl_pdata->panel_read_mtp.gamma_add2_mtp[temp_index][index] = ctrl_pdata->panel_read_mtp.mtp_RGB[temp_index][index] 
																+ ctrl_pdata->panel_read_mtp.panel_gamma_data[temp_index][index];
			}
		}	
		
#if 0//def SMART_DIMMING_DEBUG
		for(index = 0;index < 10;index++)
			printk("mtp_data_R[%d]\n",ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][index]);
		printk("======\n");
		for(index = 0;index < 10;index++)
			printk("mtp_data_G[%d]\n",ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][index]);
		printk("======\n");
		for(index = 0;index < 10;index++)
			printk("mtp_data_B[%d]\n",ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][index]);

		for(index = 0;index < 10;index++)
			printk("panel_gamma_data_R[%d]\n",ctrl_pdata->panel_read_mtp.panel_gamma_data[RGB_R][index]);
		printk("======\n");
		for(index = 0;index < 10;index++)
			printk("panel_gamma_data_G[%d]\n",ctrl_pdata->panel_read_mtp.panel_gamma_data[RGB_G][index]);
		printk("======\n");
		for(index = 0;index < 10;index++)
			printk("panel_gamma_data_B[%d]\n",ctrl_pdata->panel_read_mtp.panel_gamma_data[RGB_B][index]);
		
		for(index = 0;index < 10;index++)
			printk("gamma_add2_mtp[%d]\n",ctrl_pdata->panel_read_mtp.gamma_add2_mtp[RGB_R][index]);
		printk("======\n");
		for(index = 0;index < 10;index++)
			printk("gamma_add2_mtp[%d]\n",ctrl_pdata->panel_read_mtp.gamma_add2_mtp[RGB_G][index]);
		printk("======\n");
		for(index = 0;index < 10;index++)
			printk("gamma_add2_mtp[%d]\n",ctrl_pdata->panel_read_mtp.gamma_add2_mtp[RGB_B][index]);
#endif
	}

}	
void mtp_read_work(struct work_struct *work)
{
	struct msm_fb_data_type *mfd = mfdmsm_fb_get_mfd();
	struct mdss_panel_info *panel_info = mfd->panel_info;
	struct mdss_panel_data * pdata =NULL;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct dcs_cmd_req cmdreq;
	
	char mtp_write_data[2] = {0xb0, 0x00};	
	struct dsi_cmd_desc mtp_data_cmd = {{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(mtp_write_data)},mtp_write_data};
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &mtp_data_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;
	
	pdata = container_of(panel_info, struct mdss_panel_data,
				panel_info);
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	mdss_dsi_op_mode_config(panel_info->mipi.mode, pdata);

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, false);
	mdss_set_tx_power_mode(0 , pdata );

#ifdef F_WA_WATCHDOG_DURING_BOOTUP
	{
		char rbuf[]={0};
	 	mdss_dsi_panel_cmd_read(ctrl_pdata, 0x0a, 0x00, NULL, rbuf, 1);
	
		if(rbuf[0] == 0x9C)
		{
			ctrl_pdata->lcd_connect_check = 1;
			printk("[PANTECH_LCD]LCD Connect : 0x0A = %x\n",rbuf[0]);
		}			
		else
		{
			ctrl_pdata->lcd_connect_check = 0;
			printk("[PANTECH_LCD]Maybe... LCD Disconnect : 0x0A = %x\n",rbuf[0]);
		}
	}
#endif

	mdss_dsi_panel_cmd_read(ctrl_pdata,0xc8,0x00,mtp_read,ctrl_pdata->rx_buf.data,10);
	
	*(cmdreq.cmds->payload + 1) = 0x0A;
	mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
	mdss_dsi_panel_cmd_read(ctrl_pdata,0xc8,0x00,mtp_read,ctrl_pdata->rx_buf.data,10);

	*(cmdreq.cmds->payload + 1) = 0x14;
	mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
	mdss_dsi_panel_cmd_read(ctrl_pdata,0xc8,0x00,mtp_read,ctrl_pdata->rx_buf.data,10);

	*(cmdreq.cmds->payload + 1) = 0x1E;
	mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
	mdss_dsi_panel_cmd_read(ctrl_pdata,0xc8,0x00,mtp_read,ctrl_pdata->rx_buf.data,3);
	
	mdss_set_tx_power_mode(1 , pdata );
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF, false);
	
}
#endif
#if defined (CONFIG_F_SKYDISP_EF56_SS) || defined (CONFIG_F_SKYDISP_EF59_SS) || defined (CONFIG_F_SKYDISP_EF60_SS)
bool first_enable = false;
#endif
static void mdss_dsi_panel_bl_ctrl(struct mdss_panel_data *pdata,
							u32 bl_level)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct msm_fb_data_type * mfd = mfdmsm_fb_get_mfd();
	
	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}
	
	if(!mfd->panel_power_on)
	{
		printk("[%s] panel is off state (%d).....\n",__func__,mfd->panel_power_on);
		return;
	}
	
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
#if defined (CONFIG_F_SKYDISP_EF56_SS) || defined (CONFIG_F_SKYDISP_EF59_SS) || defined (CONFIG_F_SKYDISP_EF60_SS)
	if(bl_level == 0)
	{
		gpio_set_value((ctrl_pdata->bl_en_gpio), 0);	
		first_enable = false;
		printk("%s:bl_en_gpio off\n",__func__);
	}
	else
	{     
	       if(first_enable ==false)
	       {
			gpio_set_value((ctrl_pdata->bl_en_gpio), 1);		
			first_enable = true;
			printk("%s:bl_en_gpio on\n",__func__);
	       }
	}
#endif

	/*
	 * Some backlight controllers specify a minimum duty cycle
	 * for the backlight brightness. If the brightness is less
	 * than it, the controller can malfunction.
	 */

	if ((bl_level < pdata->panel_info.bl_min) && (bl_level != 0))
		bl_level = pdata->panel_info.bl_min;

	switch (ctrl_pdata->bklt_ctrl) {
	case BL_WLED:
		led_trigger_event(bl_led_trigger, bl_level);
		break;
	case BL_PWM:
		mdss_dsi_panel_bklt_pwm(ctrl_pdata, bl_level);
		break;
	case BL_DCS_CMD:
#if defined (CONFIG_F_SKYDISP_EF56_SS) || defined (CONFIG_F_SKYDISP_EF59_SS) || defined (CONFIG_F_SKYDISP_EF60_SS)
		mdss_set_tx_power_mode(0 , pdata );
		usleep_range(2000,2000);
#endif
		mdss_dsi_panel_bklt_dcs(ctrl_pdata, bl_level);
#if defined (CONFIG_F_SKYDISP_EF56_SS) || defined (CONFIG_F_SKYDISP_EF59_SS) || defined (CONFIG_F_SKYDISP_EF60_SS)
		usleep_range(1000,1000);
		mdss_set_tx_power_mode(1 , pdata);
#endif
		break;
	default:
		pr_err("%s: Unknown bl_ctrl configuration\n",
			__func__);
		break;
	}
}

static int mdss_dsi_panel_on(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	mipi  = &pdata->panel_info.mipi;

	pr_debug("%s: ctrl=%p ndx=%d\n", __func__, ctrl, ctrl->ndx);
#if defined (CONFIG_F_SKYDISP_EF63_SS) && (CONFIG_BOARD_VER >= CONFIG_WS10)
	gpio_set_value((ctrl->octa_rst_gpio), 1);
	msleep(10);
	gpio_set_value((ctrl->octa_rst_gpio), 0);
	msleep(10);
	gpio_set_value((ctrl->octa_rst_gpio), 1);
	msleep(10);
#endif

#ifdef CONFIG_F_SKYDISP_CMDS_CONTROL	
	if(ctrl->lcd_cmds_check == false){
#endif

		if (ctrl->on_cmds.cmd_cnt){
#if defined(CONFIG_MACH_MSM8974_EF56S) || defined(CONFIG_F_SKYDISP_EF60_SS)
	       	if(ctrl->lcd_on_skip_during_bootup)
				ctrl->on_cmds.buf[189] = 0x00;
#elif defined(CONFIG_F_SKYDISP_EF59_SS)
	      		 if(ctrl->lcd_on_skip_during_bootup)
				ctrl->on_cmds.buf[160] = 0x00;
#endif
			mdss_dsi_panel_cmds_send(ctrl, &ctrl->on_cmds);
		}
#ifdef CONFIG_F_SKYDISP_CMDS_CONTROL		
	}else if(ctrl->lcd_cmds_check == true){
		if (ctrl->on_cmds_user.cmd_cnt)
			mdss_dsi_panel_cmds_send(ctrl, &ctrl->on_cmds_user);
		pr_info("user LCD on cmds---------------->\n");
	}
#endif

#ifdef CONFIG_F_SKYDISP_SHARP_DIMMING_CTRL
	ctrl->onoff_state =1;
#endif
	pr_err("[LCD_ON_SPEED]%s:-\n", __func__);
	return 0;
}

static int mdss_dsi_panel_off(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	pr_debug("%s: ctrl=%p ndx=%d\n", __func__, ctrl, ctrl->ndx);

	mipi  = &pdata->panel_info.mipi;

	if (ctrl->off_cmds.cmd_cnt)
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->off_cmds);

#if defined(CONFIG_MACH_MSM8974_EF56S) || defined(CONFIG_F_SKYDISP_EF60_SS) || \
	defined(CONFIG_F_SKYDISP_EF59_SS)
	if(!ctrl->lcd_on_skip_during_bootup)
		ctrl->lcd_on_skip_during_bootup = true;
#endif
#ifdef CONFIG_F_SKYDISP_HBM_FOR_AMOLED
	pdata->hbm_flag = 0;
#endif
#ifdef CONFIG_F_SKYDISP_SHARP_DIMMING_CTRL
	ctrl->onoff_state =0;
#endif
	pr_err("%s:-\n", __func__);
	return 0;
}

static void mdss_dsi_parse_lane_swap(struct device_node *np, char *dlane_swap)
{
	const char *data;

	*dlane_swap = DSI_LANE_MAP_0123;
	data = of_get_property(np, "qcom,mdss-dsi-lane-map", NULL);
	if (data) {
		if (!strcmp(data, "lane_map_3012"))
			*dlane_swap = DSI_LANE_MAP_3012;
		else if (!strcmp(data, "lane_map_2301"))
			*dlane_swap = DSI_LANE_MAP_2301;
		else if (!strcmp(data, "lane_map_1230"))
			*dlane_swap = DSI_LANE_MAP_1230;
		else if (!strcmp(data, "lane_map_0321"))
			*dlane_swap = DSI_LANE_MAP_0321;
		else if (!strcmp(data, "lane_map_1032"))
			*dlane_swap = DSI_LANE_MAP_1032;
		else if (!strcmp(data, "lane_map_2103"))
			*dlane_swap = DSI_LANE_MAP_2103;
		else if (!strcmp(data, "lane_map_3210"))
			*dlane_swap = DSI_LANE_MAP_3210;
	}
}

static void mdss_dsi_parse_trigger(struct device_node *np, char *trigger,
		char *trigger_key)
{
	const char *data;

	*trigger = DSI_CMD_TRIGGER_SW;
	data = of_get_property(np, trigger_key, NULL);
	if (data) {
		if (!strcmp(data, "none"))
			*trigger = DSI_CMD_TRIGGER_NONE;
		else if (!strcmp(data, "trigger_te"))
			*trigger = DSI_CMD_TRIGGER_TE;
		else if (!strcmp(data, "trigger_sw_seof"))
			*trigger = DSI_CMD_TRIGGER_SW_SEOF;
		else if (!strcmp(data, "trigger_sw_te"))
			*trigger = DSI_CMD_TRIGGER_SW_TE;
	}
}


static int mdss_dsi_parse_dcs_cmds(struct device_node *np,
		struct dsi_panel_cmds *pcmds, char *cmd_key, char *link_key)
{
	const char *data;
	int blen = 0, len;
	char *buf, *bp;
	struct dsi_ctrl_hdr *dchdr;
	int i, cnt;

	data = of_get_property(np, cmd_key, &blen);
	if (!data) {
		pr_err("%s: failed, key=%s\n", __func__, cmd_key);
		return -ENOMEM;
	}

	buf = kzalloc(sizeof(char) * blen, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	memcpy(buf, data, blen);

	/* scan dcs commands */
	bp = buf;
	len = blen;
	cnt = 0;
	while (len > sizeof(*dchdr)) {
		dchdr = (struct dsi_ctrl_hdr *)bp;
		dchdr->dlen = ntohs(dchdr->dlen);
		if (dchdr->dlen > len) {
			pr_err("%s: dtsi cmd=%x error, len=%d",
				__func__, dchdr->dtype, dchdr->dlen);
			goto exit_free;
		}
		bp += sizeof(*dchdr);
		len -= sizeof(*dchdr);
		bp += dchdr->dlen;
		len -= dchdr->dlen;
		cnt++;
	}

	if (len != 0) {
		pr_err("%s: dcs_cmd=%x len=%d error!",
				__func__, buf[0], blen);
		goto exit_free;
	}

	pcmds->cmds = kzalloc(cnt * sizeof(struct dsi_cmd_desc),
						GFP_KERNEL);
	if (!pcmds->cmds)
		goto exit_free;

	pcmds->cmd_cnt = cnt;
	pcmds->buf = buf;
	pcmds->blen = blen;

	bp = buf;
	len = blen;
	for (i = 0; i < cnt; i++) {
		dchdr = (struct dsi_ctrl_hdr *)bp;
		len -= sizeof(*dchdr);
		bp += sizeof(*dchdr);
		pcmds->cmds[i].dchdr = *dchdr;
		pcmds->cmds[i].payload = bp;
		bp += dchdr->dlen;
		len -= dchdr->dlen;
	}

	data = of_get_property(np, link_key, NULL);
	if (data && !strcmp(data, "dsi_hs_mode"))
		pcmds->link_state = DSI_HS_MODE;
	else
		pcmds->link_state = DSI_LP_MODE;

	pr_err("%s: dcs_cmd=%x len=%d, cmd_cnt=%d link_state=%d\n", __func__,
		pcmds->buf[0], pcmds->blen, pcmds->cmd_cnt, pcmds->link_state);


	return 0;

exit_free:
	kfree(buf);
	return -ENOMEM;
}


static int mdss_panel_dt_get_dst_fmt(u32 bpp, char mipi_mode, u32 pixel_packing,
				char *dst_format)
{
	int rc = 0;
	switch (bpp) {
	case 3:
		*dst_format = DSI_CMD_DST_FORMAT_RGB111;
		break;
	case 8:
		*dst_format = DSI_CMD_DST_FORMAT_RGB332;
		break;
	case 12:
		*dst_format = DSI_CMD_DST_FORMAT_RGB444;
		break;
	case 16:
		switch (mipi_mode) {
		case DSI_VIDEO_MODE:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB565;
			break;
		case DSI_CMD_MODE:
			*dst_format = DSI_CMD_DST_FORMAT_RGB565;
			break;
		default:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB565;
			break;
		}
		break;
	case 18:
		switch (mipi_mode) {
		case DSI_VIDEO_MODE:
			if (pixel_packing == 0)
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666;
			else
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666_LOOSE;
			break;
		case DSI_CMD_MODE:
			*dst_format = DSI_CMD_DST_FORMAT_RGB666;
			break;
		default:
			if (pixel_packing == 0)
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666;
			else
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666_LOOSE;
			break;
		}
		break;
	case 24:
		switch (mipi_mode) {
		case DSI_VIDEO_MODE:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
			break;
		case DSI_CMD_MODE:
			*dst_format = DSI_CMD_DST_FORMAT_RGB888;
			break;
		default:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
			break;
		}
		break;
	default:
		rc = -EINVAL;
		break;
	}
	return rc;
}


static int mdss_dsi_parse_fbc_params(struct device_node *np,
				struct mdss_panel_info *panel_info)
{
	int rc, fbc_enabled = 0;
	u32 tmp;

	fbc_enabled = of_property_read_bool(np,	"qcom,mdss-dsi-fbc-enable");
	if (fbc_enabled) {
		pr_debug("%s:%d FBC panel enabled.\n", __func__, __LINE__);
		panel_info->fbc.enabled = 1;
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-bpp", &tmp);
		panel_info->fbc.target_bpp =	(!rc ? tmp : panel_info->bpp);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-packing",
				&tmp);
		panel_info->fbc.comp_mode = (!rc ? tmp : 0);
		panel_info->fbc.qerr_enable = of_property_read_bool(np,
			"qcom,mdss-dsi-fbc-quant-error");
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-bias", &tmp);
		panel_info->fbc.cd_bias = (!rc ? tmp : 0);
		panel_info->fbc.pat_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-pat-mode");
		panel_info->fbc.vlc_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-vlc-mode");
		panel_info->fbc.bflc_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-bflc-mode");
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-h-line-budget",
				&tmp);
		panel_info->fbc.line_x_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-budget-ctrl",
				&tmp);
		panel_info->fbc.block_x_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-block-budget",
				&tmp);
		panel_info->fbc.block_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossless-threshold", &tmp);
		panel_info->fbc.lossless_mode_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossy-threshold", &tmp);
		panel_info->fbc.lossy_mode_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-rgb-threshold",
				&tmp);
		panel_info->fbc.lossy_rgb_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossy-mode-idx", &tmp);
		panel_info->fbc.lossy_mode_idx = (!rc ? tmp : 0);
	} else {
		pr_debug("%s:%d Panel does not support FBC.\n",
				__func__, __LINE__);
		panel_info->fbc.enabled = 0;
		panel_info->fbc.target_bpp =
			panel_info->bpp;
	}
	return 0;
}


static int mdss_dsi_parse_reset_seq(struct device_node *np,
		u32 rst_seq[MDSS_DSI_RST_SEQ_LEN], u32 *rst_len,
		const char *name)
{
	int num = 0, i;
	int rc;
	struct property *data;
	u32 tmp[MDSS_DSI_RST_SEQ_LEN];
	*rst_len = 0;
	data = of_find_property(np, name, &num);
	num /= sizeof(u32);
	if (!data || !num || num > MDSS_DSI_RST_SEQ_LEN || num % 2) {
		pr_debug("%s:%d, error reading %s, length found = %d\n",
			__func__, __LINE__, name, num);
	} else {
		rc = of_property_read_u32_array(np, name, tmp, num);
		if (rc)
			pr_debug("%s:%d, error reading %s, rc = %d\n",
				__func__, __LINE__, name, rc);
		else {
			for (i = 0; i < num; ++i)
				rst_seq[i] = tmp[i];
			*rst_len = num;
		}
	}
	return 0;
}


static int mdss_panel_parse_dt(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	u32 tmp;
	int rc, i, len;
	const char *data;
	static const char *pdest;
	struct mdss_panel_info *pinfo = &(ctrl_pdata->panel_data.panel_info);

	rc = of_property_read_u32(np, "qcom,mdss-dsi-panel-width", &tmp);
	if (rc) {
		pr_err("%s:%d, panel width not specified\n",
						__func__, __LINE__);
		return -EINVAL;
	}
	pinfo->xres = (!rc ? tmp : 640);

	rc = of_property_read_u32(np, "qcom,mdss-dsi-panel-height", &tmp);
	if (rc) {
		pr_err("%s:%d, panel height not specified\n",
						__func__, __LINE__);
		return -EINVAL;
	}
	pinfo->yres = (!rc ? tmp : 480);

	rc = of_property_read_u32(np,
		"qcom,mdss-pan-physical-width-dimension", &tmp);
	pinfo->physical_width = (!rc ? tmp : 0);
	rc = of_property_read_u32(np,
		"qcom,mdss-pan-physical-height-dimension", &tmp);
	pinfo->physical_height = (!rc ? tmp : 0);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-left-border", &tmp);
	pinfo->lcdc.xres_pad = (!rc ? tmp : 0);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-right-border", &tmp);
	if (!rc)
		pinfo->lcdc.xres_pad += tmp;
	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-top-border", &tmp);
	pinfo->lcdc.yres_pad = (!rc ? tmp : 0);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-bottom-border", &tmp);
	if (!rc)
		pinfo->lcdc.yres_pad += tmp;
	rc = of_property_read_u32(np, "qcom,mdss-dsi-bpp", &tmp);
	if (rc) {
		pr_err("%s:%d, bpp not specified\n", __func__, __LINE__);
		return -EINVAL;
	}
	pinfo->bpp = (!rc ? tmp : 24);
	pinfo->mipi.mode = DSI_VIDEO_MODE;
	data = of_get_property(np, "qcom,mdss-dsi-panel-type", NULL);
	if (data && !strncmp(data, "dsi_cmd_mode", 12))
		pinfo->mipi.mode = DSI_CMD_MODE;
	tmp = 0;
	data = of_get_property(np, "qcom,mdss-dsi-pixel-packing", NULL);
	if (data && !strcmp(data, "loose"))
		tmp = 1;
	rc = mdss_panel_dt_get_dst_fmt(pinfo->bpp,
		pinfo->mipi.mode, tmp,
		&(pinfo->mipi.dst_format));
	if (rc) {
		pr_debug("%s: problem determining dst format. Set Default\n",
			__func__);
		pinfo->mipi.dst_format =
			DSI_VIDEO_DST_FORMAT_RGB888;
	}
	pdest = of_get_property(np,
		"qcom,mdss-dsi-panel-destination", NULL);

	if (pdest) {
		if (strlen(pdest) != 9) {
			pr_err("%s: Unknown pdest specified\n", __func__);
			return -EINVAL;
		}
		if (!strcmp(pdest, "display_1"))
			pinfo->pdest = DISPLAY_1;
		else if (!strcmp(pdest, "display_2"))
			pinfo->pdest = DISPLAY_2;
		else {
			pr_debug("%s: pdest not specified. Set Default\n",
								__func__);
			pinfo->pdest = DISPLAY_1;
		}
	} else {
		pr_err("%s: pdest not specified\n", __func__);
		return -EINVAL;
	}
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-front-porch", &tmp);
	pinfo->lcdc.h_front_porch = (!rc ? tmp : 6);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-back-porch", &tmp);
	pinfo->lcdc.h_back_porch = (!rc ? tmp : 6);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-pulse-width", &tmp);
	pinfo->lcdc.h_pulse_width = (!rc ? tmp : 2);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-sync-skew", &tmp);
	pinfo->lcdc.hsync_skew = (!rc ? tmp : 0);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-back-porch", &tmp);
	pinfo->lcdc.v_back_porch = (!rc ? tmp : 6);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-front-porch", &tmp);
	pinfo->lcdc.v_front_porch = (!rc ? tmp : 6);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-pulse-width", &tmp);
	pinfo->lcdc.v_pulse_width = (!rc ? tmp : 2);
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-underflow-color", &tmp);
	pinfo->lcdc.underflow_clr = (!rc ? tmp : 0xff);
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-border-color", &tmp);
	pinfo->lcdc.border_clr = (!rc ? tmp : 0);
	pinfo->bklt_ctrl = UNKNOWN_CTRL;
	data = of_get_property(np, "qcom,mdss-dsi-bl-pmic-control-type", NULL);
	if (data) {
		if (!strncmp(data, "bl_ctrl_wled", 12)) {
			led_trigger_register_simple("bkl-trigger",
				&bl_led_trigger);
			pr_debug("%s: SUCCESS-> WLED TRIGGER register\n",
				__func__);
			ctrl_pdata->bklt_ctrl = BL_WLED;
		} else if (!strncmp(data, "bl_ctrl_pwm", 11)) {
			ctrl_pdata->bklt_ctrl = BL_PWM;
			rc = of_property_read_u32(np,
				"qcom,mdss-dsi-bl-pmic-pwm-frequency", &tmp);
			if (rc) {
				pr_err("%s:%d, Error, panel pwm_period\n",
						__func__, __LINE__);
				return -EINVAL;
			}
			ctrl_pdata->pwm_period = tmp;
			rc = of_property_read_u32(np,
				"qcom,mdss-dsi-bl-pmic-bank-select", &tmp);
			if (rc) {
				pr_err("%s:%d, Error, dsi lpg channel\n",
						__func__, __LINE__);
				return -EINVAL;
			}
			ctrl_pdata->pwm_lpg_chan = tmp;
			tmp = of_get_named_gpio(np,
				"qcom,mdss-dsi-pwm-gpio", 0);
			ctrl_pdata->pwm_pmic_gpio = tmp;
		} else if (!strncmp(data, "bl_ctrl_dcs", 11)) {
			ctrl_pdata->bklt_ctrl = BL_DCS_CMD;
		}
	}
	rc = of_property_read_u32(np, "qcom,mdss-brightness-max-level", &tmp);
	pinfo->brightness_max = (!rc ? tmp : MDSS_MAX_BL_BRIGHTNESS);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-bl-min-level", &tmp);
	pinfo->bl_min = (!rc ? tmp : 0);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-bl-max-level", &tmp);
	pinfo->bl_max = (!rc ? tmp : 255);
	ctrl_pdata->bklt_max = pinfo->bl_max;

	rc = of_property_read_u32(np, "qcom,mdss-dsi-interleave-mode", &tmp);
	pinfo->mipi.interleave_mode = (!rc ? tmp : 0);

	pinfo->mipi.vsync_enable = of_property_read_bool(np,
		"qcom,mdss-dsi-te-check-enable");
	pinfo->mipi.hw_vsync_mode = of_property_read_bool(np,
		"qcom,mdss-dsi-te-using-te-pin");

	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-h-sync-pulse", &tmp);
	pinfo->mipi.pulse_mode_hsa_he = (!rc ? tmp : false);

	pinfo->mipi.hfp_power_stop = of_property_read_bool(np,
		"qcom,mdss-dsi-hfp-power-mode");
	pinfo->mipi.hsa_power_stop = of_property_read_bool(np,
		"qcom,mdss-dsi-hsa-power-mode");
	pinfo->mipi.hbp_power_stop = of_property_read_bool(np,
		"qcom,mdss-dsi-hbp-power-mode");
	pinfo->mipi.bllp_power_stop = of_property_read_bool(np,
		"qcom,mdss-dsi-bllp-power-mode");
	pinfo->mipi.eof_bllp_power_stop = of_property_read_bool(
		np, "qcom,mdss-dsi-bllp-eof-power-mode");
	pinfo->mipi.traffic_mode = DSI_NON_BURST_SYNCH_PULSE;
	data = of_get_property(np, "qcom,mdss-dsi-traffic-mode", NULL);
	if (data) {
		if (!strcmp(data, "non_burst_sync_event"))
			pinfo->mipi.traffic_mode = DSI_NON_BURST_SYNCH_EVENT;
		else if (!strcmp(data, "burst_mode"))
			pinfo->mipi.traffic_mode = DSI_BURST_MODE;
	}
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-te-dcs-command", &tmp);
	pinfo->mipi.insert_dcs_cmd =
			(!rc ? tmp : 1);
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-te-v-sync-continue-lines", &tmp);
	pinfo->mipi.wr_mem_continue =
			(!rc ? tmp : 0x3c);
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-te-v-sync-rd-ptr-irq-line", &tmp);
	pinfo->mipi.wr_mem_start =
			(!rc ? tmp : 0x2c);
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-te-pin-select", &tmp);
	pinfo->mipi.te_sel =
			(!rc ? tmp : 1);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-virtual-channel-id", &tmp);
	pinfo->mipi.vc = (!rc ? tmp : 0);
	pinfo->mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	data = of_get_property(np, "mdss-dsi-color-order", NULL);
	if (data) {
		if (!strcmp(data, "rgb_swap_rbg"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_RBG;
		else if (!strcmp(data, "rgb_swap_bgr"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_BGR;
		else if (!strcmp(data, "rgb_swap_brg"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_BRG;
		else if (!strcmp(data, "rgb_swap_grb"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_GRB;
		else if (!strcmp(data, "rgb_swap_gbr"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_GBR;
	}
	pinfo->mipi.data_lane0 = of_property_read_bool(np,
		"qcom,mdss-dsi-lane-0-state");
	pinfo->mipi.data_lane1 = of_property_read_bool(np,
		"qcom,mdss-dsi-lane-1-state");
	pinfo->mipi.data_lane2 = of_property_read_bool(np,
		"qcom,mdss-dsi-lane-2-state");
	pinfo->mipi.data_lane3 = of_property_read_bool(np,
		"qcom,mdss-dsi-lane-3-state");

	rc = of_property_read_u32(np, "qcom,mdss-dsi-t-clk-pre", &tmp);
	pinfo->mipi.t_clk_pre = (!rc ? tmp : 0x24);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-t-clk-post", &tmp);
	pinfo->mipi.t_clk_post = (!rc ? tmp : 0x03);

	rc = of_property_read_u32(np, "qcom,mdss-dsi-stream", &tmp);
	pinfo->mipi.stream = (!rc ? tmp : 0);

	data = of_get_property(np, "qcom,mdss-dsi-panel-mode-gpio-state", NULL);
	if (data) {
		if (!strcmp(data, "high"))
			pinfo->mode_gpio_state = MODE_GPIO_HIGH;
		else if (!strcmp(data, "low"))
			pinfo->mode_gpio_state = MODE_GPIO_LOW;
	} else {
		pinfo->mode_gpio_state = MODE_GPIO_NOT_VALID;
	}

	rc = of_property_read_u32(np, "qcom,mdss-dsi-panel-framerate", &tmp);
	pinfo->mipi.frame_rate = (!rc ? tmp : 60);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-panel-clockrate", &tmp);
	pinfo->clk_rate = (!rc ? tmp : 0);
	data = of_get_property(np, "qcom,mdss-dsi-panel-timings", &len);
	if ((!data) || (len != 12)) {
		pr_err("%s:%d, Unable to read Phy timing settings",
		       __func__, __LINE__);
		goto error;
	}
	for (i = 0; i < len; i++)
		pinfo->mipi.dsi_phy_db.timing[i] = data[i];

	pinfo->mipi.lp11_init = of_property_read_bool(np,
					"qcom,mdss-dsi-lp11-init");
	rc = of_property_read_u32(np, "qcom,mdss-dsi-init-delay-us", &tmp);
	pinfo->mipi.init_delay = (!rc ? tmp : 0);

	mdss_dsi_parse_fbc_params(np, pinfo);

	mdss_dsi_parse_trigger(np, &(pinfo->mipi.mdp_trigger),
		"qcom,mdss-dsi-mdp-trigger");

	mdss_dsi_parse_trigger(np, &(pinfo->mipi.dma_trigger),
		"qcom,mdss-dsi-dma-trigger");

	mdss_dsi_parse_lane_swap(np, &(pinfo->mipi.dlane_swap));

	mdss_dsi_parse_reset_seq(np, pinfo->rst_seq, &(pinfo->rst_seq_len),
		"qcom,mdss-dsi-reset-sequence");

	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->on_cmds,
		"qcom,mdss-dsi-on-command", "qcom,mdss-dsi-on-command-state");

	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->off_cmds,
		"qcom,mdss-dsi-off-command", "qcom,mdss-dsi-off-command-state");
#ifdef CONFIG_F_SKYDISP_CABC_CONTROL
	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->cabc_cmds,
		"qcom,mdss-dsi-cabc-command", "qcom,mdss-dsi-cabc-command-state");
#endif	
	return 0;

error:
	return -EINVAL;
}

int mdss_dsi_panel_init(struct device_node *node,
	struct mdss_dsi_ctrl_pdata *ctrl_pdata,
	bool cmd_cfg_cont_splash)
{
	int rc = 0;
	static const char *panel_name;
	bool cont_splash_enabled;
	bool partial_update_enabled;
#ifdef CONFIG_F_SKYDISP_SMARTDIMMING
	int i = 0;
	oem_pm_smem_vendor1_data_type *smem_id_vendor1_ptr;
#endif 
	if (!node) {
		pr_err("%s: no panel node\n", __func__);
		return -ENODEV;
	}

	pr_debug("%s:%d\n", __func__, __LINE__);
	panel_name = of_get_property(node, "qcom,mdss-dsi-panel-name", NULL);
	if (!panel_name)
		pr_info("%s:%d, Panel name not specified\n",
						__func__, __LINE__);
	else
		pr_info("%s: Panel Name = %s\n", __func__, panel_name);

	rc = mdss_panel_parse_dt(node, ctrl_pdata);
	if (rc) {
		pr_err("%s:%d panel dt parse failed\n", __func__, __LINE__);
		return rc;
	}

	if (cmd_cfg_cont_splash)
		cont_splash_enabled = of_property_read_bool(node,
				"qcom,cont-splash-enabled");
	else
		cont_splash_enabled = false;
	if (!cont_splash_enabled) {
		pr_info("%s:%d Continuous splash flag not found.\n",
				__func__, __LINE__);
		ctrl_pdata->panel_data.panel_info.cont_splash_enabled = 0;
	} else {
		pr_info("%s:%d Continuous splash flag enabled.\n",
				__func__, __LINE__);

		ctrl_pdata->panel_data.panel_info.cont_splash_enabled = 1;
	}

	partial_update_enabled = of_property_read_bool(node,
						"qcom,partial-update-enabled");
	if (partial_update_enabled) {
		pr_info("%s:%d Partial update enabled.\n", __func__, __LINE__);
		ctrl_pdata->panel_data.panel_info.partial_update_enabled = 1;
		ctrl_pdata->partial_update_fnc = mdss_dsi_panel_partial_update;
	} else {
		pr_info("%s:%d Partial update disabled.\n", __func__, __LINE__);
		ctrl_pdata->panel_data.panel_info.partial_update_enabled = 0;
		ctrl_pdata->partial_update_fnc = NULL;
	}

	ctrl_pdata->on = mdss_dsi_panel_on;
	ctrl_pdata->off = mdss_dsi_panel_off;
	ctrl_pdata->panel_data.set_backlight = mdss_dsi_panel_bl_ctrl;
#ifdef CONFIG_F_SKYDISP_HBM_FOR_AMOLED
	ctrl_pdata->panel_data.hbm_control = hbm_control;
#endif

#ifdef CONFIG_F_SKYDISP_SMARTDIMMING
	INIT_DELAYED_WORK_DEFERRABLE(&ctrl_pdata->panel_read_work, mtp_read_work);
	schedule_delayed_work(&ctrl_pdata->panel_read_work, msecs_to_jiffies(3000));

	for(ctrl_pdata->mtp_cnt = 0;ctrl_pdata->mtp_cnt < V255_MAX; ctrl_pdata->mtp_cnt++){
		for(i = 0 ; i < RGB_MAX; i++){
			if(ctrl_pdata->mtp_cnt == VT)
				ctrl_pdata->panel_read_mtp.panel_gamma_data[i][ctrl_pdata->mtp_cnt] = 0;
			else if(ctrl_pdata->mtp_cnt > VT && ctrl_pdata->mtp_cnt < V255)
				ctrl_pdata->panel_read_mtp.panel_gamma_data[i][ctrl_pdata->mtp_cnt] = 128;
			else if(ctrl_pdata->mtp_cnt == V255)
				ctrl_pdata->panel_read_mtp.panel_gamma_data[i][ctrl_pdata->mtp_cnt] = 256;
		}
	}
	//panel_gamma_data
	ctrl_pdata->mtp_cnt = 0;	
	ctrl_pdata->gamma_sort = panel_gamma_sort;
       ctrl_pdata->gamma_buf = oled_gamma;

	smem_id_vendor1_ptr =  (oem_pm_smem_vendor1_data_type*)smem_alloc(SMEM_ID_VENDOR1,sizeof(oem_pm_smem_vendor1_data_type));
	if(smem_id_vendor1_ptr->power_on_mode == 0){
		ctrl_pdata->offline_charger=1;
	}

	pr_err(" Boot Mode: %s\n",ctrl_pdata->offline_charger ? "Offline": "Online");

#endif
	return 0;
}
