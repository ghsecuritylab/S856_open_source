/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "msm_sensor.h"
#define OV8825_SENSOR_NAME "ov8825"
DEFINE_MSM_MUTEX(ov8825_mut);

#define CONFIG_MSMB_CAMERA_DEBUG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

static struct msm_sensor_ctrl_t ov8825_s_ctrl;

static struct msm_sensor_power_setting ov8825_power_setting[] = {
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VIO,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VANA,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
//#if defined(CONFIG_PRODUCT_SHELLW) || defined(CONFIG_PRODUCT_SHELLE)
    {
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VDIG,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
//#else
//	{
//		.seq_type = SENSOR_VREG,
//		.seq_val = CAM_VDIG,
//		.config_val = 0,
//		.delay = 1,
//	},
//#endif
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VAF,
		.config_val = 0,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_LOW,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_HIGH,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 10,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 24000000,
		.delay = 10,
	},
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
};

static struct v4l2_subdev_info ov8825_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};

static const struct i2c_device_id ov8825_i2c_id[] = {
	{OV8825_SENSOR_NAME, (kernel_ulong_t)&ov8825_s_ctrl},
	{ }
};
#if 1
//int RG_Ratio_Typical = 90;
static int RG_Ratio_Typical = 0x59;
//int BG_Ratio_Typical = 91;
static int BG_Ratio_Typical = 0x59;
struct otp_struct current_otp;

static int R_gain, G_gain, B_gain, G_gain_R, G_gain_B;
uint16_t ov8825_read_i2c(struct msm_sensor_ctrl_t *s_ctrl,uint32_t address,uint16_t *dataval,int datatype)
{
	int rc=0;
	//uint16_t ret_val=0;
	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
			s_ctrl->sensor_i2c_client,
			address, dataval,datatype);
		//printk("read otp register addrees[0x%x] dataval=0x%x, rc=%d\n", address,*dataval, rc);

	if(rc<0)
		printk("read otp register addrees[0x%x] fail, rc=%d\n", address, rc);
	return rc;


}
int ov8825_write_i2c(struct msm_sensor_ctrl_t *s_ctrl,int address, int val,int datatype)
{
	int ret=0;
	ret = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
			s_ctrl->sensor_i2c_client, address, val, datatype);
	return ret;
}


// index: index of otp group. (0, 1, 2)
// return:0, group index is empty
// 1, group index has invalid data
// 2, group index has valid data
int ov8825_check_otp_wb(struct msm_sensor_ctrl_t *s_ctrl, uint16_t index)
{
	uint16_t flag, i;
	uint16_t address;
	// select bank 0
	ov8825_write_i2c(s_ctrl, 0x3d84, 0x08,MSM_CAMERA_I2C_BYTE_DATA );
	// read otp into buffer
	ov8825_write_i2c(s_ctrl,0x3d81, 0x01,MSM_CAMERA_I2C_BYTE_DATA);
	// read flag
	address = 0x3d05 + index*9;
	ov8825_read_i2c(s_ctrl, address,&flag,MSM_CAMERA_I2C_BYTE_DATA);
	// disable otp read
	ov8825_write_i2c(s_ctrl,0x3d81, 0x00,MSM_CAMERA_I2C_BYTE_DATA);
	// clear otp buffer
	for (i=0;i<32;i++) {
		ov8825_write_i2c(s_ctrl,0x3d00 + i, 0x00,MSM_CAMERA_I2C_BYTE_DATA);
	}
	if (!flag) {
		return 0;
	}
	else if ((!(flag & 0x80)) && (flag & 0x7f)) {
		return 2;
	}
	else {
		return 1;
	}
}
// index: index of otp group. (0, 1, 2)
// return:0, group index is empty
// 1, group index has invalid data
// 2, group index has valid data
int ov8825_check_otp_lenc(struct msm_sensor_ctrl_t *s_ctrl, uint16_t index)
{
	uint16_t flag, i;
	uint16_t address;
	// select bank: index*2+1
	ov8825_write_i2c(s_ctrl,0x3d84, 0x09 + index*2,MSM_CAMERA_I2C_BYTE_DATA);
	// read otp into buffer
	ov8825_write_i2c(s_ctrl,0x3d81, 0x01,MSM_CAMERA_I2C_BYTE_DATA);
	// read flag
	address = 0x3d00;
	ov8825_read_i2c(s_ctrl, address, &flag, MSM_CAMERA_I2C_BYTE_DATA);
	flag = flag & 0xc0;
	// disable otp read
	ov8825_write_i2c(s_ctrl,0x3d81, 0x00,MSM_CAMERA_I2C_BYTE_DATA);
	// clear otp buffer
	for (i=0;i<32;i++) {
		ov8825_write_i2c(s_ctrl,0x3d00 + i, 0x00,MSM_CAMERA_I2C_BYTE_DATA);
	}
	if (!flag) {
		return 0;
	}
	else if ((!(flag & 0x80)) && (flag & 0x7f)) {
		return 2;
	}
	else {
		return 1;
	}
}
// index: index of otp group. (0, 1, 2)
// otp_ptr: pointer of otp_struct
// return:0,
int ov8825_read_otp_wb(struct msm_sensor_ctrl_t *s_ctrl,uint16_t index, struct otp_struct * otp_ptr)
{
	int i;
	uint16_t address;
	uint16_t temp=0;
    int loop=0;
    CDBG("%s: index=%d\n",__func__,index);

	// select bank 0
	do{
    	ov8825_write_i2c(s_ctrl,0x3d84, 0x08,MSM_CAMERA_I2C_BYTE_DATA);
    	// read otp into buffer
    	ov8825_write_i2c(s_ctrl,0x3d81, 0x01,MSM_CAMERA_I2C_BYTE_DATA);
    	address = 0x3d05 + index*9;
    	ov8825_read_i2c(s_ctrl, address,&temp, MSM_CAMERA_I2C_BYTE_DATA);
    	(*otp_ptr).module_integrator_id = temp;

    	ov8825_read_i2c(s_ctrl, address + 1,&temp, MSM_CAMERA_I2C_BYTE_DATA);
    	(*otp_ptr).lens_id = temp;

    	ov8825_read_i2c(s_ctrl, address + 2,&temp, MSM_CAMERA_I2C_BYTE_DATA);
    	(*otp_ptr).rg_ratio = temp;

    	ov8825_read_i2c(s_ctrl, address + 3,&temp, MSM_CAMERA_I2C_BYTE_DATA);
    	(*otp_ptr).bg_ratio = temp;

    	ov8825_read_i2c(s_ctrl, address + 4,&temp, MSM_CAMERA_I2C_BYTE_DATA);
    	(*otp_ptr).user_data[0] = temp;

    	ov8825_read_i2c(s_ctrl, address + 5,&temp, MSM_CAMERA_I2C_BYTE_DATA);
    	(*otp_ptr).user_data[1] = temp;

    	ov8825_read_i2c(s_ctrl, address + 6,&temp, MSM_CAMERA_I2C_BYTE_DATA);
    	(*otp_ptr).user_data[2] = temp;

    	ov8825_read_i2c(s_ctrl, address + 7,&temp, MSM_CAMERA_I2C_BYTE_DATA);
    	(*otp_ptr).light_rg = temp;

    	ov8825_read_i2c(s_ctrl, address + 8,&temp, MSM_CAMERA_I2C_BYTE_DATA);
    	(*otp_ptr).light_bg = temp;
    	CDBG("%s module_integrator_id  = 0x%02x\r\n", __func__, otp_ptr->module_integrator_id);
    	CDBG("%s lens_id      		   = 0x%02x\r\n", __func__, otp_ptr->lens_id);
    	CDBG("%s rg_ratio              = 0x%02x\r\n", __func__, otp_ptr->rg_ratio);
    	CDBG("%s bg_ratio              = 0x%02x\r\n", __func__, otp_ptr->bg_ratio);
    	CDBG("%s user_data[0]          = 0x%02x\r\n", __func__, otp_ptr->user_data[0]);
    	CDBG("%s user_data[1]          = 0x%02x\r\n", __func__, otp_ptr->user_data[1]);
    	CDBG("%s user_data[2]          = 0x%02x\r\n", __func__, otp_ptr->user_data[2]);
    	CDBG("%s light_rg              = 0x%02x\r\n", __func__, otp_ptr->light_rg);
    	CDBG("%s light_bg              = 0x%02x\r\n", __func__, otp_ptr->light_bg);
    	// disable otp read
    	ov8825_write_i2c(s_ctrl,0x3d81, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
    	// clear otp buffer
    	for (i=0;i<32;i++) {
    		ov8825_write_i2c(s_ctrl,0x3d00 + i, 0x00, MSM_CAMERA_I2C_BYTE_DATA);
    	}
        if (otp_ptr->module_integrator_id != 0)
        {
            (* otp_ptr).checksum_wb_ok=1;
            CDBG("%s: otp wb data check sum ok read success! checksum_wb_ok=%d\n",__func__, (* otp_ptr).checksum_wb_ok);
            break;
        }
        else
        {
            (* otp_ptr).checksum_wb_ok=0; //check sum error, so be like no otp
            CDBG("%s: otp wb data check sum fail retry n=%d! checksum_wb_ok=%d\n",__func__,loop, (* otp_ptr).checksum_wb_ok);
        }
    	loop ++;
    }while(loop<5);


	return 0;
}
// index: index of otp group. (0, 1, 2)
// otp_ptr: pointer of otp_struct
// return:0,
int ov8825_read_otp_lenc(struct msm_sensor_ctrl_t *s_ctrl,uint16_t index, struct otp_struct * otp_ptr)
{
	uint16_t bank, i;
	uint16_t address;
	uint16_t temp=0;
    int loop=0;
    CDBG("%s: index=%d\n",__func__,index);

	// select bank: index*2+1
    do{
    	bank = index*2 + 1;
    	ov8825_write_i2c(s_ctrl,0x3d84, bank + 0x08,MSM_CAMERA_I2C_BYTE_DATA);
    	// read otp into buffer
    	ov8825_write_i2c(s_ctrl,0x3d81, 0x01,MSM_CAMERA_I2C_BYTE_DATA);
    	address = 0x3d01;
    	for(i=0;i<31;i++) {
    		ov8825_read_i2c(s_ctrl, address,&temp,MSM_CAMERA_I2C_BYTE_DATA);
    		(* otp_ptr).lenc[i]=temp;
    		address++;
    	}
    	// disable otp read
    	ov8825_write_i2c(s_ctrl,0x3d81, 0x00,MSM_CAMERA_I2C_BYTE_DATA);
    	// clear otp buffer
    	for (i=0;i<32;i++) {
    		ov8825_write_i2c(s_ctrl,0x3d00 + i, 0x00,MSM_CAMERA_I2C_BYTE_DATA);
    	}
    	// select next bank
    	bank++;
    	ov8825_write_i2c(s_ctrl,0x3d84, bank + 0x08,MSM_CAMERA_I2C_BYTE_DATA);
    	// read otp
    	ov8825_write_i2c(s_ctrl,0x3d81, 0x01,MSM_CAMERA_I2C_BYTE_DATA);
    	address = 0x3d00;
    	for(i=31;i<62;i++) {
    		ov8825_read_i2c(s_ctrl, address, &temp,MSM_CAMERA_I2C_BYTE_DATA);
    		(* otp_ptr).lenc[i]=temp;
    		address++;
    	}
    	// disable otp read
    	ov8825_write_i2c(s_ctrl,0x3d81, 0x00,MSM_CAMERA_I2C_BYTE_DATA);
    	// clear otp buffer
    	for (i=0;i<32;i++) {
    		ov8825_write_i2c(s_ctrl,0x3d00 + i, 0x00,MSM_CAMERA_I2C_BYTE_DATA);
    	}
        if((* otp_ptr).lenc[0]!= 0)
        {
           (* otp_ptr).checksum_lsc_ok=1;
           CDBG("%s: otp wb data check sum ok read success! checksum_lsc_ok=%d\n",__func__, (* otp_ptr).checksum_lsc_ok);
            break;
        }
        else
        {
           (* otp_ptr).checksum_lsc_ok=0; //check sum error, so be like no otp
           CDBG("%s: otp wb data check sum fail retry n=%d! checksum_lsc_ok=%d\n",__func__,loop, (* otp_ptr).checksum_lsc_ok);
        }
        loop ++;
	}while(loop<5);
	return 0;
}
// R_gain, sensor red gain of AWB, 0x400 =1
// G_gain, sensor green gain of AWB, 0x400 =1
// B_gain, sensor blue gain of AWB, 0x400 =1
// return 0;
int ov8825_update_awb_gain(struct msm_sensor_ctrl_t *s_ctrl,int R_gain, int G_gain, int B_gain,struct otp_struct * otp_ptr)
{
    if((*otp_ptr).checksum_wb_ok==1)
    {
    	if (R_gain>0x400) {
    		ov8825_write_i2c(s_ctrl,0x3400, R_gain>>8,MSM_CAMERA_I2C_BYTE_DATA);
    		ov8825_write_i2c(s_ctrl,0x3401, R_gain & 0x00ff,MSM_CAMERA_I2C_BYTE_DATA);
    	}
    	if (G_gain>0x400) {
    		ov8825_write_i2c(s_ctrl,0x3402, G_gain>>8,MSM_CAMERA_I2C_BYTE_DATA);
    		ov8825_write_i2c(s_ctrl,0x3403, G_gain & 0x00ff,MSM_CAMERA_I2C_BYTE_DATA);
    	}
    	if (B_gain>0x400) {
    		ov8825_write_i2c(s_ctrl,0x3404, B_gain>>8,MSM_CAMERA_I2C_BYTE_DATA);
    		ov8825_write_i2c(s_ctrl,0x3405, B_gain & 0x00ff,MSM_CAMERA_I2C_BYTE_DATA);
    	}
    		CDBG("%s set wb otp success R_gain=0x%x, G_gain=0x%x, B_gain=0x%x",__func__,R_gain,G_gain,B_gain);
    }
    else
    {
    		CDBG("%s checksum_wb_ok==0 can't set wb otp",__func__);
    }
	return 0;
}
// otp_ptr: pointer of otp_struct
int ov8825_update_lenc(struct msm_sensor_ctrl_t *s_ctrl,struct otp_struct * otp_ptr)
{
	uint16_t i, temp;
	if ((*otp_ptr).checksum_lsc_ok == 1)
	{
    	//ov8825_write_i2c(s_ctrl,0x5000, 0x80,MSM_CAMERA_I2C_BYTE_DATA);
    	temp = 0x80 | (*otp_ptr).lenc[0];
    	ov8825_write_i2c(s_ctrl,0x5800, temp,MSM_CAMERA_I2C_BYTE_DATA);
    	//CDBG("%s\n\n",__func__);
    	for(i=0;i<62;i++) {
    		ov8825_write_i2c(s_ctrl,0x5800 + i, (*otp_ptr).lenc[i],MSM_CAMERA_I2C_BYTE_DATA);
    		//CDBG("0x%x,",(*otp_ptr).lenc[i] );
    	}
    		CDBG("%s set lenc otp success",__func__);

    }
    else
    {
    		CDBG("%s checksum_wb_ok==0 can't set lenc otp",__func__);
    }

	//CDBG("\n\n");
	return 0;
}
// call this function after OV8820 initialization
// return value: 0 update success
// 1, no OTP
int ov8825_store_otp_wb(struct msm_sensor_ctrl_t *s_ctrl)
{
	int i;
	uint16_t otp_index;
	int temp;
	int rg,bg;

    if(current_otp.checksum_wb_ok == 0)
    {
	    CDBG("%s \n",__func__);

    	// R/G and B/G of current camera module is read out from sensor OTP
    	// check first OTP with valid data
    	for(i=0;i<3;i++) {
    		temp = ov8825_check_otp_wb(s_ctrl,i);
    		if (temp == 2) {
    			otp_index = i;
    			break;
    		}
    	}
    	if (i==3) {
    		// no valid wb OTP data
    		return 1;
    	}
    	ov8825_read_otp_wb(s_ctrl,otp_index, &current_otp);
    	if(current_otp.light_rg==0) {
    		// no light source information in OTP, light factor = 1
    		rg = current_otp.rg_ratio;
    	}
    	else {
    		rg = current_otp.rg_ratio * (current_otp.light_rg +128) / 256;
    	}
    	if(current_otp.light_bg==0) {
    		// not light source information in OTP, light factor = 1
    		bg = current_otp.bg_ratio;
    	}
    	else {
    		bg = current_otp.bg_ratio * (current_otp.light_bg +128) / 256;
    	}
    	//calculate G gain
    	//0x400 = 1x gain
    	if(bg < BG_Ratio_Typical) {
    		if (rg< RG_Ratio_Typical) {
    			// current_otp.bg_ratio < BG_Ratio_typical &&
    			// current_otp.rg_ratio < RG_Ratio_typical
    			G_gain = 0x400;
    			B_gain = 0x400 * BG_Ratio_Typical / bg;
    			R_gain = 0x400 * RG_Ratio_Typical / rg;
    		}
    		else {
    			// current_otp.bg_ratio < BG_Ratio_typical &&
    			// current_otp.rg_ratio >= RG_Ratio_typical
    			R_gain = 0x400;
    			G_gain = 0x400 * rg / RG_Ratio_Typical;
    			B_gain = G_gain * BG_Ratio_Typical /bg;
    		}
    	}
    	else {
    	if (rg < RG_Ratio_Typical) {
    			// current_otp.bg_ratio >= BG_Ratio_typical &&
    			// current_otp.rg_ratio < RG_Ratio_typical
    			B_gain = 0x400;
    			G_gain = 0x400 * bg / BG_Ratio_Typical;
    			R_gain = G_gain * RG_Ratio_Typical / rg;
    		}
    		else {
    			// current_otp.bg_ratio >= BG_Ratio_typical &&
    			// current_otp.rg_ratio >= RG_Ratio_typical
    			G_gain_B = 0x400 * bg / BG_Ratio_Typical;
    			G_gain_R = 0x400 * rg / RG_Ratio_Typical;
    			if(G_gain_B > G_gain_R ) {
    				B_gain = 0x400;
    				G_gain = G_gain_B;
    				R_gain = G_gain * RG_Ratio_Typical /rg;
    			}
    			else {
    				R_gain = 0x400;
    				G_gain = G_gain_R;
    				B_gain = G_gain * BG_Ratio_Typical / bg;
    			}
    		}
    	}
	}
	return 0;
}
// call this function after OV8820 initialization
// return value: 0 update success
// 1, no OTP
int ov8825_store_otp_lenc(struct msm_sensor_ctrl_t *s_ctrl)
{
	uint16_t i;
	uint16_t otp_index;
	int temp;
    if(current_otp.checksum_lsc_ok == 0)
    {
    	CDBG("%s \n",__func__);
    	// check first lens correction OTP with valid data
    	for(i=0;i<3;i++) {
    		temp = ov8825_check_otp_lenc(s_ctrl,i);
    		if (temp == 2) {
    			otp_index = i;
    			break;
    		}
    	}
    	if (i==3) {
    		// no valid wb OTP data
    		return 1;
    	}
    	ov8825_read_otp_lenc(s_ctrl,otp_index, &current_otp);
	}
	// success
	return 0;
}

#endif


int32_t ov8825_sensor_get_otp(struct msm_sensor_ctrl_t *s_ctrl,struct msm_sensor_otp_params *sensor_otp_params_ptr)
{
/* + add for vcm 657(otp 4) & 658(otp 0) */
    (* sensor_otp_params_ptr).module_integrator_id=current_otp.user_data[0];//current_otp.module_integrator_id;
    pr_err("%s  ljk ov8825_sensor_get_otp MID=%d",__func__,(* sensor_otp_params_ptr).module_integrator_id);
    return 0;
/* + end */
}


int32_t ov8825_sensor_read_mid(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc=-1;

// start stream
	ov8825_write_i2c(s_ctrl, 0x0100, 0x01,MSM_CAMERA_I2C_BYTE_DATA );
	ov8825_write_i2c(s_ctrl, 0x301c, 0xf0,MSM_CAMERA_I2C_BYTE_DATA );
	ov8825_write_i2c(s_ctrl, 0x301a, 0x70,MSM_CAMERA_I2C_BYTE_DATA );
    msleep(5);

	rc=ov8825_store_otp_wb(s_ctrl);
	if(rc!=0)
		printk("no valid wb otp wb data rc=%d\n", rc);


	rc=ov8825_store_otp_lenc(s_ctrl);
	if(rc!=0)
		printk("no valid wb otp lenc data rc=%d\n", rc);

//stop stream
	ov8825_write_i2c(s_ctrl, 0x301a, 0x71,MSM_CAMERA_I2C_BYTE_DATA );
	ov8825_write_i2c(s_ctrl, 0x301c, 0xf4,MSM_CAMERA_I2C_BYTE_DATA );
	ov8825_write_i2c(s_ctrl, 0x0100, 0x00,MSM_CAMERA_I2C_BYTE_DATA );

   return rc;
}


int32_t ov8825_sensor_otp_proc(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc=0;
	printk("ov8825_sensor_otp_proc start\n");

	if(!s_ctrl->sensor_i2c_client)
	{
		printk("sensor_i2c_client is null fail\n");
		return 0;

	}
	if(!s_ctrl->sensor_i2c_client->i2c_func_tbl)
	{
		printk("i2c_func_tbl is null fail\n");
		return 0;

	}
	if(!s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read)
	{
		printk("i2c_read is null fail\n");
		return 0;

	}
	if(!s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write)
	{
		printk("i2c_write is null fail\n");
		return 0;

	}


	rc=ov8825_store_otp_wb(s_ctrl);
	if(rc!=0)
		printk("no valid wb otp wb data rc=%d\n", rc);


	rc=ov8825_store_otp_lenc(s_ctrl);
	if(rc!=0)
		printk("no valid wb otp lenc data rc=%d\n", rc);

	ov8825_update_lenc(s_ctrl,&current_otp);
	ov8825_update_awb_gain(s_ctrl,R_gain, G_gain, B_gain,&current_otp);

	if(rc!=0)
		printk("no valid len otp data rc=%d\n", rc);
	return rc;

}


static int32_t msm_ov8825_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &ov8825_s_ctrl);
}

static struct i2c_driver ov8825_i2c_driver = {
	.id_table = ov8825_i2c_id,
	.probe  = msm_ov8825_i2c_probe,
	.driver = {
		.name = OV8825_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov8825_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static const struct of_device_id ov8825_dt_match[] = {
	{.compatible = "qcom,ov8825", .data = &ov8825_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, ov8825_dt_match);

static struct platform_driver ov8825_platform_driver = {
	.driver = {
		.name = "qcom,ov8825",
		.owner = THIS_MODULE,
		.of_match_table = ov8825_dt_match,
	},
};

static int32_t ov8825_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;
	match = of_match_device(ov8825_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}

static int __init ov8825_init_module(void)
{
	int32_t rc = 0;
	pr_info("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_probe(&ov8825_platform_driver,
		ov8825_platform_probe);
	if (!rc)
		return rc;
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&ov8825_i2c_driver);
}

static void __exit ov8825_exit_module(void)
{
	pr_info("%s:%d\n", __func__, __LINE__);
	if (ov8825_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&ov8825_s_ctrl);
		platform_driver_unregister(&ov8825_platform_driver);
	} else
		i2c_del_driver(&ov8825_i2c_driver);
	return;
}

static struct msm_sensor_ctrl_t ov8825_s_ctrl = {
	.sensor_i2c_client = &ov8825_sensor_i2c_client,
	.power_setting_array.power_setting = ov8825_power_setting,
	.power_setting_array.size = ARRAY_SIZE(ov8825_power_setting),
	.msm_sensor_mutex = &ov8825_mut,
	.sensor_v4l2_subdev_info = ov8825_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov8825_subdev_info),
	.sensor_read_mid = ov8825_sensor_read_mid,
	.sensor_otp_proc = ov8825_sensor_otp_proc,
	.sensor_get_otp = ov8825_sensor_get_otp,
};

module_init(ov8825_init_module);
module_exit(ov8825_exit_module);
MODULE_DESCRIPTION("ov8825");
MODULE_LICENSE("GPL v2");
