/* Copyright (c) 2014-2016, Lenovo.
 * Created by zhanggb, 20140228
 */
 
#include "lenovo_mdss_utils.h"

#include <linux/of.h>
#include <linux/delay.h>
#include <linux/unistd.h>
#include <linux/fb.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/err.h>
#include <asm/uaccess.h>
#include <asm/system.h>

#include "mdss_dsi.h"	

static char g_cabc = 0x03;
static char g_ce = 0x80;
static char g_dimming = 0x2c; //bit3
static char *mybuf = NULL;
static int rawdataindex = 0;
static int get_data(char *buf)
{
	char a, b;

	if (buf[0] >= 'A' && buf[0] <= 'F') {
		a = buf[0] - 55;
	} else if (buf[0] >= 'a' && buf[0] <= 'f') {
		a = buf[0] - 87;
	} else {
		a = buf[0] - 48;
	}
	if (buf[1] >= 'A' && buf[1] <= 'F') {
		b = buf[1] - 55;
	} else if (buf[1] >= 'a' && buf[1] <= 'f') {
		b = buf[1] - 87;
	} else {
		b = buf[1] - 48;
	}
	return a * 16 + b;
}

static int next_data_ptr(char * srcptr)//get a line of data
{
	char * retp = srcptr;
	int finddata = 0;
	//char tem[256];
	//char littletem[16];
	int iscomments = 0;
	int cnt = 0;

	//memset(tem, 0, 256);
	//memset(littletem, 0, 16);
	if(!retp)
		return finddata;
	while(*retp != '\0' && *retp != '\n' && !(retp[0]==0x0d && retp[1]=='\n')) //null, or return; or dos return
	{
		if(*retp ==  '/' || *retp ==  '[') //'/'
		{
			if(iscomments==0)
				pr_info("%s: find comments\n", __func__);
			iscomments = 1;
		}
		if((*retp !=  ' ') && (*retp !=  0x09/*tab*/) && (iscomments == 0))
		{
			//memset(littletem, 0, 16);
			//memcpy(littletem, retp, 2);
			//littletem[2] = '\0';
			mybuf[rawdataindex++] = get_data(retp);
			//printk("%s%02x@", littletem,mybuf[rawdataindex-1]);
			retp += 2;
			finddata = 1;
			cnt += 2;
		}else
		{
			retp++;
			cnt++;
		}
	}
	//memcpy(tem, srcptr, cnt);
	//tem[cnt] = '\0';
	//pr_info("@ %s: \n", tem);
	return finddata;
}

#define LCD_PARA_BUFSIZE 1024*100
int mdss_lcd_parse_file_cmds(struct dsi_panel_cmds *pcmds)  //now only support ASIIC code
{
	int cnt =0;
	struct file *myfile;
	int i = 0, j=0;
	ssize_t vfs_read_retval = 0;
	char *bp, *bptem;
	loff_t pos;
	loff_t offset = (unsigned long)0;
	mm_segment_t old_fs;
	const char *filename = "/storage/sdcard0/lcd.txt";
	const char *filename_bak = "/data/lcd.txt";
	char temdata[256];

	//for sys control interface, need sync with dtsi
	g_cabc = 0x01;
	g_ce = 0x80;
	g_dimming = 0x2c;
	
	//pr_info("%s: ######\n", __func__);
	if(mybuf)
	{
		kfree(mybuf);
	}
	rawdataindex = 0;
	myfile = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(myfile)) {
		const int open_errno = -PTR_ERR(myfile);
		pr_info("%s(): cant find file (%s), errno = %d!, try (%s)\n", __func__, filename, open_errno, filename_bak);
		myfile = filp_open(filename_bak, O_RDONLY, 0);
		if(IS_ERR(myfile))
		{
			pr_info("%s(): cant find file (%s), errno = %d!\n", __func__, filename_bak, open_errno);
			return PTR_ERR(myfile);
		}else
			pr_info("%s(): find (%s)\n", __func__, filename_bak);
	}else
		pr_info("%s(): find (%s)\n", __func__, filename);
	
	//begin read and process
	{
		if (!(myfile->f_op)) {
			pr_err("%s(): File has no file operations registered!\n", __func__);
			filp_close(myfile, NULL);
			return -EIO;
		}

		if (!myfile->f_op->read) {
			pr_err("%s(): File has no READ operations registered!\n", __func__);
			filp_close(myfile, NULL);
			return -EIO;
		}

		pos = myfile->f_pos;
		old_fs = get_fs();
		set_fs(KERNEL_DS);

		vfs_read_retval = generic_file_llseek(myfile, 0, SEEK_END);
		pr_info("%s: ###### file size %d\n", __func__, vfs_read_retval);
		
		pos = offset;
		if (vfs_read_retval > 0 ) {
			mybuf = kmalloc(sizeof(char) * (vfs_read_retval+8), GFP_KERNEL);  //make sure here is memory for '\0' to end the parsing
			if(!mybuf)
				goto exit_free;
			memset(mybuf, 0, vfs_read_retval+8);
			vfs_read_retval = vfs_read(myfile, mybuf, vfs_read_retval, &pos);
		}

		set_fs(old_fs);
		myfile->f_pos = 0;
		filp_close(myfile, NULL);

		/* scan dcs commands */
		bp = mybuf;
		cnt = 0;
		bptem = strstr(bp,"\n");
		if(!bptem)
			goto exit_free;
		memset(temdata,0,256);
		memcpy(temdata, bp, bptem-bp);
		//pr_info("#%s\n", temdata);
		if(*temdata != '[' && *temdata != '/')
		{
			if(next_data_ptr(bp))
				cnt++;
		}
		bp = bptem + 1;
		if(!bp)
			goto exit_free;
		while (*bp) {
			bptem = strstr(bp,"\n");
			if(!bptem)
				break;
			memset(temdata,0,256);
			memcpy(temdata, bp, bptem-bp);
			if(*temdata != '[' && *temdata != '/')
			{
				if(next_data_ptr(bp))
					cnt++;
			}
			bp = bptem + 1;
			//pr_info("#%s: cnt=%d\n", temdata, cnt);
			if(!bp)
				break;
			if((bp-mybuf) >= vfs_read_retval)
			{
				pr_info("#%s: file end\n", __func__);
				break;
			}
		}
		kfree(pcmds->cmds);
		pcmds->cmds = kzalloc(cnt * sizeof(struct dsi_cmd_desc), GFP_KERNEL);
		pcmds->cmd_cnt = cnt;
		pcmds->buf = mybuf;
		pcmds->blen = vfs_read_retval;

		bp = mybuf;
		for (i = 0; i < cnt; i++) {
			pcmds->cmds[i].dchdr.dtype = bp[0];
			pcmds->cmds[i].dchdr.last = bp[1];
			pcmds->cmds[i].dchdr.vc = bp[2];
			pcmds->cmds[i].dchdr.ack = bp[3];
			pcmds->cmds[i].dchdr.wait= bp[4];
			pcmds->cmds[i].dchdr.dlen= bp[5]*256 + bp[6];
			pcmds->cmds[i].payload = bp + 7;
			printk("mdss: ");
			for(j = 0; j < (pcmds->cmds[i].dchdr.dlen + 7); j++)
			{
				printk("%02x ", bp[j]);
			}
			printk("## cnt=%d\n", i+1);
			bp += (pcmds->cmds[i].dchdr.dlen + 7);
		}
	}

	pr_info("%s: dcs_cmd=%x len=%d, cmd_cnt=%d link_state=%d\n", __func__,
		pcmds->buf[0], pcmds->blen, pcmds->cmd_cnt, pcmds->link_state);
	return 0;

exit_free:
	set_fs(old_fs);
	myfile->f_pos = 0;
	filp_close(myfile, NULL);
	return -ENOMEM;
}

//==sys interface====================================================
extern struct mdss_panel_data *pdata_esd;
static char cmd_ctrl_onoff[2] = {0x55, 0x0};	/* DTYPE_DCS_WRITE1 */
static struct dsi_cmd_desc dcs_cabc_onoff_cmd = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(cmd_ctrl_onoff)},
	cmd_ctrl_onoff
};
int mdss_dsi_get_cabc_state(void)
{
	return g_cabc;
}
void mdss_dsi_panel_cabc_onoff(int onoff)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct dcs_cmd_req cmdreq;
	char nowval = 0, setval = 0, temval = 0;

	temval = (char)onoff;
	if (pdata_esd == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}
	ctrl_pdata = container_of(pdata_esd, struct mdss_dsi_ctrl_pdata,panel_data);

	if (!(ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT))
	{
		pr_info("###%s: lcd already power off\n", __func__);
		return;
	}
	
	//mdss_dsi_panel_cmd_read(ctrl_pdata, 0x56, 0, NULL, &nowval, 1);
	nowval = g_cabc | g_ce;
	if(onoff >= 0 && onoff <= 3)
		setval = (nowval & 0xf0) | (temval & 0x0f);
	else
		setval = temval;
	pr_info("%s: 0x%02x, now=0x%02x, set=0x%02x\n", __func__, temval, nowval, setval);

	cmd_ctrl_onoff[0] = 0x55;
	cmd_ctrl_onoff[1] = setval;

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &dcs_cabc_onoff_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);

	g_cabc = setval & 0x0f;
}

int mdss_dsi_get_ce_state(void)
{
	return g_ce;
}
void mdss_dsi_panel_ce_onoff(int onoff)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct dcs_cmd_req cmdreq;
	char nowval = 0, setval = 0, temval = 0;

	temval = (char)onoff;
	if (pdata_esd == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}
	ctrl_pdata = container_of(pdata_esd, struct mdss_dsi_ctrl_pdata,panel_data);

	if (!(ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT))
	{
		pr_info("###%s: lcd already power off\n", __func__);
		return;
	}
	
	//mdss_dsi_panel_cmd_read(ctrl_pdata, 0x56, 0, NULL, &nowval, 1);
	nowval = g_cabc | g_ce;
	if(temval >= 0 && temval <= 3)
	{
		if(temval == 1)
			temval = 0x80;
		if(temval == 2)
			temval = 0x90;
		if(temval == 3)
			temval = 0xb0;
		setval = (nowval & 0x0f) | (temval & 0xf0);
	}
	else
		setval = temval;
	pr_info("%s: 0x%02x, now=0x%02x, set=0x%02x\n", __func__, temval, nowval, setval);

	cmd_ctrl_onoff[0] = 0x55;
	cmd_ctrl_onoff[1] = setval;

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &dcs_cabc_onoff_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
	
	g_ce = setval & 0xf0;
}

int mdss_dsi_get_dimming_state(void)
{
	return g_dimming;
}
void mdss_dsi_panel_dimming_onoff(int onoff)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct dcs_cmd_req cmdreq;
	char nowval = 0, setval = 0, temval = 0;

	temval = (char)onoff;
	if (pdata_esd == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}
	ctrl_pdata = container_of(pdata_esd, struct mdss_dsi_ctrl_pdata,panel_data);

	if (!(ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT))
	{
		pr_info("###%s: lcd already power off\n", __func__);
		return;
	}
	
	//mdss_dsi_panel_cmd_read(ctrl_pdata, 0x54, 0, NULL, &nowval, 1);
	nowval = g_dimming;
	if(temval == 0)
	{
		setval = 0x24;
	}
	else
		setval = 0x2c;
	pr_info("%s: 0x%02x, now=0x%02x, set=0x%02x\n", __func__, temval, nowval, setval);

	cmd_ctrl_onoff[0] = 0x53;
	cmd_ctrl_onoff[1] = setval;

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &dcs_cabc_onoff_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
	
	g_dimming = setval;
}

