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
#define OV8865_SENSOR_NAME "ov8865"
#define OV8865_OTP_FEATURE
DEFINE_MSM_MUTEX(ov8865_mut);

#define CONFIG_MSMB_CAMERA_DEBUG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

static struct msm_sensor_ctrl_t ov8865_s_ctrl;

static struct msm_sensor_power_setting ov8865_power_setting[] = {
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

#ifdef OV8865_OTP_FEATURE

// R/G and B/G of typical camera module is defined here
int ov8865_RG_Ratio_Typical = 0x142;
int ov8865_BG_Ratio_Typical = 0x11d;
static int ov8865_R_gain, ov8865_B_gain, ov8865_G_gain;

struct otp_struct_ov8865 ov8865_current_otp;

uint16_t ov8865_read_i2c(struct msm_sensor_ctrl_t *s_ctrl,uint32_t address)
{
	int rc =0;
	uint16_t ret_val =0;
	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
			s_ctrl->sensor_i2c_client,
			address, &ret_val,MSM_CAMERA_I2C_BYTE_DATA);
		//printk("read otp register address[0x%x] dataval=0x%x, rc=%d\n", address,ret_val, rc);
	if(rc<0)
		printk("read otp register address[0x%x] fail, rc=%d\n", address, rc);
	return ret_val;


}
int ov8865_write_i2c(struct msm_sensor_ctrl_t *s_ctrl,int address, int val)
{
	int ret = 0;
	ret = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
			s_ctrl->sensor_i2c_client, address, val, MSM_CAMERA_I2C_BYTE_DATA);
	return ret;
}

// index: index of otp group. (1, 2, 3)
// return: 0, group index is empty
// 1, group index has invalid data
// 2, group index has valid data
int ov8865_check_otp_wb(struct msm_sensor_ctrl_t *s_ctrl,int index)
{
    int flag;
   ov8865_write_i2c(s_ctrl,0x3d84, 0xC0);
    //partial mode OTP write start address
   ov8865_write_i2c(s_ctrl,0x3d88, 0x70);
   ov8865_write_i2c(s_ctrl,0x3d89, 0x20);
    // partial mode OTP write end address
   ov8865_write_i2c(s_ctrl,0x3d8A, 0x70);
   ov8865_write_i2c(s_ctrl,0x3d8B, 0x20);
    // read otp into buffer
   ov8865_write_i2c(s_ctrl,0x3d81, 0x01);
    usleep(5000);
    //select group
    flag = ov8865_read_i2c(s_ctrl,0x7020);
    if (index == 1)
    {
       flag = (flag>>6) & 0x03;
    }
    else if (index == 2)
    {
       flag = (flag>>4) & 0x03;
    }
    else if (index == 3)
    {
       flag = (flag>>2) & 0x03;
    }
    // clear otp buffer
    ov8865_write_i2c(s_ctrl, 0x7020, 0x00);
    if (flag == 0x00) {
                return 0;
    }
    else if (flag & 0x02) {
                return 1;
    }
    else {
                return 2;
    }

}

// index: index of otp group. (1, 2, 3)
// return: 0, group index is empty
// 1, group index has invalid data
// 2, group index has valid data
int ov8865_check_otp_lenc(struct msm_sensor_ctrl_t *s_ctrl,int index)
{
    int flag;
   ov8865_write_i2c(s_ctrl,0x3d84, 0xC0);
    //partial mode OTP write start address
   ov8865_write_i2c(s_ctrl,0x3d88, 0x70);
   ov8865_write_i2c(s_ctrl,0x3d89, 0x3A);
    // partial mode OTP write end address
   ov8865_write_i2c(s_ctrl,0x3d8A, 0x70);
   ov8865_write_i2c(s_ctrl,0x3d8B, 0x3A);
    // read otp into buffer
   ov8865_write_i2c(s_ctrl,0x3d81, 0x01);
    usleep(5000);
    flag = ov8865_read_i2c(s_ctrl,0x703a);
    if (index == 1)
    {
       flag = (flag>>6) & 0x03;
    }
    else if (index == 2)
    {
       flag = (flag>>4) & 0x03;
    }
    else if (index == 3)
    {
       flag = (flag>> 2)& 0x03;
    }
    // clear otp buffer
  ov8865_write_i2c(s_ctrl, 0x703a, 0x00);
    if (flag == 0x00) {
                return 0;
    }
    else if (flag & 0x02) {
                return 1;
    }
    else {
                return 2;
    }

}

// index: index of otp group. (1, 2, 3)
// otp_ptr: pointer of otp_struct_ov8865
// return: 0,
int ov8865_read_otp_wb(struct msm_sensor_ctrl_t *s_ctrl,int index, struct otp_struct_ov8865 *otp_ptr)
{
    int i;
    int temp;
    int start_addr, end_addr;
    int loop=0;
    CDBG("%s: index=%d\n",__func__,index);

	do{
        if (index == 1) {
                    start_addr = 0x7021;
                    end_addr = 0x7025;
        }
        else if (index == 2) {
                    start_addr = 0x7026;
                    end_addr = 0x702a;
        }
        else if (index == 3) {
                    start_addr = 0x702b;
                    end_addr = 0x702f;
        }
       ov8865_write_i2c(s_ctrl,0x3d84, 0xC0);
        //partial mode OTP write start address
       ov8865_write_i2c(s_ctrl,0x3d88, (start_addr >> 8) & 0xff);
       ov8865_write_i2c(s_ctrl,0x3d89, start_addr & 0xff);
        // partial mode OTP write end address
       ov8865_write_i2c(s_ctrl,0x3d8A, (end_addr >> 8) & 0xff);
       ov8865_write_i2c(s_ctrl,0x3d8B, end_addr & 0xff);
        // read otp into buffer
       ov8865_write_i2c(s_ctrl,0x3d81, 0x01);
        usleep(5000);
        temp = ov8865_read_i2c(s_ctrl,start_addr + 4);
        (*otp_ptr).rg_ratio = (ov8865_read_i2c(s_ctrl,start_addr)<<2) + ((temp>>6) & 0x03);
        (*otp_ptr).bg_ratio = (ov8865_read_i2c(s_ctrl,start_addr + 1)<<2) + ((temp>>4) & 0x03);
        (*otp_ptr).light_rg = (ov8865_read_i2c(s_ctrl,start_addr + 2) <<2) + ((temp>>2) & 0x03);
        (*otp_ptr).light_bg = (ov8865_read_i2c(s_ctrl,start_addr + 3)<<2) + (temp & 0x03);
        (*otp_ptr).user_data[0] = 4;//for vcm 657 all the time for ov8865
        CDBG("%s: (*otp_ptr).rg_ratio=0x%x\n",__func__, (*otp_ptr).rg_ratio);
        CDBG("%s: (*otp_ptr).bg_ratio=0x%x\n",__func__, (*otp_ptr).bg_ratio);
        CDBG("%s: (*otp_ptr).light_rg=0x%x\n",__func__, (*otp_ptr).light_rg);
        CDBG("%s: (*otp_ptr).light_bg=0x%x\n",__func__, (*otp_ptr).light_bg);
        CDBG("%s: (*otp_ptr).user_data[0]=%d\n",__func__, (*otp_ptr).user_data[0]);
        CDBG("%s: golden.rg_ratio=0x%x\n",__func__, ov8865_RG_Ratio_Typical);
        CDBG("%s: golden.rg_ratio=0x%x\n",__func__, ov8865_BG_Ratio_Typical);

        // clear otp buffer
        for (i=start_addr; i<=end_addr; i++) {
                  ov8865_write_i2c(s_ctrl,i, 0x00);
        }

        if (otp_ptr->rg_ratio != 0)
        {
           (* otp_ptr).checksum_wb_ok=1;
           CDBG("%s: otp wb data check sum ok read success! checksum_wb_ok=%d userdata[0]=%d\n",__func__, (* otp_ptr).checksum_wb_ok,(*otp_ptr).user_data[0]);
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

// index: index of otp group. (1, 2, 3)
// otp_ptr: pointer of otp_struct_ov8865
// return: 0,
int ov8865_read_otp_lenc(struct msm_sensor_ctrl_t *s_ctrl,int index, struct otp_struct_ov8865 *otp_ptr)
{
    int i;
    int start_addr, end_addr;
    int loop=0;
    CDBG("%s: index=%d\n",__func__,index);

	// select bank: index*2+1
    do{
        if (index == 1) {
                    start_addr = 0x703b;
                    end_addr = 0x7078;
        }
        else if (index == 2) {
                    start_addr = 0x7079;
                    end_addr = 0x70b6;
        }
        else if (index == 3) {
                    start_addr = 0x70b7;
                    end_addr = 0x70f4;
        }
       ov8865_write_i2c(s_ctrl,0x3d84, 0xC0);
        //partial mode OTP write start address
       ov8865_write_i2c(s_ctrl,0x3d88, (start_addr >> 8) & 0xff);
       ov8865_write_i2c(s_ctrl,0x3d89, start_addr & 0xff);
        // partial mode OTP write end address
       ov8865_write_i2c(s_ctrl,0x3d8A, (end_addr >> 8) & 0xff);
       ov8865_write_i2c(s_ctrl,0x3d8B, end_addr & 0xff);
        // read otp into buffer
       ov8865_write_i2c(s_ctrl,0x3d81, 0x01);
        usleep(10000);
        for(i=0; i<62; i++) {
           (* otp_ptr).lenc[i]=ov8865_read_i2c(s_ctrl,(start_addr + i));
           CDBG("%s: (* otp_ptr).lenc[%d]=0x%x\n",__func__,i, (* otp_ptr).lenc[i]);

        }
        // clear otp buffer
        for (i=start_addr; i<=end_addr; i++) {
                   ov8865_write_i2c(s_ctrl,i, 0x00);
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
int ov8865_update_awb_gain(struct msm_sensor_ctrl_t *s_ctrl,int R_gain, int G_gain, int B_gain,struct otp_struct_ov8865 * otp_ptr)
{
    if((*otp_ptr).checksum_wb_ok==1)
    {

        if (R_gain>0x400) {
                 ov8865_write_i2c(s_ctrl,0x5018, R_gain>>6);
                 ov8865_write_i2c(s_ctrl,0x5019, R_gain & 0x003f);
        }
        if (G_gain>0x400) {
                 ov8865_write_i2c(s_ctrl,0x501A, G_gain>>6);
                 ov8865_write_i2c(s_ctrl,0x501B, G_gain & 0x003f);
        }
        if (B_gain>0x400) {
                 ov8865_write_i2c(s_ctrl,0x501C, B_gain>>6);
                 ov8865_write_i2c(s_ctrl,0x501D, B_gain & 0x003f);
        }
    }
    else
    {
    		CDBG("%s checksum_wb_ok==0 can't set wb otp",__func__);
    }
    return 0;

}

// otp_ptr: pointer of otp_struct_ov8865
int ov8865_update_lenc(struct msm_sensor_ctrl_t *s_ctrl,struct otp_struct_ov8865 * otp_ptr)
{
    int i, temp;
	if ((*otp_ptr).checksum_lsc_ok == 1)
	{

        temp = ov8865_read_i2c(s_ctrl,0x5000);
        temp = 0x80 | temp;
       ov8865_write_i2c(s_ctrl,0x5000, temp);
        for(i=0;i<62;i++) {
                  ov8865_write_i2c(s_ctrl,0x5800 + i, (*otp_ptr).lenc[i]);
        }
    }
    else
    {
    		CDBG("%s checksum_wb_ok==0 can't set lenc otp",__func__);
    }
    return 0;

}

// call this function after OV8865 initialization
// return value: 0 update success
// 1, no OTP
//int ov8865_update_otp_wb(struct msm_sensor_ctrl_t *s_ctrl)

int ov8865_store_otp_wb(struct msm_sensor_ctrl_t *s_ctrl)
{
    int i;
    int otp_index;
    int temp;
    int rg,bg;
    //calculate G gain
    int nR_G_gain, nB_G_gain, nG_G_gain;
    int nBase_gain;
    // R/G and B/G of current camera module is read out from sensor OTP
    // check first OTP with valid data
    if(ov8865_current_otp.checksum_wb_ok == 0)
    {
        for(i=1;i<=3;i++) {
                   temp = ov8865_check_otp_wb(s_ctrl,i);
                   if (temp == 2) {
                              otp_index = i;
                              break;
                   }
        }
        if (i>3) {
                   // no valid wb OTP data
                   return 1;
        }
        ov8865_read_otp_wb(s_ctrl,otp_index, &ov8865_current_otp);
        if(ov8865_current_otp.light_rg==0) {
                   // no light source information in OTP, light factor = 1
                   rg = ov8865_current_otp.rg_ratio;
        }
        else {
                   rg = ov8865_current_otp.rg_ratio * (ov8865_current_otp.light_rg +512) / 1024;
        }
        if(ov8865_current_otp.light_bg==0) {
                   // not light source information in OTP, light factor = 1
                   bg = ov8865_current_otp.bg_ratio;
        }
        else {
                   bg = ov8865_current_otp.bg_ratio * (ov8865_current_otp.light_bg +512) / 1024;
        }

        nR_G_gain = (ov8865_RG_Ratio_Typical*1000) / rg;
        nB_G_gain = (ov8865_BG_Ratio_Typical*1000) / bg;
        nG_G_gain = 1000;
        if (nR_G_gain < 1000 || nB_G_gain < 1000)
        {
           if (nR_G_gain < nB_G_gain)
              nBase_gain = nR_G_gain;
          else
              nBase_gain = nB_G_gain;
        }
        else
        {
             nBase_gain = nG_G_gain;
        }
        ov8865_R_gain = 0x400 * nR_G_gain / (nBase_gain);
        ov8865_B_gain = 0x400 * nB_G_gain / (nBase_gain);
        ov8865_G_gain = 0x400 * nG_G_gain / (nBase_gain);
       // ov8865_update_awb_gain(s_ctrl,ov8865_R_gain, ov8865_G_gain, ov8865_B_gain);
   }
    return 0;

}

// call this function after OV8865 initialization
// return value: 0 update success
// 1, no OTP
//int ov8865_update_otp_lenc(struct msm_sensor_ctrl_t *s_ctrl)
int ov8865_store_otp_lenc(struct msm_sensor_ctrl_t *s_ctrl)

{
    int i;
    int otp_index;
    int temp;
    if(ov8865_current_otp.checksum_lsc_ok == 0)
    {
        // check first lens correction OTP with valid data
        for(i=1;i<=3;i++) {
                   temp = ov8865_check_otp_lenc(s_ctrl,i);
                   if (temp == 2) {
                              otp_index = i;
                              break;
                   }
        }
        if (i>3) {
                   // no valid WB OTP data
                   return 1;
        }
        ov8865_read_otp_lenc(s_ctrl,otp_index, &ov8865_current_otp);
        //ov8865_update_lenc(s_ctrl,&ov8865_current_otp);
        // success
    }
    return 0;

}

int32_t ov8865_sensor_read_mid(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc=-1;

// start stream
	ov8865_write_i2c(s_ctrl, 0x0100, 0x01);

    msleep(5);

	rc=ov8865_store_otp_wb(s_ctrl);
	if(rc!=0)
		printk("no valid wb otp wb data rc=%d\n", rc);


	rc=ov8865_store_otp_lenc(s_ctrl);
	if(rc!=0)
		printk("no valid wb otp lenc data rc=%d\n", rc);

//stop stream
	ov8865_write_i2c(s_ctrl, 0x0100, 0x00);

   return rc;
}


int32_t ov8865_sensor_otp_proc(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc=0;

		printk("ov8865_sensor_otp_proc start\n");

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
        rc=ov8865_store_otp_wb(s_ctrl);
        rc=ov8865_store_otp_wb(s_ctrl);
		rc=ov8865_update_lenc(s_ctrl,&ov8865_current_otp);
		if(rc!=0)
			printk("no valid wb otp lenc data rc=%d\n", rc);

		rc= ov8865_update_awb_gain(s_ctrl,ov8865_R_gain, ov8865_G_gain, ov8865_B_gain,&ov8865_current_otp);
		if(rc!=0)
			printk("no valid wb otp wb data rc=%d\n", rc);

		/*set flag to 0, not updating otp only once*/

	return rc;

}

int32_t ov8865_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc = 0;
    rc = msm_sensor_power_up(s_ctrl);
    return rc;
}

int32_t ov8865_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc = 0;
    rc = msm_sensor_power_down(s_ctrl);
    return rc;
}

#endif

static struct v4l2_subdev_info ov8865_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};

static const struct i2c_device_id ov8865_i2c_id[] = {
	{OV8865_SENSOR_NAME, (kernel_ulong_t)&ov8865_s_ctrl},
	{ }
};

static int32_t msm_ov8865_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &ov8865_s_ctrl);
}

static struct i2c_driver ov8865_i2c_driver = {
	.id_table = ov8865_i2c_id,
	.probe  = msm_ov8865_i2c_probe,
	.driver = {
		.name = OV8865_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov8865_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static const struct of_device_id ov8865_dt_match[] = {
	{.compatible = "qcom,ov8865", .data = &ov8865_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, ov8865_dt_match);

static struct platform_driver ov8865_platform_driver = {
	.driver = {
		.name = "qcom,ov8865",
		.owner = THIS_MODULE,
		.of_match_table = ov8865_dt_match,
	},
};

static int32_t ov8865_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;
	match = of_match_device(ov8865_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}

static int __init ov8865_init_module(void)
{
	int32_t rc = 0;
	pr_info("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_probe(&ov8865_platform_driver,
		ov8865_platform_probe);
	if (!rc)
		return rc;
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&ov8865_i2c_driver);
}

static void __exit ov8865_exit_module(void)
{
	pr_info("%s:%d\n", __func__, __LINE__);
	if (ov8865_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&ov8865_s_ctrl);
		platform_driver_unregister(&ov8865_platform_driver);
	} else
		i2c_del_driver(&ov8865_i2c_driver);
	return;
}

static struct msm_sensor_fn_t ov8865_sensor_func_tbl = {
	.sensor_config = msm_sensor_config,
	.sensor_power_up = ov8865_power_up,
	.sensor_power_down = ov8865_power_down,
	.sensor_match_id = msm_sensor_match_id,
};

static struct msm_sensor_ctrl_t ov8865_s_ctrl = {
	.sensor_i2c_client = &ov8865_sensor_i2c_client,
	.power_setting_array.power_setting = ov8865_power_setting,
	.power_setting_array.size = ARRAY_SIZE(ov8865_power_setting),
	.msm_sensor_mutex = &ov8865_mut,
	.sensor_v4l2_subdev_info = ov8865_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov8865_subdev_info),
#ifdef OV8865_OTP_FEATURE
	.sensor_read_mid = ov8865_sensor_read_mid,
	.sensor_otp_proc = ov8865_sensor_otp_proc,
	.func_tbl = &ov8865_sensor_func_tbl,
#endif
};

module_init(ov8865_init_module);
module_exit(ov8865_exit_module);
MODULE_DESCRIPTION("ov8865");
MODULE_LICENSE("GPL v2");
