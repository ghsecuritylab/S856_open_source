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

#define OV9760_SENSOR_NAME "ov9760"
DEFINE_MSM_MUTEX(ov9760_mut);

#define CONFIG_MSMB_CAMERA_DEBUG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

static struct msm_sensor_ctrl_t ov9760_s_ctrl;

static struct msm_sensor_power_setting ov9760_power_setting[] = {
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VANA,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VIO,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VDIG,
		.config_val = 0,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_LOW,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
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

static struct v4l2_subdev_info ov9760_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};
#if 1
struct otp_struct_ov9760 {
          int module_integrator_id;
          int lens_id;
          int production_year;
          int production_month;
          int production_day;
          int rg_ratio;
          int bg_ratio;
          int typical_rg;
          int typical_bg;
          int otp_version;
          int temperature;
          int IR_BG;
          int checksum_AWB;
          int VCM_start;
          int VCM_end;
          int VCM_dir;
          int lenc[24];
          int checksum_LSC;
          int checksum_wb_ok;
          int checksum_lsc_ok;
};
static struct otp_struct_ov9760 current_otp;


static int R_gain, G_gain, B_gain, G_gain_R, G_gain_B;

//int RG_Ratio_Typical = 90;
static int RG_Ratio_Typical = 0x17a;
//int BG_Ratio_Typical = 91;
static int BG_Ratio_Typical = 0x139;
uint16_t OV9760_read_i2c(struct msm_sensor_ctrl_t *s_ctrl,uint32_t address)
{
	int rc=0;
	uint16_t ret_val=0;
	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
			s_ctrl->sensor_i2c_client,
			address, &ret_val,MSM_CAMERA_I2C_BYTE_DATA);
	if(rc<0)
		printk("read otp register addrees[0x%x] fail, rc=%d\n", address, rc);
	return ret_val;


}
int OV9760_write_i2c(struct msm_sensor_ctrl_t *s_ctrl,int address, int val)
{
	int ret=0;
	ret = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
			s_ctrl->sensor_i2c_client, address, val, MSM_CAMERA_I2C_BYTE_DATA);
	return ret;
}


// index: index of otp group. (1, 2, 3)
// return:              0, group index is empty
//                      1, group index has invalid data
//                      2, group index has valid data
int check_otp_wb(struct msm_sensor_ctrl_t *s_ctrl,int index)
{
           int flag, i;
           int bank, address;
           // select bank index
           bank = 0xc0 | index;
           OV9760_write_i2c(s_ctrl,0x3d84, bank);
           // read otp into buffer
           OV9760_write_i2c(s_ctrl,0x3d81, 0x01);
           msleep(5);
           // read flag
           address = 0x3d00;
           flag = OV9760_read_i2c(s_ctrl,address);
           flag = flag & 0xc0;
           // clear otp buffer
           for (i=0;i<16;i++) {
                        OV9760_write_i2c(s_ctrl,0x3d00 + i, 0x00);
           }
           if (flag == 0x00) {
                       return 0;
           }
           else if (flag & 0x80) {
                       return 1;
           }
           else {
                       return 2;
           }
}
// index: index of otp group. (1, 2, 3)
// return:             0, group index is empty
//                     1, group index has invalid data
//                     2, group index has valid data
int check_otp_lenc(struct msm_sensor_ctrl_t *s_ctrl,int index)
{
           int flag, i;
           int bank, address;
           // select bank index
           bank = 0xc0 | ((index+1) *2);
           OV9760_write_i2c(s_ctrl,0x3d84, bank);
           // read otp into buffer
           OV9760_write_i2c(s_ctrl,0x3d81, 0x01);
           msleep(5);
           // read flag
           address = 0x3d00;
           flag = OV9760_read_i2c(s_ctrl,address);
           flag = flag & 0xc0;
           // clear otp buffer
           for (i=0;i<16;i++) {
                        OV9760_write_i2c(s_ctrl,0x3d00 + i, 0x00);
           }
           if (flag == 0x00) {
                        return 0;
           }
           else if (flag & 0x80) {
                        return 1;
           }
           else {
                        return 2;
           }
}
// index: index of otp group. (1, 2, 3)
// otp_ptr: pointer of otp_struct
// return:              0,
int read_otp_wb(struct msm_sensor_ctrl_t *s_ctrl,int index, struct otp_struct_ov9760 *otp_ptr)
{
       int i, bank;
       int address;
       int temp;
       int loop=0;
       int value=0;
       int sum_wb=0;
       // select bank index
       CDBG("%s: index=%d\n",__func__,index);
       do{
           bank = 0xc0 | index;
           OV9760_write_i2c(s_ctrl,0x3d84, bank);
           // read otp into buffer
           OV9760_write_i2c(s_ctrl,0x3d81, 0x01);
           msleep(5);

           address = 0x3d00;

           (*otp_ptr).module_integrator_id = OV9760_read_i2c(s_ctrl,address + 1);
           sum_wb=sum_wb+(*otp_ptr).module_integrator_id; //+1
           CDBG("%s: sum_wb=%d! module_integrator_id=%d\n",__func__,sum_wb, (* otp_ptr).module_integrator_id);

           (*otp_ptr).lens_id = OV9760_read_i2c(s_ctrl,address + 2);
           sum_wb=sum_wb+(*otp_ptr).lens_id; //+2
           CDBG("%s: sum_wb=%d! lens_id=%d\n",__func__,sum_wb, (* otp_ptr).lens_id);

           (*otp_ptr).production_year = OV9760_read_i2c(s_ctrl,address + 3);
           sum_wb=sum_wb+(*otp_ptr).production_year; //+3
           CDBG("%s: sum_wb=%d! production_year=%d\n",__func__,sum_wb, (* otp_ptr).production_year);

           (*otp_ptr).production_month = OV9760_read_i2c(s_ctrl,address + 4);
           sum_wb=sum_wb+(*otp_ptr).production_month;//+4
           CDBG("%s: sum_wb=%d! production_month=%d\n",__func__,sum_wb, (* otp_ptr).production_month);
           (*otp_ptr).production_day = OV9760_read_i2c(s_ctrl,address + 5);
           sum_wb=sum_wb+(*otp_ptr).production_day;//+5
           CDBG("%s: sum_wb=%d! production_day=%d\n",__func__,sum_wb, (* otp_ptr).production_day);
           temp = OV9760_read_i2c(s_ctrl,address + 10);
           sum_wb=sum_wb+temp;//+10
           CDBG("%s: sum_wb=%d! temp10 %d\n",__func__,sum_wb, temp);

           value=OV9760_read_i2c(s_ctrl,address + 6);
           sum_wb=sum_wb+value;//+6
           CDBG("%s: sum_wb=%d! temp6 %d\n",__func__,sum_wb, value);

           (*otp_ptr).rg_ratio = (value<<2) + ((temp>>6) & 0x03);
           value=OV9760_read_i2c(s_ctrl,address + 7);//+7
           sum_wb=sum_wb+value;
           CDBG("%s: sum_wb=%d! temp7 %d\n",__func__,sum_wb, value);

           (*otp_ptr).bg_ratio = (value<<2) + ((temp>>4) & 0x03);
           value=OV9760_read_i2c(s_ctrl,address + 8);//+8
           sum_wb=sum_wb+value;
           CDBG("%s: sum_wb=%d! temp8 %d\n",__func__,sum_wb, value);

           (*otp_ptr).typical_rg = (value<<2) + ((temp>>2) & 0x03);
           value=OV9760_read_i2c(s_ctrl,address + 9);//+9
           sum_wb=sum_wb+value;
           CDBG("%s: sum_wb=%d! temp9 %d\n",__func__,sum_wb, value);

           (*otp_ptr).typical_bg = (value<<2) + (temp & 0x03);
           value=OV9760_read_i2c(s_ctrl,address + 11);//+11
           sum_wb=sum_wb+value;
           CDBG("%s: sum_wb=%d! temp11 %d\n",__func__,sum_wb, value);

           (*otp_ptr).otp_version = (value>>5)&0x07;
           (*otp_ptr).temperature = (value>>3)&0x03;
           (*otp_ptr).IR_BG = value&0x07;
          // temp = OV9760_read_i2c(s_ctrl,address + 15);
         //  (* otp_ptr).VCM_start = (OV13850_read_i2c(address + 13)<<2) | ((temp>>2) & 0x03);
          // (* otp_ptr).VCM_max = (OV13850_read_i2c(address + 14)<< 2) | (temp & 0x03);
          // (* otp_ptr).VCM_dir = (temp>>6) & 0x03;
           // clear otp buffer
           (*otp_ptr).checksum_AWB = OV9760_read_i2c(s_ctrl,address + 12);
           CDBG("%s: checksum_AWB %d\n",__func__,(*otp_ptr).checksum_AWB);
           for (i=0;i<16;i++) {
                      OV9760_write_i2c(s_ctrl,0x3d00 + i, 0x00);
           }
        if((*otp_ptr).checksum_AWB!=0)
        {
            CDBG("%s: sum_wb=0x%x   sum_wb 0/0 0xff = 0x%x (* p_otp).checksum_AWB=0x%x n=%d new checksum=0x%x\n",__func__,sum_wb, (sum_wb%(0xff)),(* otp_ptr).checksum_AWB,loop,sum_wb%0xff+1);
            if((sum_wb%0xff + 1) == (* otp_ptr).checksum_AWB)
            {
                (* otp_ptr).checksum_wb_ok=1;
                CDBG("%s: otp wb data check sum ok read success! checksum_wb_ok=%d\n",__func__, (* otp_ptr).checksum_wb_ok);
                break; //no need to loop any more
            }
            else
            {
                (* otp_ptr).checksum_wb_ok=0; //check sum error, so be like no otp
                CDBG("%s: otp wb data check sum fail retry n=%d! checksum_wb_ok=%d\n",__func__,loop, (* otp_ptr).checksum_wb_ok);
            }
        }
        else //checksum==0
        {
            CDBG("%s: old module no checksum data!\n",__func__);
            break;
        }


        loop++;
       }while(loop<5);
    	CDBG("%s loop  = 0x%x\r\n", __func__, loop);
    	CDBG("%s module_integrator_id  = 0x%02x\r\n", __func__, otp_ptr->module_integrator_id);
    	CDBG("%s lens_id      		   = 0x%02x\r\n", __func__, otp_ptr->lens_id);
    	CDBG("%s rg_ratio              = 0x%02x\r\n", __func__, otp_ptr->rg_ratio);
    	CDBG("%s bg_ratio              = 0x%02x\r\n", __func__, otp_ptr->bg_ratio);
    	CDBG("%s typical_rg            = 0x%02x\r\n", __func__, otp_ptr->typical_rg);
    	CDBG("%s typical_bg            = 0x%02x\r\n", __func__, otp_ptr->typical_bg);
    	CDBG("%s otp_version           = 0x%02x\r\n", __func__, otp_ptr->otp_version);
    	CDBG("%s IR_BG                 = 0x%02x\r\n", __func__, otp_ptr->IR_BG);
    	CDBG("%s checksum_AWB          = 0x%02x\r\n", __func__, otp_ptr->checksum_AWB);
           return 0;
}
// index: index of otp group. (1, 2, 3)
// otp_ptr: pointer of otp_struct
// return:            0,
int read_otp_lenc(struct msm_sensor_ctrl_t *s_ctrl,int index, struct otp_struct_ov9760 *otp_ptr)
{
       int i, bank;
       int address;
       int temp;
       int loop=0;
       int sum_lsc=0;
       CDBG("%s: index=%d\n",__func__,index);
       do{
           // select bank index
           bank = 0xc0 | ((index+1) *2);
           OV9760_write_i2c(s_ctrl,0x3d84, bank);
            // read otp into buffer
            OV9760_write_i2c(s_ctrl,0x3d81, 0x01);
            msleep(5);
            address = 0x3d01;
            temp = OV9760_read_i2c(s_ctrl,address + 2);
            sum_lsc=sum_lsc+temp;//+2
            (*otp_ptr).lenc[0] = (temp & 0x39)>>3;                         // Red_X0[10:8]
            (*otp_ptr).lenc[1] = OV9760_read_i2c(s_ctrl,address);                 // Red_X0[7:0]
            sum_lsc=sum_lsc+(*otp_ptr).lenc[1];//+0

            (*otp_ptr).lenc[2] = temp & 0x07;                              // Red_Y0[10;8}
            (*otp_ptr).lenc[3] = OV9760_read_i2c(s_ctrl,address + 1);    // Red_Y0[7:0]
            sum_lsc=sum_lsc+(*otp_ptr).lenc[3];//+1

            (*otp_ptr).lenc[4] = OV9760_read_i2c(s_ctrl,address + 3);    // Red_A1
            sum_lsc=sum_lsc+(*otp_ptr).lenc[4];//+3

            temp = OV9760_read_i2c(s_ctrl,address + 5);
            sum_lsc=sum_lsc+temp;//+5

            (*otp_ptr).lenc[5] = temp>>4;                         // Red_A2
            (*otp_ptr).lenc[6] = OV9760_read_i2c(s_ctrl,address + 4);    // Red_B1
            sum_lsc=sum_lsc+(*otp_ptr).lenc[6];//+4

            (*otp_ptr).lenc[7] = temp & 0x0f;                     // Red_B2
            temp = OV9760_read_i2c(s_ctrl,address + 8);
            sum_lsc=sum_lsc+temp;//+8

            (*otp_ptr).lenc[8] = (temp & 0x39)>>3;                // Green_X0[10:8]
            (*otp_ptr).lenc[9] = OV9760_read_i2c(s_ctrl,address + 6);    // Green_X0[7:0]
            sum_lsc=sum_lsc+(*otp_ptr).lenc[9];//+6

            (*otp_ptr).lenc[10] = temp & 0x07;                    // Green_Y0[10;8}
            (*otp_ptr).lenc[11] = OV9760_read_i2c(s_ctrl,address + 7);   // Green_Y0[7:0]
            sum_lsc=sum_lsc+(*otp_ptr).lenc[11];//+7

            (*otp_ptr).lenc[12] = OV9760_read_i2c(s_ctrl,address + 9);   // Green_A1
            sum_lsc=sum_lsc+(*otp_ptr).lenc[12];//+9

            temp = OV9760_read_i2c(s_ctrl,address + 11);
            sum_lsc=sum_lsc+temp;//+11

            (*otp_ptr).lenc[13] = temp>>4;                        // Green_A2
            (*otp_ptr).lenc[14] = OV9760_read_i2c(s_ctrl,address + 10);  // Green_B1
            sum_lsc=sum_lsc+(*otp_ptr).lenc[14];//+10

            (*otp_ptr).lenc[15] = temp & 0x0f;                    // Green_B2
            temp = OV9760_read_i2c(s_ctrl,address + 14);
            sum_lsc=sum_lsc+temp;//+14

            (*otp_ptr).lenc[16] = (temp & 0x39)>>3;               // Blue_X0[10:8]
            (*otp_ptr).lenc[17] = OV9760_read_i2c(s_ctrl,address + 12);  // Blue_X0[7:0]
            sum_lsc=sum_lsc+(*otp_ptr).lenc[17];//+12

            (*otp_ptr).lenc[18] = temp & 0x07;                    // Blue_Y0[10;8}
            (*otp_ptr).lenc[19] = OV9760_read_i2c(s_ctrl,address + 13);  // Blue_Y0[7:0]
            sum_lsc=sum_lsc+(*otp_ptr).lenc[19];//+13

            bank ++;
            OV9760_write_i2c(s_ctrl,0x3d84, bank);
            // read otp into buffer
            OV9760_write_i2c(s_ctrl,0x3d81, 0x01);
            address = 0x3d00;
            (*otp_ptr).lenc[20] = OV9760_read_i2c(s_ctrl,address);       // Blue_A1
            sum_lsc=sum_lsc+(*otp_ptr).lenc[20];//+0

            temp = OV9760_read_i2c(s_ctrl,address + 2);
            sum_lsc=sum_lsc+temp;//+2

            (*otp_ptr).lenc[21] = temp>>4;                        // Blue_A2
            (*otp_ptr).lenc[22] = OV9760_read_i2c(s_ctrl,address + 1);   // Blue_B1
            sum_lsc=sum_lsc+(*otp_ptr).lenc[22];//+1

            (*otp_ptr).lenc[23] = temp & 0x0f;                    // Blue_B2
            (*otp_ptr).checksum_LSC = OV9760_read_i2c(s_ctrl,address + 3);

            // clear otp buffer
            for (i=0;i<16;i++) {
                       OV9760_write_i2c(s_ctrl,0x3d00 + i, 0x00);
                        }

        if((*otp_ptr).checksum_LSC!=0)
        {
            CDBG("%s: sum_lsc=0x%x   sum_lsc 0/0 0xff = 0x%x (* otp_ptr).checksum_LSC=0x%x n=%d new checksum=0x%x\n",__func__,sum_lsc, (sum_lsc%(0xff)),(* otp_ptr).checksum_LSC,loop,sum_lsc%0xff+1);
            if((sum_lsc%0xff + 1) == (* otp_ptr).checksum_LSC)
            {
                (* otp_ptr).checksum_lsc_ok=1;
                CDBG("%s: otp wb data check sum ok read success! checksum_lsc_ok=%d\n",__func__, (* otp_ptr).checksum_lsc_ok);
                break; //no need to loop any more
            }
            else
            {
                (* otp_ptr).checksum_lsc_ok=0; //check sum error, so be like no otp
                CDBG("%s: otp wb data check sum fail retry n=%d! checksum_lsc_ok=%d\n",__func__,loop, (* otp_ptr).checksum_lsc_ok);
            }
        }
        else //checksum==0
        {
            CDBG("%s: old module no checksum data!\n",__func__);
            break;
        }
           for (i=0;i<16;i++) {
                      OV9760_write_i2c(s_ctrl,0x3d00 + i, 0x00);
           }

        loop++;
       }while(loop<5);
       return 0;
}
// R_gain, sensor red gain of AWB, 0x400 =1
// G_gain, sensor green gain of AWB, 0x400 =1
// B_gain, sensor blue gain of AWB, 0x400 =1
// return 0;
int update_awb_gain(struct msm_sensor_ctrl_t *s_ctrl,int R_gain, int G_gain, int B_gain)
{
            if (R_gain>0x400) {
                       OV9760_write_i2c(s_ctrl,0x5180, R_gain>>8);
                       OV9760_write_i2c(s_ctrl,0x5181, R_gain & 0x00ff);
            }
            if (G_gain>0x400) {
                       OV9760_write_i2c(s_ctrl,0x5182, G_gain>>8);
                       OV9760_write_i2c(s_ctrl,0x5183, G_gain & 0x00ff);
            }
            if (B_gain>0x400) {
                       OV9760_write_i2c(s_ctrl,0x5184, B_gain>>8);
                       OV9760_write_i2c(s_ctrl,0x5185, B_gain & 0x00ff);
            }
		    CDBG("%s R_gain=0x%x, G_gain=0x%x, B_gain=0x%x",__func__,R_gain,G_gain,B_gain);
            return 0;
}

// call this function after OV9760 initialization
// return value:       0 update success
//                     1, no OTP
int update_otp_wb(struct msm_sensor_ctrl_t *s_ctrl)
{
      if(current_otp.checksum_wb_ok == 1)
        update_awb_gain(s_ctrl,R_gain, G_gain, B_gain);
      return 0;
}

int store_otp_wb(struct msm_sensor_ctrl_t *s_ctrl)
{
        int i;
        int otp_index;
        int temp;
        int rg,bg;
            // R/G and B/G of current camera module is read out from sensor OTP
            // check first OTP with valid data
        if(current_otp.checksum_wb_ok == 0)
        {
            for(i=1;i<=3;i++) {
                       temp = check_otp_wb(s_ctrl,i);
                       if (temp == 2) {
                                  otp_index = i;
                            	  CDBG("%s otp_index=0x%x",__func__,otp_index);
                                  break;
                       }
            }
            if (i>3) {
                       // no valid wb OTP data
           return 1;
            }
            read_otp_wb(s_ctrl,otp_index, &current_otp);
            RG_Ratio_Typical=current_otp.typical_rg;
            BG_Ratio_Typical=current_otp.typical_bg;
		    CDBG("%s RG_Ratio_Typical=0x%x, BG_Ratio_Typical=0x%x",__func__,RG_Ratio_Typical,BG_Ratio_Typical);
            // no light source information in OTP, light factor = 1
            rg = current_otp.rg_ratio;
            // not light source information in OTP, light factor = 1
            bg = current_otp.bg_ratio;
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
// otp_ptr: pointer of otp_struct
int update_lenc(struct msm_sensor_ctrl_t *s_ctrl,struct otp_struct_ov9760 * otp_ptr)
{
   int i, temp;
   temp = OV9760_read_i2c(s_ctrl,0x5000);
   temp = 0x80 | temp;
   OV9760_write_i2c(s_ctrl,0x5000, temp);
   for(i=0;i<24;i++) {
      CDBG("%s (*otp_ptr).lenc[%d]=0x%x",__func__,i,(*otp_ptr).lenc[i]);
      OV9760_write_i2c(s_ctrl,0x5800 + i, (*otp_ptr).lenc[i]);
   }
   return 0;
}
int update_otp_lenc(struct msm_sensor_ctrl_t *s_ctrl)
{
   if(current_otp.checksum_lsc_ok == 1)
      update_lenc(s_ctrl,&current_otp);
   return 0;
}
// call this function after OV9760 initialization
// return value: 0 update success
//                     1, no OTP
int store_otp_lenc(struct msm_sensor_ctrl_t *s_ctrl)
{
        int i;
        int otp_index;
        int temp;
        if(current_otp.checksum_lsc_ok == 0)
        {

            // check first lens correction OTP with valid data
            for(i=1;i<=3;i++) {
                       temp = check_otp_lenc(s_ctrl,i);
                       if (temp == 2) {
                                  otp_index = i;
                            	  CDBG("%s otp_index=0x%x",__func__,otp_index);
                                  break;
                       }
            }
            if (i>3) {
                       // no valid WB OTP data
                       return 1;
            }
            read_otp_lenc(s_ctrl,otp_index, &current_otp);
        }
            // success
        return 0;
}
// call this function after OV9760 initialization
// return value:        1 use CP data from REG3D0A
//                      2 use Module data from REG3D0A
//                      0 data ErRoR
int update_blc_ratio(struct msm_sensor_ctrl_t *s_ctrl)
{
            int K;
            int temp;
            OV9760_write_i2c(s_ctrl,0x3d84, 0xdf);
            OV9760_write_i2c(s_ctrl,0x3d81, 0x01);
            msleep(5);
            K = OV9760_read_i2c(s_ctrl,0x3d0b);
            if (K != 0) {
                        if (K >= 0x15 && K <= 0x40) {
                                   // auto load mode
                                   temp = OV9760_read_i2c(s_ctrl,0x4000);
                                   temp &= 0x9f;
                                   OV9760_write_i2c(s_ctrl,0x4000, temp);
                                   return 2;
                        }
            }
            K = OV9760_read_i2c(s_ctrl,0x3d0a);
            if (K >= 0x10 && K <= 0x40) {
                        // manual load mode
                        OV9760_write_i2c(s_ctrl,0x4006, K);
                        temp = OV9760_read_i2c(s_ctrl,0x4000);
                        temp |= 0x40;
                        temp &= 0xdf;
                        OV9760_write_i2c(s_ctrl,0x4000, temp);
                        return 1;
            }
            else {
                        // set to default
                        OV9760_write_i2c(s_ctrl,0x4006, 0x20);
                        temp = OV9760_read_i2c(s_ctrl,0x4000);
                        temp |= 0x40;
                        temp &= 0xdf;
                        OV9760_write_i2c(s_ctrl,0x4000, temp);
                        return 0;
            }
}

#endif
int32_t ov9760_sensor_get_otp(struct msm_sensor_ctrl_t *s_ctrl,struct msm_sensor_otp_params *sensor_otp_params_ptr)
{
    (* sensor_otp_params_ptr).module_integrator_id=current_otp.module_integrator_id;
    pr_err("%s  ljk ov9760_sensor_get_otp MID=%d",__func__,(* sensor_otp_params_ptr).module_integrator_id);
    return 0;
}
int32_t ov9760_sensor_read_mid(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc=-1;

    OV9760_write_i2c(s_ctrl,0x0100, 0x01);

	rc=store_otp_wb(s_ctrl);
	if(rc!=0)
		printk("no valid wb otp data rc=%d\n", rc);
	rc=store_otp_lenc(s_ctrl);
	if(rc!=0)
		printk("no valid len otp data rc=%d\n", rc);

    OV9760_write_i2c(s_ctrl,0x0100, 0x00);

   return rc;
}
int32_t ov9760_sensor_otp_proc(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc=0;
	printk("ov9760_sensor_otp_proc start\n");

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
	rc=update_blc_ratio(s_ctrl);
	if(rc!=0)
		printk("no valid blc otp data rc=%d\n", rc);

	rc=update_otp_wb(s_ctrl);
	if(rc!=0)
		printk("no valid wb otp data rc=%d\n", rc);


	rc=update_otp_lenc(s_ctrl);
	if(rc!=0)
		printk("no valid len otp data rc=%d\n", rc);
	return rc;

}

static int32_t msm_ov9760_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &ov9760_s_ctrl);
}

static const struct i2c_device_id ov9760_i2c_id[] = {
	{OV9760_SENSOR_NAME, (kernel_ulong_t)&ov9760_s_ctrl},
	{ }
};

static struct i2c_driver ov9760_i2c_driver = {
	.id_table = ov9760_i2c_id,
	.probe  = msm_ov9760_i2c_probe,
	.driver = {
		.name = OV9760_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov9760_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static struct msm_sensor_ctrl_t ov9760_s_ctrl = {
	.sensor_i2c_client = &ov9760_sensor_i2c_client,
	.power_setting_array.power_setting = ov9760_power_setting,
	.power_setting_array.size = ARRAY_SIZE(ov9760_power_setting),
	.msm_sensor_mutex = &ov9760_mut,
	.sensor_v4l2_subdev_info = ov9760_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov9760_subdev_info),
	.sensor_read_mid = ov9760_sensor_read_mid,
	.sensor_otp_proc = ov9760_sensor_otp_proc,
	.sensor_get_otp = ov9760_sensor_get_otp,
};

static const struct of_device_id ov9760_dt_match[] = {
	{.compatible = "qcom,ov9760", .data = &ov9760_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, ov9760_dt_match);

static struct platform_driver ov9760_platform_driver = {
	.driver = {
		.name = "qcom,ov9760",
		.owner = THIS_MODULE,
		.of_match_table = ov9760_dt_match,
	},
};

static int32_t ov9760_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;

	match = of_match_device(ov9760_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}

static int __init ov9760_init_module(void)
{
	int32_t rc = 0;
	pr_err("%s:%d\n", __func__, __LINE__);

	rc = platform_driver_probe(&ov9760_platform_driver,
		ov9760_platform_probe);
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	if (!rc)
		return rc;
	return i2c_add_driver(&ov9760_i2c_driver);
}

static void __exit ov9760_exit_module(void)
{
	pr_err("%s:%d\n", __func__, __LINE__);
	if (ov9760_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&ov9760_s_ctrl);
		platform_driver_unregister(&ov9760_platform_driver);
	} else
		i2c_del_driver(&ov9760_i2c_driver);
	return;
}

module_init(ov9760_init_module);
module_exit(ov9760_exit_module);
MODULE_DESCRIPTION("ov9760");
MODULE_LICENSE("GPL v2");
