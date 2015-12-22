#ifndef _BT_TOOL_H_
#define _BT_TOOL_H_

#define BT_TOOL_VERSION 	"0.1"

#ifndef MAX_EVENT_SIZE
#define MAX_EVENT_SIZE	260
#endif

#define MAX_BT_NAME_LEN		248

//BT op error no
#define BT_OK			0
#define BT_ERROR		-1


/* Define about BT Host Information */
typedef struct thost_info{
	unsigned char LAP[3];
	unsigned char Inquiry_Length;
	unsigned char Num_Responses;
}thost_info;

/* Define scan mode */
typedef enum {
	BT_SCAN_MODE_NOSCAN,	//0x00,No Scans enabled.
	BT_SCAN_MODE_PSCAN,		//0x01,Inquiry Scan enabled.Page Scan disabled.
	BT_SCAN_MODE_ISCAN,		//0x02,Inquiry Scan disabled.,Page Scan enabled.
	BT_SCAN_MODE_PISCAN,	//0x03,Inquiry Scan enabled.Page Scan enabled.
} bt_scan_mode_t;




/* Define about Inqiry */

#define BT_INQIRY_MODE_DEFAULT		0x00
#define BT_INQIRY_MODE_RSSI			0x01
#define BT_INQIRY_MODE_EXTENDED		0x02


//length = 5
typedef struct tinquiry_param{
//	unsigned char LAP[3];
	unsigned char timeout;	//Inquiry max time
	unsigned char dev_num;	//Inquiry max device
	unsigned char mode;
}tinquiry_param;



typedef struct tDevice_InquiryInfo {
	uint8_t EventMode;
	uint8_t bdaddr[6];
	uint8_t dev_class[3];
	uint8_t pscan_rep_mode;
	uint8_t pscan_period_mode;
	uint8_t pscan_mode;
	uint16_t clock_offset;
	uint8_t rssi;
	uint8_t name_type;	//0:Complete local name; 1:Shortened local name
	uint8_t name[MAX_BT_NAME_LEN];
}tDevice_InquiryInfo;
/*
typedef struct tDevice_InqiryInfo {
	unsigned char BD_ADDR[6];
	unsigned char Page_Scan_Repetition_Mode;
	unsigned char Reserved1;
	unsigned char Reserved2;
	unsigned char Class_of_Device[3];
	unsigned char Clock_Offset[2];
}__attribute__ ((packed)) tDevice_InqiryInfo;

typedef struct tDevice_InqiryInfo_Rssi {
	unsigned char BD_ADDR[6];
	unsigned char Page_Scan_Repetition_Mode;
	unsigned char Reserved;
	unsigned char Class_of_Device[3];
	unsigned char Clock_Offset[2];
	unsigned char RSSI;
}__attribute__ ((packed)) tDevice_InqiryInfo_Rssi;

typedef struct tDevice_InqiryInfo_Ext {
	unsigned char BD_ADDR[6];
	unsigned char Page_Scan_Repetition_Mode;
	unsigned char Reserved;
	unsigned char Class_of_Device[3];
	unsigned char Clock_Offset[2];
	unsigned char RSSI;
	unsigned char * EIR
}__attribute__ ((packed)) tDevice_InqiryInfo_Ext;
*/

/* Callbak of Inrqiry Result Event. */
typedef int (*fEvent_InquiryResult)(tDevice_InquiryInfo *info);
/* Callbak of Inrqiry Complete Event. */
typedef int (*fEvent_InquiryComlpete)(void);

//Host information
typedef struct tHost_Info {
	uint8_t		hci_ver;
	uint16_t	hci_rev;
	uint8_t		lmp_ver;
	uint16_t	manufacturer;
	uint16_t	lmp_subver;

	uint8_t 	features[8];
//	uint8_t Packet_type[];
//	uint8_t 	Link_Policy[2];
//	uint8_t Link_Mode[2];
	uint8_t 	BD_ADDR[6];
	uint8_t 	class[3];
	uint8_t 	name[MAX_BT_NAME_LEN];

	uint16_t	acl_mtu;
	uint8_t		sco_mtu;
	uint16_t	acl_max_pkt;
	uint16_t	sco_max_pkt;
}tHost_Info;


/******	Bluetooth Function	******/


int bt_enable(void);
int bt_disable(void);
int bt_is_enabled(void);


int bt_reset(void);


/* Local Host Info */
int bt_get_host_name(char *name, int max_len);
int bt_set_host_name(const char *name);
int bt_get_host_inof(tHost_Info *info);

/* Local Host Control */
int bt_get_host_scan_mode(bt_scan_mode_t *mode);
int bt_set_host_scan_mode(bt_scan_mode_t mode);

//Send normal HCI CMD
int bt_hcicmd(uint16_t ogf, uint16_t ocf, uint8_t plen, unsigned char *param);
int bt_hcicmd_result(uint16_t ogf, uint16_t ocf, uint8_t plen, unsigned char *param, unsigned char *result);

//Send Vendor specific HCI CMD
int bt_hcicmd_vendor(uint16_t ogf, uint16_t ocf, uint8_t plen, unsigned char *param);
int bt_hcicmd_vendor_result(uint16_t ogf, uint16_t ocf, uint8_t plen, unsigned char *param, unsigned char *result);


//高位地址在前,与手机显示的地址顺序相同
int bt_get_mac(unsigned char *mac);
//高位地址在前,与手机显示的地址顺序相同
int bt_set_mac(const unsigned char *mac);

//Enter/Exit DUT test mode.
int bt_dutmode(int on);
int bt_inquiry(tinquiry_param inq_param,fEvent_InquiryResult fEIR, fEvent_InquiryComlpete fEIC);


//use for bt current on/off test
int bt_vendor_tx_continuous(void);



#endif

