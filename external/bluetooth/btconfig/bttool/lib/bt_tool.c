/*



About QCOM Vendor Specific HCI CMD.
qcom vendor cmd, EVENT: buf[0]=0xff, buf[1]=param_len, buf[2]...=params



*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <endian.h>
#include <byteswap.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <cutils/properties.h>

#ifdef ANDROID
#include <termios.h>
#else
#include <sys/termios.h>
#include <sys/ioctl.h>
#include <limits.h>
#endif

#include "bluetooth.h"
#include "hci.h"

#include "btconfig.h"
#include "bt_tool.h"


//#define BT_DEBUG

#ifdef BT_DEBUG

#define DBG(fmt, arg...) do {printf(fmt, ##arg); }while(0)

#else
#define DBG(fmt, arg...) do {}while(0)

#endif

#define ERR(fmt, arg...) do {fprintf(stderr, fmt, ##arg); }while(0)



//printf("%s:%s()" fmt, __FILE__, __func__, ##arg);

/*

	Define

*/


#define PRONTO_SOC TRUE


#define BT_DEV_WCN36X0			"/dev/smd3"
#define HCISMD_MOD_PARAM 		"/sys/module/hci_smd/parameters/hcismd_set"


#define ERR_DEV_NOT_OPEN		"Bluetooth is not enable.\n"
#define ERR_INVALID_PARAM		"Invalid parameter.\n"

#define CMD_EVENT_DELAY_US		(10*1000)		//10ms


#define DEFAULT_LOCAL_NAME		"Lenovo 8974"




//BT HCI CMD offset

/******		BT HCI EVENT offset		******/
//Command HCI command's return event
#define OFFSET_HCI_EVENT_STATUS		5

/******		BT HCI CMD ERROR CODE	******/
#define BT_CMD_STATUS_SUCCESS		0


int bt_fd = -1;

/*

	Global Variables

*/

fEvent_InquiryResult gcb_Event_InqiryResult = NULL;
fEvent_InquiryComlpete gcb_Event_InqiryComlpete = NULL;



/*

	Function define.

*/
static int bt_open(void);
static int bt_close(void);
static int bt_setup(void);
static int open_bt_device(void);
static int init_smd(char *dev);
static int set_hci_smd_transport(int on);
static int hci_send_cmd(int fd, uint16_t ogf, uint16_t ocf, uint8_t plen, unsigned char *param);
static int read_hci_event(int fd, unsigned char* buf, int size);
static int read_event_modified(int fd, unsigned char* buf, int to);
static int read_incoming_events(int fd, unsigned char* buf, int to);
static int writeHciCommand(int fd, uint16_t ogf, uint16_t ocf, uint8_t plen, unsigned char *buf);

static void LoadPSHeader(unsigned char *HCI_PS_Command,unsigned char opcode,int length,int index);

#define OPEN_BT()               \
do {                            \
	if (bt_open() != BT_OK)     \
		return BT_ERROR;        \
} while(0)

//Send HCI CMD, not read Event
#define SEND_CMD(ogf, ocf, len, buf)                                            \
do {                                                                            \
	if(hci_send_cmd(bt_fd, ogf, ocf, len, buf) == BT_ERROR) {                   \
		ERR( "Send HCI CMD (ogf=0x%02X, ocf=0x%04X) failed\n", ogf, ocf);       \
		return BT_ERROR;                                                        \
	}                                                                           \
} while(0)

//Send HCI CMD and read Event
#define SEND_CMD_EVENT(ogf, ocf, len, buf)                                              \
do {                                                                                    \
	if(writeHciCommand(bt_fd, ogf, ocf, len, buf) == BT_ERROR) {                        \
		ERR( "Send HCI CMD (ogf=0x%02X, ocf=0x%04X) failed\n", ogf, ocf);               \
		return BT_ERROR;                                                                \
	} else {                                                                            \
		if(buf[OFFSET_HCI_EVENT_STATUS] != BT_CMD_STATUS_SUCCESS){                      \
			ERR("Send HCI CMD (ogf=0x%02X, ocf=0x%04X) failed due to reason 0x%X\n",    \
				ogf, ocf, buf[OFFSET_HCI_EVENT_STATUS]);                                \
			return BT_ERROR;                                                            \
		}                                                                               \
	}                                                                                   \
} while(0)





/*

	Public Function.

*/

int bt_is_enabled(void)
{
	return (bt_fd != -1) ? 1 : 0;
}

/*
	Disable bluetooth.
	Parameters:
	Return:
		return BT_ERROR  when failed.
		return BT_OK when success.
*/
int bt_enable(void)
{

	if (bt_is_enabled()) {
		//bt has been enable before, do nothing.
		return BT_OK;
	}

	#ifdef BT_DEBUG
	system("/system/bin/hci_qcomm_init -vvv -e");
	#else
//	system("/system/bin/hci_qcomm_init -e >/dev/null 2>&1");
	system("/system/bin/hci_qcomm_init -e");
	#endif
	usleep(10*1000);

//	set_hci_smd_transport(1);
//	usleep(10*1000);

	if (bt_open() == BT_ERROR){
		ERR("%s : open bt failed\n", __func__);
		return BT_ERROR;
	}

	if (bt_setup() == BT_ERROR) {
		ERR("%s : setup bt failed\n", __func__);
		return BT_ERROR;
	}


	return BT_OK;
}


/*
	Disable bluetooth.
	Parameters:
	Return:
		return BT_ERROR  when failed.
		return BT_OK when success.
*/
int bt_disable(void)
{
	bt_reset();

	bt_close();
//	set_hci_smd_transport(0);
	return BT_OK;
}

/*
	Reset bluetooth local host.
	Parameters:
	Return:
		return BT_ERROR  when failed.
		return BT_OK when success.
*/
int bt_reset(void)
{
	OPEN_BT();

	int ret;
	unsigned char buf[MAX_EVENT_SIZE];

	memset(buf, 0, MAX_EVENT_SIZE);
	// OGF_HOST_CTL 0x03
	// OCF_RESET 0x0003
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_RESET, 0, buf);

	return BT_OK;
}

/* Local Host Info */
/*
	Read the bluetooth local host name.
	Parameters:
		name		Store bluetooth local host name.
		max_len		Length of name.  Suggest set to MAX_BT_NAME_LEN (248)
	Return:
		return BT_ERROR  when failed.
		return length of  bluetooth local host name when success.
*/
int bt_get_host_name(char *name, int max_len)
{
	if (name == NULL) {
		ERR( ERR_INVALID_PARAM);
		return BT_ERROR;
	}

	OPEN_BT();

	int len;
	unsigned char buf[MAX_EVENT_SIZE];

	memset(buf, 0, MAX_EVENT_SIZE);

	//0x03   		OGF_HOST_CTL
	//0x0014 	OCF_READ_LOCAL_NAME
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_READ_LOCAL_NAME, 0, buf);

	len = strlen(buf+6);
	if (len < max_len){
		strcpy(name, buf+6);
	} else {
		memcpy(name, buf+6, max_len);
		name[max_len-1] = '\0';
	}

	return strlen(name);

}

/*
	Write the bluetooth local host name.
	Parameters:
		name		Bluetooth local host name.
	Return:
		return BT_ERROR  when failed.
		return length of  bluetooth local host name when success.
*/
int bt_set_host_name(const char *name)
{
	if (name == NULL) {
		ERR( ERR_INVALID_PARAM);
		return BT_ERROR;
	}

	OPEN_BT();

	int len;
	unsigned char buf[MAX_EVENT_SIZE];

	memset(buf, 0, MAX_EVENT_SIZE);

	len = strlen(name);
	if (len < MAX_BT_NAME_LEN)
		strcpy((char *)buf, name);
	else {
		len = MAX_BT_NAME_LEN-1;
		memcpy((char *)buf, name, len);
	}

	//0x03		OGF_HOST_CTL
	//0x0013	OCF_CHANGE_LOCAL_NAME
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_CHANGE_LOCAL_NAME, MAX_BT_NAME_LEN, buf);

	return len;

}

/*
	Get Bluetooth local host information.
	Parameters:
		info			store bluetooth local host information.
	Return:
		return BT_ERROR  when failed.
		return BT_OK when success.
*/
int bt_get_host_info(tHost_Info *info)
{
	if (info == NULL) {
		ERR( ERR_INVALID_PARAM);
		return BT_ERROR;
	}

	OPEN_BT();

	int i, len, ret;
	unsigned char buf[MAX_EVENT_SIZE];


/********	Send HCI CMD : Read Local Version Information				********/
	memset(buf, 0, MAX_EVENT_SIZE);
	// OGF_INFO_PARAM 0x04
	// OCF_READ_LOCAL_VERSION 0x0001
	SEND_CMD_EVENT(OGF_INFO_PARAM, OCF_READ_LOCAL_VERSION, 0, buf);

	read_local_version_rp *local_version = (read_local_version_rp *)&buf[5];

	info->hci_ver = local_version->hci_ver;
	info->hci_rev = local_version->hci_rev;
	info->lmp_ver = local_version->lmp_ver;
	info->lmp_subver = local_version->lmp_subver;
	info->manufacturer = local_version->manufacturer;

	//Print local version
	if (buf[6] == 0x06)
		DBG("\tHCI Version		 = 0x%02X  [Bluetooth Core Specification 4.0]\n", buf[6]);
	else
		DBG("\tHCI Version		 = 0x%02X\n", buf[6]);
	DBG("\tHCI Revision 	 = 0x%02X%02X\n", buf[8], buf[7]);
	DBG("\tLMP Version		 = 0x%02X\n", buf[9]);
	if ((buf[11] == 0x00) && (buf[10] == 0x1D))
		DBG("\tManufacturer Name = 0x%02X%02X  [Qualcomm]\n", buf[11], buf[10]);
	else
		DBG("\tManufacturer Name = 0x%02X%02X\n", buf[11], buf[10]);
	DBG("\tLMP Subversion	 = 0x%02X%02X\n", buf[13], buf[12]);
	DBG("\n\n");


/********	Send HCI CMD : Read Local Supported Feature 					********/
	memset(buf, 0, MAX_EVENT_SIZE);
	// OGF_INFO_PARAM 0x04
	// OCF_READ_LOCAL_FEATURES 0x0003
	SEND_CMD_EVENT(OGF_INFO_PARAM, OCF_READ_LOCAL_FEATURES, 0, buf);

	memcpy(info->features, &buf[6], 8);
	//print infor
	DBG("\tLocal Supported Feature	= 0x %02X %02X %02X %02X %02X %02X %02X %02X\n\n\n",
		buf[13], buf[12], buf[11], buf[10], buf[9], buf[8], buf[7], buf[6]);


/********	Send HCI CMD : Read BD_ADDR 									********/
	memset(buf, 0, MAX_EVENT_SIZE);

	// OGF_INFO_PARAM 0x04
	// OCF_READ_BD_ADDR 0x0009
	SEND_CMD_EVENT(OGF_INFO_PARAM, OCF_READ_BD_ADDR, 0, buf);

	memcpy(info->BD_ADDR, &buf[6], 6);

	//print local address
	DBG("\tLocal Address  = 0x %02X %02X %02X %02X %02X %02X \n\n\n",
		buf[11], buf[10], buf[9], buf[8], buf[7], buf[6]);


/********	Send HCI CMD : Read Class of Devices					********/
	memset(buf, 0, MAX_EVENT_SIZE);
	//0x04		OGF_HOST_CTL
	//0x0023	OCF_READ_CLASS_OF_DEV
	DBG("\tWrite Class of Device = 0x%02X%02X%02X\n\n\n", buf[0], buf[1], buf[2]);
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_READ_CLASS_OF_DEV, 0, buf);

	memcpy(info->class, &buf[6], 3);


/********	Send HCI CMD : Read Local Name						********/
	memset(buf, 0, MAX_EVENT_SIZE);
	//0x03			OGF_HOST_CTL
	//0x0014	OCF_READ_LOCAL_NAME
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_READ_LOCAL_NAME, 0, buf);
	memcpy(info->name, &buf[6], HCI_MAX_NAME_LENGTH);

	//print local name
	DBG("\tLocal Name  = %s \n\n\n", buf+6);


/********	Send HCI CMD : Read Buffer Size 							********/
	memset(buf, 0, MAX_EVENT_SIZE);
	// OGF_INFO_PARAM 0x04
	// OCF_READ_BUFFER_SIZE 0x0005
	SEND_CMD_EVENT(OGF_INFO_PARAM, OCF_READ_BUFFER_SIZE, 0, buf);

	read_buffer_size_rp *buffer_szie = (read_buffer_size_rp *)&buf[5];

	info->acl_mtu = buffer_szie->acl_mtu;
	info->sco_mtu = buffer_szie->sco_mtu;
	info->acl_max_pkt = buffer_szie->acl_max_pkt;
	info->sco_max_pkt = buffer_szie->sco_max_pkt;

	return BT_OK;

}

/* Local Host Control */

/*
	Get the bluetooth local host scan mode.
	Parameters:
		mode		Store bluetooth local host scan mode.
	Return:
		return BT_ERROR  when failed.
		return BT_OK hwn success.
*/
int bt_get_host_scan_mode(bt_scan_mode_t *mode)
{
	if (mode == NULL) {
		ERR( ERR_INVALID_PARAM);
		return BT_ERROR;
	}

	OPEN_BT();

	int len;
	unsigned char buf[MAX_EVENT_SIZE];

	memset(buf, 0, MAX_EVENT_SIZE);

	//0x03   		OGF_HOST_CTL
	//0x0019 	OCF_READ_SCAN_ENABLE
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_READ_SCAN_ENABLE, 0, buf);

	*mode = buf[6];

	return BT_OK;

}

/*
	Set the bluetooth local host scan mode.
	Parameters:
		mode		Bluetooth local host scan mode.
	Return:
		return BT_ERROR  when failed.
		return BT_OK hwn success.
*/
int bt_set_host_scan_mode(bt_scan_mode_t mode)
{

	OPEN_BT();

	int len;
	unsigned char buf[MAX_EVENT_SIZE];

	memset(buf, 0, MAX_EVENT_SIZE);
	/*
	0x00 	No Scans enabled.
	0x01
			Inquiry Scan enabled.
			Page Scan disabled.
	0x02
			Inquiry Scan disabled.
			Page Scan enabled.
	0x03
			Inquiry Scan enabled.
			Page Scan enabled.

	*/
	//0x04		OGF_HOST_CTL
	//0x001A	OCF_WRITE_SCAN_ENABLE
	memset(buf, 0, MAX_EVENT_SIZE);
	buf[0] = mode;	//Inquiry Scan disabled. Page Scan enabled.
	DBG("Write Scan Mode = 0x%02X\n\n\n", buf[0]);
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_WRITE_SCAN_ENABLE, 1, buf);

	return BT_OK;

}


/*
	Send normal HCI CMD, not include vendor specific HCI CMD.
	Parameters:
		ogf			HCI CMD OGF.
		ocf 			HCI CMD OCF.
		plen			HCI CMD parameters length.
		param		HCI CMD parameters.
	Return:
		return BT_ERROR  when failed.
		return length of  bluetooth local host name when success.
*/
int bt_hcicmd(uint16_t ogf, uint16_t ocf, uint8_t plen, unsigned char *param)
{
	OPEN_BT();

	int ret;
	unsigned char buf[MAX_EVENT_SIZE];

	memset(buf, 0, MAX_EVENT_SIZE);

	if (plen) {
		if (param == NULL) {
			ERR( "Invalid param : param is NULL\n");
			return BT_ERROR;
		}
		memcpy(buf, param, plen);
	}

	ret = writeHciCommand(bt_fd, ogf, ocf,plen,buf);
	if (ret < 0) {
		ERR( "Send HCI cmd (ogf=0x%02X, ocf=0x%04X) failed\n", ogf, ocf);
		return BT_ERROR;
	}
	if (buf[0] == 0xFF) {
		//qcom vendor cmd, EVENT: buf[0]=0xff, buf[1]=param_len, buf[2]...=params
		return BT_OK;
	} else if(buf[OFFSET_HCI_EVENT_STATUS] != BT_CMD_STATUS_SUCCESS){
		DBG("HCI cmd command (ogf=0x%02X, ocf=0x%04X) failed due to reason 0x%X\n", ogf, ocf,buf[OFFSET_HCI_EVENT_STATUS]);
		return BT_ERROR;
	}

	return BT_OK;
}

/*
	Send normal HCI CMD and return the HCI Event, not include vendor specific HCI CMD.
	Parameters:
		ogf			HCI CMD OGF.
		ocf 			HCI CMD OCF.
		plen			HCI CMD parameters length.
		param		HCI CMD parameters.
		result		HCI CMD Event, length must be set to MAX_EVENT_SIZE
	Return:
		return BT_ERROR  when failed.
		return length of  bluetooth local host name when success.
*/
int bt_hcicmd_result(uint16_t ogf, uint16_t ocf, uint8_t plen, unsigned char *param, unsigned char *result)
{
	OPEN_BT();

	if (result == NULL) {
		ERR( "Invalid param : result is NULL\n");
		return BT_ERROR;
	}

	int ret;
	unsigned char *buf = result;

	memset(buf, 0, MAX_EVENT_SIZE);

	if (plen) {
		if (param == NULL) {
			ERR( "Invalid param : param is NULL\n");
			return BT_ERROR;
		}
		memcpy(buf, param, plen);
	}
	ret = writeHciCommand(bt_fd, ogf, ocf,plen,buf);
	if (ret < 0) {
		ERR( "Send HCI cmd (ogf=0x%02X, ocf=0x%04X) failed\n", ogf, ocf);
		return BT_ERROR;
	}
	if (buf[0] == 0xFF) {
		//qcom vendor cmd, EVENT: buf[0]=0xff, buf[1]=param_len, buf[2]...=params
		return BT_OK;
	}
	else if(buf[OFFSET_HCI_EVENT_STATUS] != BT_CMD_STATUS_SUCCESS){
		DBG("HCI cmd command (ogf=0x%02X, ocf=0x%04X) failed due to reason 0x%X\n", ogf, ocf,buf[OFFSET_HCI_EVENT_STATUS]);
		return BT_ERROR;
	}

	return BT_OK;
}


/*
	Send vendor specific HCI CMD
	Parameters:
		ogf			HCI CMD OGF.
		ocf 			HCI CMD OCF.
		plen			HCI CMD parameters length.
		param		HCI CMD parameters.
	Return:
		return BT_ERROR  when failed.
		return length of  bluetooth local host name when success.
*/
int bt_hcicmd_vendor(uint16_t ogf, uint16_t ocf, uint8_t plen, unsigned char *param)
{
	OPEN_BT();

	unsigned char buf[MAX_EVENT_SIZE];

	memset(buf, 0, MAX_EVENT_SIZE);

	if (plen) {
		if (param == NULL) {
			ERR( "Invalid param : param is NULL\n");
			return BT_ERROR;
		}
		memcpy(buf, param, plen);
	}

	SEND_CMD(ogf, ocf, plen, buf);

	return BT_OK;

}

/*
	Send vendor specific HCI CMD and return the HCI Event.
	Parameters:
		ogf			HCI CMD OGF.
		ocf 			HCI CMD OCF.
		plen			HCI CMD parameters length.
		param		HCI CMD parameters.
		result		HCI CMD Event, length must be set to MAX_EVENT_SIZE
	Return:
		return BT_ERROR  when failed.
		return length of  bluetooth local host name when success.
*/
int bt_hcicmd_vendor_result(uint16_t ogf, uint16_t ocf, uint8_t plen, unsigned char *param, unsigned char *result)
{
	OPEN_BT();

	if (result == NULL) {
		ERR( "Invalid param : result is NULL\n");
		return BT_ERROR;
	}

	unsigned char *buf = result;

	memset(buf, 0, MAX_EVENT_SIZE);

	if (plen) {
		if (param == NULL) {
			ERR( "Invalid param : param is NULL\n");
			return BT_ERROR;
		}
		memcpy(buf, param, plen);
	}

	SEND_CMD(ogf, ocf, plen, buf);

	usleep(CMD_EVENT_DELAY_US);
	memset(buf, 0, MAX_EVENT_SIZE);
	return read_event_modified(bt_fd, buf, 0);


}

/*
	Get bluetooth local host address.
	Parameters:
		mac			Store the address of bluetooth local host. Length is BD_ADDR_SIZE.
	Return:
		return BT_ERROR  when failed.
		return length of  bluetooth local host address.
*/
int bt_get_mac(unsigned char *mac)
{
	if (mac == NULL) {
		ERR( ERR_INVALID_PARAM);
		return BT_ERROR;
	}

	OPEN_BT();
	int i;
	unsigned char buf[MAX_EVENT_SIZE];

	memset(buf, 0, MAX_EVENT_SIZE);

	// OGF_INFO_PARAM 0x04
	// OCF_READ_BD_ADDR 0x0009
	SEND_CMD_EVENT(OGF_INFO_PARAM, OCF_READ_BD_ADDR, 0, buf);

	for(i=0; i<BD_ADDR_SIZE; i++){
		mac[5-i] = buf[6+i];
	}

	return BD_ADDR_SIZE;
}

/*
	Set bluetooth local host address.
	Parameters:
		mac			Address of bluetooth local host. Length is BD_ADDR_SIZE.
	Return:
		return BT_ERROR  when failed.
		return BT_OK when success.
*/
int bt_set_mac(const unsigned char *mac)
{
	if (mac == NULL) {
		ERR( ERR_INVALID_PARAM);
		return BT_ERROR;
	}

	OPEN_BT();

	int i, ret;
	unsigned char buf[MAX_EVENT_SIZE];

	LoadPSHeader(buf,PS_WRITE,BD_ADDR_SIZE,BD_ADDR_PSTAG);

	for(i=0; i<BD_ADDR_SIZE; i++)
		buf[i+3] = mac[5-i];

	SEND_CMD(HCI_VENDOR_CMD_OGF, OCF_PS, BD_ADDR_SIZE + 3, buf);

	usleep(10*1000);

	//send hci cmd RESET
	memset(buf,0,sizeof(buf));
	// OGF_HOST_CTL  0x03
	// OCF_RESET 0x0003
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_RESET, 0, buf);

	return BT_OK;
}

/*
	Enter/Exit DUT test mode.
	Parameters:
		on			1 for enter DUT; 0 for exit DUT.
	Return:
		return BT_ERROR  when failed.
		return BT_OK when success.
*/
int bt_dutmode(int on)
{
	OPEN_BT();

	int ret;
	unsigned char buf[MAX_EVENT_SIZE];

	if (on) {
		memset(buf,0,MAX_EVENT_SIZE);
		//OGF_TEST_CMD
		//OCF_ENABLE_DEVICE_UNDER_TEST_MODE
		SEND_CMD_EVENT(OGF_TEST_CMD, OCF_ENABLE_DEVICE_UNDER_TEST_MODE, 0, buf);


		memset(buf,0,MAX_EVENT_SIZE);
		buf[0] = 0x02;
		buf[1] = 0x00;
		buf[2] = 0x02;
		//OGF_HOST_CTL
		//OCF_SET_EVENT_FILTER
		//02 Connection Setup, 00 Allow all connections, 02 Aoto accept: Role switch disabled
		SEND_CMD_EVENT(OGF_HOST_CTL, OCF_SET_EVENT_FLT, 3, buf);

		memset(buf,0,MAX_EVENT_SIZE);
		buf[0] = 3; //All scan enabled
		// OGF_HOST_CTL
		// OCF_WRITE_SCAN_ENABLE
		SEND_CMD_EVENT(OGF_HOST_CTL, OCF_WRITE_SCAN_ENABLE, 1, buf);

		memset(buf,0,MAX_EVENT_SIZE);
		buf[0] = 0; //disable authentication
		// OGF_HOST_CTL
		// OCF_WRITE_AUTH_ENABLE
		SEND_CMD_EVENT(OGF_HOST_CTL, OCF_WRITE_AUTH_ENABLE, 1, buf);

		memset(buf,0,MAX_EVENT_SIZE);
		buf[0] = 0; //disable encrytion
		// OGF_HOST_CTL
		// OCF_WRITE_ENCRYPT_MODE
		SEND_CMD_EVENT(OGF_HOST_CTL, OCF_WRITE_ENCRYPT_MODE, 1, buf);

		return BT_OK;

	} else {
		//send hci cmd RESET
		memset(buf,0,sizeof(buf));
		// OGF_HOST_CTL  0x03
		// OCF_RESET 0x0003
		SEND_CMD_EVENT(OGF_HOST_CTL, OCF_RESET, 0, buf);

		return BT_OK;
	}
}


/*
	Return the bluetooth local host name.
	Parameters:
		name		Store bluetooth local host name.
		max_len 	Length of name.  Suggest set to MAX_BT_NAME_LEN (248)
	Return:
		return BT_ERROR when failed.
		return BT_OK when success.

	HCI CMD : HCI_Inquiry
	OGF = 0x01, OCF = 0x0001
	Command Parameters:
	LAP : Use 0x9E8B33
	Inquiry_Length:Range: 0x01-0x30, time=Inquiry_Length*1.28s
	Num_Responses:Max device numbers to inqiry
*/
int bt_inquiry(tinquiry_param inq_param,fEvent_InquiryResult fEIR, fEvent_InquiryComlpete fEIC)
{
	OPEN_BT();

	int ret, i;
	unsigned char buf[MAX_EVENT_SIZE];
	unsigned char resultBuf[MAX_EVENT_SIZE];
	unsigned long val32;


/********	Write Inquiry Mode		********/
	memset(buf, 0, MAX_EVENT_SIZE);
	/*
	Inquiry Mode :
	0x00	 Standard Inquiry Result event format (default)
			Event为0x02=Inquiry Result Event，不包含设备名称和RSSI
	0x01	 Inquiry Result format with RSSI
			Event为0x22=Inquiry Result with RSSI Event，不包含设别名称，包含RSSI
	0x02	 Inquiry Result with RSSI format or Extended Inquiry Result format
			Event为0x2F=Extended Inquiry Result Event，包含设备名称和RSSI

	*/
	if (inq_param.mode > BT_INQIRY_MODE_EXTENDED)
		inq_param.mode = BT_INQIRY_MODE_DEFAULT;
	buf[0] = inq_param.mode;
	//0x03		OGF_HOST_CTL
	//0x0045	OCF_WRITE_INQUIRY_MODE
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_WRITE_INQUIRY_MODE, 1, buf);

/********	 Inquiry 					********/

	memset(buf,0,MAX_EVENT_SIZE);
	// INQUIRY_CP_SIZE 5
	buf[0] = 0x33;	//LAP[0]
	buf[1] = 0x8B;	//LAP[1]
	buf[2] = 0x9E;	//LAP[2]
	buf[3] = inq_param.timeout;
	buf[4] = inq_param.dev_num;

	//0x01 		OGF_LINK_CTL
	//0x0001 	OCF_INQUIRY
	// INQUIRY_CP_SIZE 5
	SEND_CMD(OGF_LINK_CTL, OCF_INQUIRY, 5, buf);

	gcb_Event_InqiryResult = fEIR;
	gcb_Event_InqiryComlpete = fEIC;

	memset(&resultBuf, 0, MAX_EVENT_SIZE);
	read_incoming_events(bt_fd, resultBuf, 0);

	gcb_Event_InqiryResult = NULL;
	gcb_Event_InqiryComlpete = NULL;

	return BT_OK;

}

/*
	Set bluetooth to do TX cntinuous.
	Parameters:
		name		Store bluetooth local host name.
		max_len		Length of name.  Suggest set to MAX_BT_NAME_LEN (248)
	Return:
		return BT_ERROR when failed.
		return BT_OK when success.
*/
int bt_vendor_tx_continuous(void)
{
	OPEN_BT();

    int ret = 0;
	unsigned char buf[MAX_EVENT_SIZE];


	memset(buf, 0, MAX_EVENT_SIZE);
	// HCI_VENDOR_CMD_OGF
	// OCF NVM_ACCESS_SET
	buf[0] = 0x01;
	buf[1] = 0x1B;
	buf[2] = 0x01;
	buf[3] = 0x00;
	SEND_CMD(HCI_VENDOR_CMD_OGF, 0x000B, 4, buf);

	memset(buf, 0, MAX_EVENT_SIZE);
	// OGF_HOST_CTL 0x03
	// OCF_RESET 0x0003
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_RESET, 0, buf);

	memset(buf, 0, MAX_EVENT_SIZE);
	// HCI_VENDOR_CMD_OGF
	// OCF PROD_TEST_TX_CONTINUOUS
	buf[0] = 0x05;
	buf[1] = 0x27;
	buf[2] = 0x09;
	buf[3] = 0x05;
	buf[4] = 0x20;
	buf[5] = 0x00;
	buf[6] = 0x00;
	buf[7] = 0x00;
	buf[8] = 0x00;
	SEND_CMD(HCI_VENDOR_CMD_OGF, 0x0004, 9, buf);

	return BT_OK;
}



/*

	Internal Function.

*/

static int bt_open(void)
{
	if (bt_fd != -1)
		return BT_OK;

	gcb_Event_InqiryResult = NULL;
	gcb_Event_InqiryComlpete = NULL;

	bt_fd = open_bt_device();
	if (bt_fd == -1) {
		ERR("bt_open failed\n");
		return BT_ERROR;
	} else
		return BT_OK;
}

static int bt_close(void)
{
	if (bt_fd != -1) {
		close(bt_fd);
		bt_fd = -1;
	}

	gcb_Event_InqiryResult = NULL;
	gcb_Event_InqiryComlpete = NULL;

	return BT_OK;
}




static int open_bt_device(void)
{
	return init_smd(BT_DEV_WCN36X0);

#if 0
	int ret;
	int fd;
	char prop[30];

	ret = property_get("ro.qualcomm.bt.hci_transport", prop, NULL);

	if (!strcasecmp(prop, "smd"))
		fd = init_smd(BT_DEV_WCN36X0);
	else {
		ERR( "Only support qcom smd bluetooth device\n");
		return BT_ERROR;
	}

	return fd;
#endif
}

/*
	Open and init bluetooth HCI device file node.
*/
static int init_smd(char *dev)
{
	int retry = 0;
	struct termios term;
	int fd = -1;

	fd = open(dev, (O_RDWR | O_NOCTTY));

	while ((-1 == fd) && (retry < 60)) {
//		ERR( "Cannot open device. Retry after 100 ms\n");
		usleep(100*1000);
		fd = open(dev, (O_RDWR | O_NOCTTY));
		retry++;
	}

	if (-1 == fd) {
		ERR( "Cannot open device, will exit");
		return BT_ERROR;
	}

	usleep(500*1000);

	if (tcflush(fd, TCIOFLUSH) < 0) {
		ERR( "Cannot flush device");
		close(fd);
		return BT_ERROR;
	}

	if (tcgetattr(fd, &term) < 0) {
		ERR( "Error while getting attributes");
		close(fd);
		return BT_ERROR;
	}

	cfmakeraw(&term);
	term.c_cflag |= (CRTSCTS | CLOCAL);

	if (tcsetattr(fd, TCSANOW, &term) < 0) {
		ERR( "Error while getting attributes");
		close(fd);
		return BT_ERROR;
	}

	return fd;
}


static int set_hci_smd_transport(int on) {
    int sz;
    int fd = -1;
    int ret = -1;
    const char buffer = (on ? '1' : '0');

    fd = open(HCISMD_MOD_PARAM, O_WRONLY);
    if (fd < 0) {
        ERR( "open(%s) for write failed: %s (%d)", HCISMD_MOD_PARAM,
             strerror(errno), errno);
        goto out;
    }
    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        ERR( "write(%s) failed: %s (%d)", HCISMD_MOD_PARAM, strerror(errno),
             errno);
        goto out;
    }
    ret = 0;

out:
    if (fd >= 0) close(fd);
    return ret;
}


/*
	Send HCI cmd. Not waitting event.
*/
static int hci_send_cmd(int fd, uint16_t ogf, uint16_t ocf, uint8_t plen, unsigned char *param)
{
	uint8_t type = 0x01; //HCI_COMMAND_PKT
	uint8_t hci_command_buf[256] = {0};
	uint16_t opcode;
	int i;

	opcode = htobs(HCI_OPCODE_PACK(ogf, ocf));

	//hci_command_buf[0]= type;
	hci_command_buf[0]= (uint8_t)(opcode & 0xff	);
	hci_command_buf[1]= (uint8_t)((opcode>>8) & 0xff);
	hci_command_buf[2]= plen;

	if(plen) {
		memcpy(&hci_command_buf[3], (uint8_t*) param, plen);
	}

#ifdef BT_DEBUG
	if (plen > 0) {
		DBG("Send HCI cmd, ogf=0x%02X, 0cf=0x%04X, plen=%d, param=",ogf, ocf, plen);
		int j;
		for (j=0; j<plen; j++)
			DBG("0x%02X ", hci_command_buf[3+j]);
		DBG("\n");
	} else {
		DBG("Send HCI cmd, ogf=0x%02X, 0cf=0x%04X, plen=%d\n",ogf, ocf, plen);
	}
#endif

	if( write(fd, hci_command_buf, plen+3) < 0) {
		ERR( "Write HCI cmd failed!\n");
		return BT_ERROR;
	}
	tcflush(fd, TCIOFLUSH);
	return BT_OK;
}

/*
 * Read an HCI event from the given file descriptor.
 * no use now.
 */
static int read_hci_event(int fd, unsigned char* buf, int size)
{
	int remain, r, i;
	int count = 0;

	if (size <= 0)
		return BT_ERROR;

	/* The first byte identifies the packet type. For HCI event packets, it
	 * should be 0x04, so we read until we get to the 0x04. */
	while (1) {
		r = read(fd, buf, 1);
		if (r <= 0)
			return BT_ERROR;
		if (buf[0] == 0x04) {
			DBG("pkt: %d\n", buf[0]);
			break;
		}
	}
	count++;

	/* The next two bytes are the event code and parameter total length. */
	while (count < 3) {
		r = read(fd, buf + count, 3 - count);
		if (r <= 0)
			return BT_ERROR;
		count += r;
	}
	for (i=1 ; i<3 ; i++)
		DBG("next pkt: %02x\n", buf[i]);

	/* Now we read the parameters. */
	if (buf[2] < (size - 3))
		remain = buf[2];
	else
		remain = size - 3;

	while ((count - 3) < remain) {
		r = read(fd, buf + count, remain - (count - 3));
		if (r <= 0)
			return BT_ERROR;
		count += r;
	}

	for (i=3 ; i<r ; i++)
		DBG("next pkts: %d\n", buf[i]);


	return count;
}

static int read_event_modified(int fd, unsigned char* buf, int to)
{
	int remain, r,i,size;
	int count = 0;

	//buf[0] = Event code; buf[1] = parameter len
	while (count < 2) {
		r = read(fd, buf + count, 2 - count);
		if (r <= 0)
			return BT_ERROR;
		count += r;
	}

	if (buf[1] == 0)
	{
		ERR("Read HCI CMD Event error : Zero len , invalid , buf = %02X %02X\n", buf[0], buf[1]);
		return BT_ERROR;
	}
	size = buf[1]+2;

	/* Now we read the parameters. */
	while (count  < size ) {
		r = read(fd, buf + count, size);
		if (r <= 0)
		{
			ERR("Read HCI CMD Event error : read error \n");
			return BT_ERROR;
		}
		count += r;
	}

#ifdef BT_DEBUG
	DBG("\nHCI Event (len = %d) = 0x", count);
	for (i=0 ; i< count ; i++)
		DBG(" %02X", buf[i]);
	DBG("\n");
#endif

	return count;
}
int read_incoming_events(int fd, unsigned char* buf, int to)
{
	int remain, r,i,size;
	int count = 0;
	time_t start, finish, checkbreak;
	struct timeval tv1, tv2, timeout;
	unsigned long long start_utime, end_utime, time_diff;
	unsigned long long diff;

	start = time(NULL);
	gettimeofday(&tv1, NULL);
	start_utime = tv1.tv_sec*1000000 + tv1.tv_usec;

	do{
		/* The first byte identifies the packet type. For HCI event packets, it
		 * should be 0x04, so we read until we get to the 0x04. */
		/* The next two bytes are the event code and parameter total length. */
		while (count < 2) {
			r = read(fd, buf + count, 2 - count);
			if (r <= 0){
				ERR("Read HCI Incoming Event error : read error \n");
				return BT_ERROR;
            }
			count += r;
		}


		if (buf[1] == 0)
		{
			ERR("Read HCI Incoming Event error : Zero len , invalid , buf = %02X %02X\n", buf[0], buf[1]);
			return BT_ERROR;
		}
		size = buf[1];

		/* Now we read the parameters. */
		while (count  < size ) {
			r = read(fd, buf + count, size);
			if (r <= 0)
			{
				ERR("Read HCI Incoming Event error : read error, size = %d\n",size);
				return BT_ERROR;
			}
			count += r;
		}
		#ifdef BT_DEBUG
		DBG("****************Got EVENT 0x%02X \n", buf[0]);
		#endif

		//check event code
		switch (buf[0])
		{
			int j=0;
			case EVT_CMD_STATUS:	//0x0f
				DBG("Command Complete Event : 0x");
				for (j=0 ; j< buf[1] + 2 ; j++)
					DBG(" %02X", buf[j]);
				DBG("\n");
				if (buf[2] != 0) {
					ERR("CMD complete status error = %d\n", buf[2]);
				}

/* Tony
                if(buf[2] == HCI_NO_CONNECTION)
                {
                    DBG("\nUnknown connection identifier");
                    return BT_OK;
                }
*/
				break;

			case EVT_INQUIRY_RESULT:	//0x02:
				#ifdef BT_DEBUG
				DBG("Inquiry Result Event : 0x");
				for (j=0 ; j< buf[1] + 2 ; j++)
					DBG(" %02X", buf[j]);
				DBG("\n");
				#endif

				if (gcb_Event_InqiryResult) {
					tDevice_InquiryInfo info;
					inquiry_info *inq_info;
					int k;
					int Num_Responses;
					Num_Responses = buf[2];

					memset(&info, 0, sizeof(tDevice_InquiryInfo));
					inq_info = (inquiry_info *)&buf[3];
					for (k=0; k<Num_Responses; k++) {
						info.EventMode = BT_INQIRY_MODE_DEFAULT;
						memcpy(info.bdaddr, inq_info->bdaddr.b, 6);
						memcpy(info.dev_class, inq_info->dev_class, 3);
						info.pscan_rep_mode = inq_info->pscan_rep_mode;
						info.pscan_period_mode = inq_info->pscan_period_mode;
						info.pscan_mode = inq_info->pscan_mode;
						info.clock_offset = inq_info->clock_offset;


						(*gcb_Event_InqiryResult)(&info);
						inq_info += sizeof(inquiry_info);
					}
				}

				break;

			case EVT_INQUIRY_RESULT_WITH_RSSI:	//0x22:
				#ifdef BT_DEBUG
				DBG("Inquiry result with RSSI Event : 0x");
				for (j=0 ; j< buf[1] + 2 ; j++)
					DBG(" %02X", buf[j]);
				DBG("\n");
				#endif

				if (gcb_Event_InqiryResult) {
					tDevice_InquiryInfo info;
					inquiry_info_with_rssi *inq_info;
					int k;
					int Num_Responses;
					Num_Responses = buf[2];

					memset(&info, 0, sizeof(tDevice_InquiryInfo));
					inq_info = (inquiry_info_with_rssi *)&buf[3];
					for (k=0; k<Num_Responses; k++) {
						info.EventMode = BT_INQIRY_MODE_RSSI;
						memcpy(info.bdaddr, inq_info->bdaddr.b, 6);
						memcpy(info.dev_class, inq_info->dev_class, 3);
						info.pscan_rep_mode = inq_info->pscan_rep_mode;
						info.pscan_period_mode = inq_info->pscan_period_mode;
						info.clock_offset = inq_info->clock_offset;
						info.rssi = inq_info->rssi;

						(*gcb_Event_InqiryResult)(&info);
						inq_info += sizeof(inquiry_info_with_rssi);
					}
				}

				break;

			case EVT_EXTENDED_INQUIRY_RESULT://0x2F:
				#ifdef BT_DEBUG
				DBG("Extended inquiry result Event : 0x");
				for (j=0 ; j< buf[1] + 2 ; j++)
					DBG(" %02X", buf[j]);
				DBG("\n");
				#endif

				if (gcb_Event_InqiryResult) {
					tDevice_InquiryInfo info;
					extended_inquiry_info *inq_info;
					int k;
					int Num_Responses;
					Num_Responses = buf[2];

					inq_info = (extended_inquiry_info *)&buf[3];
					for (k=0; k<Num_Responses; k++) {

						memset(&info, 0, sizeof(tDevice_InquiryInfo));

						info.EventMode = BT_INQIRY_MODE_EXTENDED;
						memcpy(info.bdaddr, inq_info->bdaddr.b, 6);
						memcpy(info.dev_class, inq_info->dev_class, 3);
						info.pscan_rep_mode = inq_info->pscan_rep_mode;
						info.pscan_period_mode = inq_info->pscan_period_mode;
						info.clock_offset = inq_info->clock_offset;
						info.rssi = inq_info->rssi;

						int eir_count=0;
						unsigned char *eir_data = inq_info->data;
						while(eir_count < HCI_MAX_EIR_LENGTH) {
							if (eir_data[0] > 0) {
								//check eir type
								if (eir_data[1] == 0x08) {
									//Shortened local name
									info.name_type = 1;
									if ( eir_data[0] > 1)
										memcpy(info.name, &eir_data[2], eir_data[0] - 1);
								} else if (eir_data[1] == 0x09) {
									//Complete local name
									info.name_type = 0;
									if ( eir_data[0] > 1)
										memcpy(info.name, &eir_data[2], eir_data[0] - 1);
								}
								eir_count += eir_data[0];
								eir_data += eir_data[0];
							} else {
								//no more EIR?
								break;
							}
						}

						(*gcb_Event_InqiryResult)(&info);
						inq_info += sizeof(extended_inquiry_info);
					}
				}

				break;

			case EVT_INQUIRY_COMPLETE:		//0x01:
				#ifdef BT_DEBUG
				DBG("Inquiry complete Event : 0x");
				for (j=0 ; j< buf[1] + 2 ; j++)
					DBG(" %02X", buf[j]);
				DBG("\n");
				#endif

				if (gcb_Event_InqiryComlpete) {
					(*gcb_Event_InqiryComlpete)();
				}

				return BT_OK;

			case EVT_CONN_COMPLETE:	//0x03:
				DBG("Connection complete Event : 0x");
				for (j=0 ; j< buf[1] + 2 ; j++)
					DBG(" %02X", buf[j]);
				DBG("\n");
				if(buf[2] == 0x00) {
					DBG("Connection complete Event : Handle is 0x%02x%02x \n",buf[4],buf[3]);
				} else {
					DBG("Connection complete Event : Error code 0x%x \n",buf[2]);
				}

				return BT_OK;

			case EVT_DISCONN_COMPLETE:	//0x05:
				DBG("Disonnection complete Event : 0x");
				for (j=0 ; j< buf[1] + 2 ; j++)
					DBG(" %02X", buf[j]);
				DBG("\n");
				DBG("Disconnection complete Event : Error code 0x%x \n",buf[5]);
				return BT_OK;

			default:
				DBG("Other event received, Breaking, Event : 0x\n");
				for (j=0 ; j< buf[1] + 2 ; j++)
					DBG(" %02X", buf[j]);
				DBG("\n");
				memset(buf , 0, sizeof(buf));
				count = 0; i = 0;size =0;
				break;

		}

		memset(buf , 0, sizeof(buf));
		count = 0; i = 0;size =0;

		/* buf[1] should be the event opcode
		 * buf[2] shoudl be the parameter Len of the event opcode
		 */
	}while (1);
	return count;
}


/*
	Send HCI cmd and get event.
*/
static int writeHciCommand(int fd, uint16_t ogf, uint16_t ocf, uint8_t plen, unsigned char *buf)
{
	int ret;

	tcflush(fd, TCIOFLUSH);

	if (hci_send_cmd(fd, ogf, ocf, plen, buf) < 0) {
		ERR( "Send HCI CMD failed");
		return BT_ERROR;
	}
	usleep(CMD_EVENT_DELAY_US);
	memset(buf, 0, MAX_EVENT_SIZE);
	ret = read_event_modified(fd, buf, 0);

	return ret;
}

static void LoadPSHeader(unsigned char *HCI_PS_Command,unsigned char opcode,int length,int index)
{

	HCI_PS_Command[0]= 0x01;
	HCI_PS_Command[1]= 0x02;
	HCI_PS_Command[2]= BD_ADDR_SIZE;

}


/*
	Config bluetooth host after enabled.
	Parameters:
	Return:
		return BT_ERROR  when failed.
		return BT_OK when success.
*/
static int bt_setup(void)
{
	OPEN_BT();

	int i, len, ret;
	unsigned char buf[MAX_EVENT_SIZE];

	memset(buf, 0, MAX_EVENT_SIZE);


/********	Send HCI CMD : RESET					 					********/

	memset(buf, 0, MAX_EVENT_SIZE);
	// OGF_HOST_CTL 0x03
	// OCF_RESET 0x0003
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_RESET, 0, buf);


/********	Send HCI CMD : Read Local Version Information 				********/
	memset(buf, 0, MAX_EVENT_SIZE);
	// OGF_INFO_PARAM 0x04
	// OCF_READ_LOCAL_VERSION 0x0001
	SEND_CMD_EVENT(OGF_INFO_PARAM, OCF_READ_LOCAL_VERSION, 0, buf);

	//Print local version
	if (buf[6] == 0x06)
		DBG("\tHCI Version       = 0x%02X  [Bluetooth Core Specification 4.0]\n", buf[6]);
	else
		DBG("\tHCI Version       = 0x%02X\n", buf[6]);
	DBG("\tHCI Revision      = 0x%02X%02X\n", buf[8], buf[7]);
	DBG("\tLMP Version       = 0x%02X\n", buf[9]);
	if ((buf[11] == 0x00) && (buf[10] == 0x1D))
		DBG("\tManufacturer Name = 0x%02X%02X  [Qualcomm]\n", buf[11], buf[10]);
	else
		DBG("\tManufacturer Name = 0x%02X%02X\n", buf[11], buf[10]);
	DBG("\tLMP Subversion    = 0x%02X%02X\n", buf[13], buf[12]);
	DBG("\n\n");


/********	Send HCI CMD : Read Flow Control Mode		 					********/
/********	Send HCI CMD : Read Buffer Size			 					********/
/********	Send HCI CMD : Read Data Block Size							********/


/********	Send HCI CMD : Read Local Supported Feature 					********/
	memset(buf, 0, MAX_EVENT_SIZE);
	// OGF_INFO_PARAM 0x04
	// OCF_READ_LOCAL_FEATURES 0x0003
	SEND_CMD_EVENT(OGF_INFO_PARAM, OCF_READ_LOCAL_FEATURES, 0, buf);

	//print infor
	DBG("\tLocal Supported Feature  = 0x %02X %02X %02X %02X %02X %02X %02X %02X\n\n\n",
		buf[13], buf[12], buf[11], buf[10], buf[9], buf[8], buf[7], buf[6]);


/********	Send HCI CMD : Read BD_ADDR                    					********/
	memset(buf, 0, MAX_EVENT_SIZE);

	// OGF_INFO_PARAM 0x04
	// OCF_READ_BD_ADDR 0x0009
	SEND_CMD_EVENT(OGF_INFO_PARAM, OCF_READ_BD_ADDR, 0, buf);

	//print local address
	DBG("\tLocal Address  = 0x %02X %02X %02X %02X %02X %02X \n\n\n",
		buf[11], buf[10], buf[9], buf[8], buf[7], buf[6]);


/********	Send HCI CMD : Read Class of Devices 					********/

/********	Send HCI CMD : Read Local Name	 					********/
	memset(buf, 0, MAX_EVENT_SIZE);
	//0x03   		OGF_HOST_CTL
	//0x0014 	OCF_READ_LOCAL_NAME
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_READ_LOCAL_NAME, 0, buf);

	//print local name
	DBG("\tLocal Name  = %s \n\n\n", buf+6);


/********	Send HCI CMD : Read Voice Setting						********/


/********	Send HCI CMD : Set Event Filter						********/
	memset(buf, 0, MAX_EVENT_SIZE);
	buf[0] = 0x00;	//Clear all filters
	//0x03 		OGF_HOST_CTL
	//0x0005 	OCF_SET_EVENT_FLT
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_SET_EVENT_FLT, 1, buf);


/********	Send HCI CMD : Write Connection Accept Timeout			********/
	memset(buf, 0, MAX_EVENT_SIZE);
	//set timeout=0x7D00 *0.625 = 20000 ms
	buf[0] = 0x00;
	buf[1] = 0x7D;
	//0x03 		OGF_HOST_CTL
	//0x0016 	OCF_WRITE_CONN_ACCEPT_TIMEOUT
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_WRITE_CONN_ACCEPT_TIMEOUT, 2, buf);


/********	Send HCI CMD : Read Local Supported Commands 		********/
	memset(buf, 0, MAX_EVENT_SIZE);
	//0x04		OGF_INFO_PARAM
	//0x00012	OCF_READ_LOCAL_COMMANDS
	SEND_CMD_EVENT(OGF_INFO_PARAM, OCF_READ_LOCAL_COMMANDS, 0, buf);

	//print 64 bytes commands
	DBG("\tLocal Supported Commands  = 0x");
	for(i=0; i<64; i++) {
		if (!(i & 0x0F))
			DBG("\n\t");
		DBG("%02X ", buf[6+i]);
	}
	DBG("\n\n");


/********	Send HCI CMD : Write Simple Pairing Mode 		********/
	memset(buf, 0, MAX_EVENT_SIZE);
	buf[0] = 0x01;	//enable SSP
	//0x03		OGF_HOST_CTL
	//0x0056	OCF_WRITE_SIMPLE_PAIRING_MODE
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_WRITE_SIMPLE_PAIRING_MODE, 1, buf);


/********	Send HCI CMD : Write Inquiry Mode		********/
	memset(buf, 0, MAX_EVENT_SIZE);
	/*
	Inquiry Mode :
	0x00	 Standard Inquiry Result event format (default)
			Event为0x02=Inquiry Result Event，不包含设备名称和RSSI
	0x01	 Inquiry Result format with RSSI
			Event为0x22=Inquiry Result with RSSI Event，不包含设别名称，包含RSSI
	0x02	 Inquiry Result with RSSI format or Extended Inquiry Result format
			Event为0x2F=Extended Inquiry Result Event，包含设备名称和RSSI

	*/
	buf[0] = 0x02;
	//0x03		OGF_HOST_CTL
	//0x0045	OCF_WRITE_INQUIRY_MODE
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_WRITE_INQUIRY_MODE, 1, buf);

/********	Send HCI CMD : Read Inquiry Transimit Power Level	********/


/********	Send HCI CMD : Set Event Mask					********/
	memset(buf, 0, MAX_EVENT_SIZE);
	buf[0] = 0xFF;
	buf[1] = 0xFF;
	buf[2] = 0xFB;
	buf[3] = 0xFF;
	buf[4] = 0x07;
	buf[5] = 0xF8;
	buf[6] = 0xBF;
	buf[7] = 0x3D;

	//0x03		OGF_HOST_CTL
	//0x0001	OCF_SET_EVENT_MASK
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_SET_EVENT_MASK, 8, buf);

/********	Send HCI CMD : Write Default Lin Policy Settings		********/
	memset(buf, 0, MAX_EVENT_SIZE);
	buf[0] = 0x07;
	buf[1] = 0x00;

	//0x02		OGF_LINK_POLICY
	//0x000F		OCF_WRITE_DEFAULT_LINK_POLICY
	SEND_CMD_EVENT(OGF_LINK_POLICY, OCF_WRITE_DEFAULT_LINK_POLICY, 2, buf);

/********	Send HCI CMD : HCI LE Read Buffer Size			********/
/********	Send HCI CMD : HCI LE Clear White List			********/
/********	Send HCI CMD : HCI LE Read White List Size 		********/

/********	Send HCI CMD : Change Local Name			********/
	//0x03		OGF_HOST_CTL
	//0x0013	OCF_CHANGE_LOCAL_NAME
	memset(buf, 0, MAX_EVENT_SIZE);
	strcpy(buf, DEFAULT_LOCAL_NAME);
	DBG("\tSet Local Name to = %s\n\n\n", buf);
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_CHANGE_LOCAL_NAME, MAX_BT_NAME_LEN, buf);



/********	Send HCI CMD : Write Class of Device			********/
	//0x04		OGF_HOST_CTL
	//0x0024	OCF_WRITE_CLASS_OF_DEV
	memset(buf, 0, MAX_EVENT_SIZE);
	buf[0] = 0x0C;
	buf[1] = 0x02;
	buf[2] = 0x5A;
	DBG("\tWrite Class of Device = 0x%02X%02X%02X\n\n\n", buf[0], buf[1], buf[2]);
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_WRITE_CLASS_OF_DEV, 3, buf);



/********	Send HCI CMD : Write Scan Enable				********/
	/*
	0x00 	No Scans enabled.
	0x01
			Inquiry Scan enabled.
			Page Scan disabled.
	0x02
			Inquiry Scan disabled.
			Page Scan enabled.
	0x03
			Inquiry Scan enabled.
			Page Scan enabled.

	*/
	//0x04		OGF_HOST_CTL
	//0x001A	OCF_WRITE_SCAN_ENABLE
	memset(buf, 0, MAX_EVENT_SIZE);
	buf[0] = 0x02;	//Inquiry Scan disabled. Page Scan enabled.
	DBG("\tWrite Scan Mode = 0x%02X\n\n\n", buf[0]);
	SEND_CMD_EVENT(OGF_HOST_CTL, OCF_WRITE_SCAN_ENABLE, 1, buf);


/********	Send HCI CMD : HCI LE Set Scan Parameters			********/



	return BT_OK;
}



