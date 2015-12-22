/*
 *
 * ISL98607 bias IC  driver.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include "isl98607_bias.h"
#include <linux/regulator/consumer.h>
#include <linux/debugfs.h>

#define NO_NODE -2

struct lcd_bias_data {
	struct i2c_client *client;
	int enp_gpio;
	int enn_gpio;
	const char *name;
	struct regulator *vdd;
	struct regulator *vcc_i2c;
	u16 addr;
};

struct lcd_bias_data isl98607_bias_data;

#if 0
static int isl98607_i2c_read(struct i2c_client *client, char *writebuf,
			   int writelen, char *readbuf, int readlen)
{
	int ret;

	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
				 .addr = client->addr,
				 .flags = 0,
				 .len = writelen,
				 .buf = writebuf,
			 },
			{
				 .addr = client->addr,
				 .flags = I2C_M_RD,
				 .len = readlen,
				 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&client->dev, "%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
				 .addr = client->addr,
				 .flags = I2C_M_RD,
				 .len = readlen,
				 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&client->dev, "%s:i2c read error.\n", __func__);
	}
	return ret;
}

static int isl98607_i2c_write(struct i2c_client *client, char *writebuf,
			    int writelen)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
			 .addr = client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
		 },
	};
	ret = i2c_transfer(client->adapter, msgs, 1);
	if (ret < 0){
		printk("[LCD]:%s: error = %d\n",__func__,ret);
		dev_err(&client->dev, "%s: i2c write error.\n", __func__);
		}

	return ret;
}

static int isl98607_write_reg(struct i2c_client *client, u8 addr, const u8 val)
{
	u8 buf[2] = {0};

	buf[0] = addr;
	buf[1] = val;

	return isl98607_i2c_write(client, buf, sizeof(buf));
}

static int isl98607_read_reg(struct i2c_client *client, u8 addr, u8 *val)
{
	return isl98607_i2c_read(client, &addr, 1, val, 1);
}
#endif

static int isl98607_parse_dt(struct device *dev,struct lcd_bias_data *bias_data)
{
	int rc;
	struct device_node *np = dev->of_node;
	//struct property *prop;

	bias_data->name = "isl98607";
	rc = of_property_read_string(np,"bias,name",&bias_data->name);

	bias_data->enp_gpio = of_get_named_gpio(np, "isl98607,enp-gpio",0);
	if (!gpio_is_valid(bias_data->enp_gpio)) {
		printk("[LCD] %s:%d, enp gpio not specified\n",
						__func__, __LINE__);
	} else {
		rc = gpio_request(bias_data->enp_gpio, "disp_enp_n");
		if (rc) {
			printk("[LCD] request enp gpio failed, rc=%d\n",
				rc);
			gpio_free(bias_data->enp_gpio);
			return -ENODEV;
		}
	}

	bias_data->enn_gpio = of_get_named_gpio(np, "isl98607,enn-gpio",0);
	if (!gpio_is_valid(bias_data->enn_gpio)) {
		printk("[LCD] %s:%d, enn gpio not specified\n",
						__func__, __LINE__);
	} else {
		rc = gpio_request(bias_data->enn_gpio, "disp_enn_n");
		if (rc) {
			printk("[LCD] request enn gpio failed, rc=%d\n",
				rc);
			gpio_free(bias_data->enn_gpio);
			gpio_free(bias_data->enp_gpio);
			return -ENODEV;
		}
	}
	pr_info("[LCD]:%s: bias name : %s, enp_gpio=%d, enn_gpio=%d\n",__func__,bias_data->name,bias_data->enp_gpio,bias_data->enn_gpio);

	return 0;
}

static int isl98607_bias_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int rc;
	struct device_node *isl98607_node;
	//u8 value;

	if(client->dev.of_node)
		isl98607_node = client->dev.of_node;
	else{
		printk("[LCD]:%s:Can't find the isl98607 device node\n",__func__);
		return NO_NODE;
	}
	
	isl98607_bias_data.vcc_i2c = regulator_get(&client->dev, "vcc_i2c");
	if (IS_ERR(isl98607_bias_data.vcc_i2c)) {
		rc = PTR_ERR(isl98607_bias_data.vcc_i2c);
		dev_err(&client->dev,
			"Regulator get failed vcc_i2c rc=%d\n", rc);
		}
	rc = regulator_enable(isl98607_bias_data.vcc_i2c);
	
	isl98607_bias_data.client = client;

	printk("[LCD]:%s: client->addr : 0x%x\n",__func__,client->addr);
	rc = isl98607_parse_dt(&client->dev, &isl98607_bias_data);
	if(rc == 0){
		printk("[LCD]:%s: successfully probe\n",__func__);
		}
	else{
		printk("[LCD]:%s: Probe failed\n",__func__);
		return -1;
	}
	return 0;
}

int isl98607_set_mode(enum bias_mode isl98607_mode)
{
	//u8 value;
	//isl98607_read_reg(isl98607_bias_data.client, 0x06, &value);
	switch(isl98607_mode){
		case BIAS_MODE_5P5V:
			//isl98607_write_reg(isl98607_bias_data.client,0x06,0x0A);
			//isl98607_write_reg(isl98607_bias_data.client,0x09,0x0A);
			//isl98607_write_reg(isl98607_bias_data.client,0x08,0x0A);
			gpio_direction_output(isl98607_bias_data.enp_gpio, 1);
			usleep(200);
			gpio_direction_output(isl98607_bias_data.enn_gpio,1);
			break;
		case BIAS_MODE_5V:
			break;
		case BIAS_MODE_SLEEP:
			gpio_direction_output(isl98607_bias_data.enp_gpio, 0);
			gpio_direction_output(isl98607_bias_data.enn_gpio,0);
			break;
		default:
			//isl98607_write_reg(isl98607_bias_data.client,0x06,0x0A);
			//isl98607_write_reg(isl98607_bias_data.client,0x09,0x0A);
			//isl98607_write_reg(isl98607_bias_data.client,0x08,0x0A);
			gpio_direction_output(isl98607_bias_data.enp_gpio, 1);
			usleep(200);
			gpio_direction_output(isl98607_bias_data.enn_gpio,1);
		}
	return 0;
}
			
static int isl98607_bias_remove(struct i2c_client *client){
	//TODO
	return 0;
}
static const struct i2c_device_id isl98607_bias_id[] = {
	{"isl98607_bias", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, isl98607_bias_id);

static struct of_device_id isl98607_match_table[] = {
	{.compatible = "bias,isl98607",},
	{},
};

static struct i2c_driver isl98607_bias_driver = {
	.probe = isl98607_bias_probe,
	.remove = isl98607_bias_remove,
	.driver = {
		.name = "isl98607_bias",
		.owner = THIS_MODULE,
		.of_match_table = isl98607_match_table,
	},
	.id_table = isl98607_bias_id,
};

static int __init isl98607_bias_init(void)
{
	return i2c_add_driver(&isl98607_bias_driver);
}
module_init(isl98607_bias_init);

static void __exit isl98607_bias_exit(void)
{
	i2c_del_driver(&isl98607_bias_driver);
}
module_exit(isl98607_bias_exit);

