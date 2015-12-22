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
#include <math.h>

//>>>>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
//<<<<<<<<<<<


#ifdef ANDROID
#include <termios.h>
#else
#include <sys/termios.h>
#include <sys/ioctl.h>
#include <limits.h>
#endif


#include "bt_tool.h"


#define PROG_NAME	"bt_tool"

#if 0
static void cmd_rba(int argc, char **argv)
{
	unsigned char mac[6];

	if (bt_get_mac(mac) < 0) {
		fprintf(stderr, "Read bt mac failed\n");
		return;
	}

	printf("Read bt mac finish. \n");
	printf("BT MAC is : %02X:%02X:%02X:%02X:%02X:%02X\n",
		mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

	return;

}


static void cmd_wba(int argc, char **argv)
{
	int i;
	unsigned char mac[6];

#if 0
	int seed;
	struct timespec sTime;

	if (-1 == clock_gettime (CLOCK_REALTIME, &sTime))
	{
		fprintf(stderr, "clock_gettime\n" );
		return;
	}

	seed = sTime.tv_nsec;
	srand ((unsigned int) seed);

	mac[0] = (rand() & 0x0FE00000) >> 20;
	mac[1] = (rand() & 0x0FF00000) >> 20;
	mac[2] = (rand() & 0x0FF00000) >> 20;
	mac[3] = (rand() & 0x0FF00000) >> 20;
	mac[4] = (rand() & 0x0FF00000) >> 20;
	mac[5] = (rand() & 0x0FF00000) >> 20;
#endif
	if (argc != 6) {
		fprintf(stderr, "We need 6 bt mac. \n");
		fprintf(stderr,"Example : bt_tool wba D8 71 57 00 11 22\n");
		return;
	}

	for (i=0; i<6; i++)
		mac[i] = strtoul(argv[i+1], NULL, 16);


	printf("Set BT MAC to : %02X:%02X:%02X:%02X:%02X:%02X\n",
		mac[5],mac[4],mac[3],mac[2],mac[1],mac[0]);

	if (bt_set_mac(mac) < 0) {
		fprintf(stderr, "Read bt mac failed\n");
		return;
	}

	printf("Write bt mac finish. \n");

	return;

}
#endif

static void cmd_on(int argc, char **argv)
{

	if (bt_enable() < 0) {
		fprintf(stderr, "BT Enable failed\n");
		return;
	}
	printf("\n");

	return;
}

static void cmd_off(int argc, char **argv)
{

	if (bt_disable() < 0) {
		fprintf(stderr, "BT Disable failed\n");
		return;
	}

	return;
}
static void get_host_info(void)
{
	char buf[256];
	tHost_Info info;
	if (bt_get_host_info(&info) < 0) {
		fprintf(stderr, "BT get host information failed\n");
		return;
	}

	printf("\tBD Address: %02X %02X %02X %02X %02X %02X\tACL MTU: %d:%d\t SCO MTU: %d:%d\n",
		info.BD_ADDR[5],info.BD_ADDR[4],info.BD_ADDR[3],
		info.BD_ADDR[2],info.BD_ADDR[1],info.BD_ADDR[0],
		info.acl_mtu, info.acl_max_pkt, info.sco_mtu, info.sco_max_pkt);

	printf("\tFeatures: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
		info.features[0],info.features[1],info.features[2],info.features[3],
		info.features[4],info.features[5],info.features[6],info.features[7]);

	printf("\tName: %s\n", info.name);

	printf("\tClass: 0x%02X%02X%02X\n",info.class[2],info.class[1],info.class[0]);
	unsigned int cls = info.class[2]<<16 | info.class[1]<<8 | info.class[0];

	sprintf(buf, "\tService Classes:");
	if((cls & 0xFFE000) & 0x010000)
		sprintf(buf+strlen(buf), " Postioning,");
	if((cls & 0xFFE000) & 0x020000)
		sprintf(buf+strlen(buf), " Networking,");
	if((cls & 0xFFE000) & 0x040000)
		sprintf(buf+strlen(buf), " Render,");
	if((cls & 0xFFE000) & 0x080000)
		sprintf(buf+strlen(buf), " Capture,");
	if((cls & 0xFFE000) & 0x100000)
		sprintf(buf+strlen(buf), " Object Transfer,");
	if((cls & 0xFFE000) & 0x200000)
		sprintf(buf+strlen(buf), " Audio,");
	if((cls & 0xFFE000) & 0x400000)
		sprintf(buf+strlen(buf), " Telephony,");
	if((cls & 0xFFE000) & 0x800000)
		sprintf(buf+strlen(buf), " Information,");
	buf[strlen(buf)-1] = '\0';
	printf("%s\n", buf);

	sprintf(buf, "\tDevice Classes:");
	if(((cls & 0x1FFC) & 0x0100) == 0x0100)
		sprintf(buf+strlen(buf), " Computer,");
	if(((cls & 0x1FFC) & 0x0200) == 0x0200)
		sprintf(buf+strlen(buf), " Phone,");
	if(((cls & 0x1FFC) & 0x020C) == 0x020C)
		sprintf(buf+strlen(buf), " Smart Phone,");
	if(((cls & 0x1FFC) & 0x0300) == 0x0300)
		sprintf(buf+strlen(buf), " Networking,");
	if(((cls & 0x1FFC) & 0x0400) == 0x0400)
		sprintf(buf+strlen(buf), " Audio Video,");
	if(((cls & 0x1FFC) & 0x0500) == 0x0500)
		sprintf(buf+strlen(buf), " Peripheral,");
	if(((cls & 0x1FFC) & 0x0600) == 0x0600)
		sprintf(buf+strlen(buf), " Imaging,");
	if(((cls & 0x1FFC) & 0x0700) == 0x0700)
		sprintf(buf+strlen(buf), " Wearable,");
	if(((cls & 0x1FFC) & 0x0800) == 0x0800)
		sprintf(buf+strlen(buf), " Toy,");
	if(((cls & 0x1FFC) & 0x0900) == 0x0900)
		sprintf(buf+strlen(buf), " Health,");
	if(((cls & 0x1FFC) & 0x1F00) == 0x1F00)
		sprintf(buf+strlen(buf), " UNCATEGORIZED,");
	if(!(cls & 0x1FFC))
		sprintf(buf+strlen(buf), " MISC,");
	buf[strlen(buf)-1] = '\0';
	printf("%s\n", buf);

	if (info.hci_ver == 0x06)
		printf("\tHCI Version: 4.0 (0x%02X)  Revison=0x%02X\n", info.hci_ver, info.hci_rev);
	else
		printf("\tHCI Version: 0x%02X  Revison=0x%02X\n", info.hci_ver, info.hci_rev);

	if (info.lmp_ver== 0x06)
		printf("\tLMP Version: 4.0 (0x%02X)  Subvison=0x%02X\n", info.lmp_ver, info.lmp_subver);
	else
		printf("\tLMP Version: 0x%02X  Subvison=0x%02X\n", info.lmp_ver, info.lmp_subver);

	if (info.manufacturer== 0x001D)
		printf("\tManufacturer: Qualcomm (%d)\n", info.manufacturer);
	else
		printf("\tManufacturer: %d\n", info.manufacturer);

	return;
}

static void cmd_reset(int argc, char **argv)
{

	if (bt_reset() < 0) {
		fprintf(stderr, "BT reset failed\n");
		return;
	}
	printf("BT reset finish. \n");

	return;
}

static int do_scan(bt_scan_mode_t mode)
{
	bt_scan_mode_t rm;
	if (bt_get_host_scan_mode(&rm) != BT_OK) {
		fprintf(stderr, "BT get scan mode failed\n");
	}
	printf("Current scan mode is %d\n", rm);
	printf("Set scan mode to %d\n", mode);
	return bt_set_host_scan_mode(mode);
}

static void cmd_piscan(int argc, char **argv)
{

	if (do_scan(BT_SCAN_MODE_PISCAN) < 0) {
		fprintf(stderr, "BT set scan mode failed\n");
		return;
	}

	return;
}

static void cmd_noscan(int argc, char **argv)
{

	if (do_scan(BT_SCAN_MODE_NOSCAN) < 0) {
		fprintf(stderr, "BT set scan mode failed\n");
		return;
	}

	return;
}

static void cmd_iscan(int argc, char **argv)
{

	if (do_scan(BT_SCAN_MODE_ISCAN) < 0) {
		fprintf(stderr, "BT set scan mode failed\n");
		return;
	}

	return;
}

static void cmd_pscan(int argc, char **argv)
{

	if (do_scan(BT_SCAN_MODE_PSCAN) < 0) {
		fprintf(stderr, "BT set scan mode failed\n");
		return;
	}

	return;
}




static void cmd_dut(int argc, char **argv)
{
	if (bt_dutmode(1) < 0) {
		fprintf(stderr, "Enter DUT mode failed\n");
		return;
	}
	printf("Enter DUT mode finish. \n");
	printf("Press any key to exit DUT mode.\n");

	getchar();
	bt_dutmode(0);
	printf("Exit DUT mode finish. \n");


	return;
}

static void cmd_duton(int argc, char **argv)
{
	if (bt_enable() << 0){
		fprintf(stderr, "Enable bt failed\n");
		return;
	}

	if (bt_dutmode(1) < 0) {
		fprintf(stderr, "Enter DUT mode failed\n");
		return;
	}

	printf("Enter DUT mode finish. \n");

	return;
}

static void cmd_dutoff(int argc, char **argv)
{

	bt_dutmode(0);
	printf("Exit DUT mode finish. \n");

	return;
}

int cb_InqiryStatus(tDevice_InquiryInfo *info)
{

	switch (info->EventMode) {
	case BT_INQIRY_MODE_DEFAULT:
		printf("\t%02X:%02X:%02X:%02X:%02X:%02X\t%-16s\tclass: 0x%02X%02X%02X    clock offset: 0x%04X\n",
			info->bdaddr[5],info->bdaddr[4],info->bdaddr[3],
			info->bdaddr[2],info->bdaddr[1],info->bdaddr[0],
			"n/a",
			info->dev_class[2],info->dev_class[1],info->dev_class[0],
			info->clock_offset);
		break;
	case BT_INQIRY_MODE_RSSI:
		printf("\t%02X:%02X:%02X:%02X:%02X:%02X\t%-16s\tclass: 0x%02X%02X%02X    clock offset: 0x%04X    ",
			info->bdaddr[5],info->bdaddr[4],info->bdaddr[3],
			info->bdaddr[2],info->bdaddr[1],info->bdaddr[0],
			"n/a",
			info->dev_class[2],info->dev_class[1],info->dev_class[0],
			info->clock_offset);
		if (info->rssi< 0x80)
			printf("%d dBm\n", (int)info->rssi);
		else
			printf("%d dBm\n", (int)info->rssi - 256);
		break;
	case BT_INQIRY_MODE_EXTENDED:
		printf("\t%02X:%02X:%02X:%02X:%02X:%02X\t%-16s\tclass: 0x%02X%02X%02X    clock offset: 0x%04X    ",
			info->bdaddr[5],info->bdaddr[4],info->bdaddr[3],
			info->bdaddr[2],info->bdaddr[1],info->bdaddr[0],
			info->name,
			info->dev_class[2],info->dev_class[1],info->dev_class[0],
			info->clock_offset);
		if (info->rssi< 0x80)
			printf("%d dBm\n", (int)info->rssi);
		else
			printf("%d dBm\n", (int)info->rssi - 256);
		break;
	default:
		break;
	}

/*
	printf("\nInquiry a devices. \n");

	printf("Inquiry Event Type = %d\n", info->EventMode);
	printf("BD_ADDR = 0x %02X:%02X:%02X:%02X:%02X:%02X\n",
		info->BD_ADDR[5],info->BD_ADDR[4],info->BD_ADDR[3],
		info->BD_ADDR[2],info->BD_ADDR[1],info->BD_ADDR[0]);
	printf("Class_of_Device = 0x%02X %02X %02X\n", info->Class_of_Device[2],
		info->Class_of_Device[1],info->Class_of_Device[0]);
	if (info->EventMode > BT_INQIRY_MODE_DEFAULT) {
		if (info->RSSI < 0x80)
			printf("RSSI = %d dBm\n", (int)info->RSSI);
		else
			printf("RSSI = %d dBm\n", (int)info->RSSI - 256);
	}
	if (info->EventMode == BT_INQIRY_MODE_EXTENDED)
		printf("Device Name[%d] = %s\n", info->Name_Type, info->Name);
	printf("\n\n");
*/
	return 0;
}

int cb_InqiryComplete(void)
{
	printf("Inquiry finish.\n");
	return 0;
}

static void cmd_inquiry(int argc, char **argv)
{
	tinquiry_param inq_param ;
	inq_param.timeout = 16;
	inq_param.dev_num = 8;
	inq_param.mode = 2;

	switch(argc) {
		case 0:
			break;
		case 2:
			inq_param.timeout = strtoul(argv[1], NULL, 16);
			inq_param.dev_num = strtoul(argv[2], NULL, 16);
			break;
		case 3:
			inq_param.timeout = strtoul(argv[1], NULL, 16);
			inq_param.dev_num = strtoul(argv[2], NULL, 16);
			inq_param.mode    = strtoul(argv[3], NULL, 16);
		default:
			printf("Usage   : %s inq								Inquiry\n");
			printf("          %s inq	[timeout dev_num]        	Inquiry\n");
			printf("          %s inq	[timeout dev_num] <mode>	Inquiry\n");
			printf("Default : %s inq 	16 8 2                      Inquiry\n");
			break;
	}

//	printf("Inquiry parameter: timeout=%d, dev_num=%d, mode=%d\n",
//		inq_param.timeout,inq_param.dev_num,inq_param.mode);

	printf("Inquiring  ... \n");


	if (bt_inquiry(inq_param, cb_InqiryStatus, cb_InqiryComplete) < 0) {
		fprintf(stderr, "Send inquiry failed\n");
		return;
	}
	printf("Inqiry finish. \n");

	return;

/*
	tinqiry_param inq_param ;
	inq_param.LAP[0]=0x33;
	inq_param.LAP[1]=0x8B;
	inq_param.LAP[2]=0x9E;
	inq_param.Inquiry_Length = 0x10;	//inqiry time = 16*1.28 = 20s
	inq_param.Num_Responses = 8;		//max devices = 8;

	if (bt_inqiry(inq_param, cb_InqiryStatus, cb_InqiryComplete) < 0) {
		fprintf(stderr, "Send inqiry failed\n");
		return;
	}
	printf("Inqiry finish. \n");

	return;
	*/
}

static void cmd_tx(int argc, char **argv)
{
	if (bt_vendor_tx_continuous() < 0) {
		fprintf(stderr, "Tx continuous failed\n");
		return;
	}
	printf("Tx continumous started. \n");
	printf("Press any key to exit.\n");

	getchar();

	printf("Tx continuous stopped. \n");

	return;
}



static void cmd_hcicmd(int argc, char **argv)
{
	int i;
	uint16_t ogf;
	uint16_t ocf;
	uint8_t plen;
	void *param;
	unsigned char buf[MAX_EVENT_SIZE];

	memset(buf,0,MAX_EVENT_SIZE);
	if (argc < 2) {
		fprintf(stderr, "At least 2 parameters.\n");
		printf("Usage : bt_test cmd [ogf] [ocf] <param1> ... <paramN>\n");
		return;
	}
	ogf = strtoul(argv[1], NULL, 16);
	ocf = strtoul(argv[2], NULL, 16);
	plen = argc - 2;
	for (i=0; i<plen; i++)
		buf[i] = strtoul(argv[i+3], NULL, 16);

	printf("Send HCI cmd OGF=0x%02X OCF=0x%04X param_len=%d params=", ogf, ocf, plen);
	for (i=0; i<plen; i++)
		printf(" %02X", buf[i]);
	printf("\n");

	if (ogf == 0x3F) {
		if (bt_hcicmd_vendor(ogf, ocf, plen, buf) < 0) {
			fprintf(stderr, "Send HCI cmd OGF=0x%02X OCF=0x%04X failed\n", ogf, ocf);
			return;
		}
	} else {
		if (bt_hcicmd(ogf, ocf, plen, buf) < 0) {
			fprintf(stderr, "Send HCI cmd OGF=0x%02X OCF=0x%04X failed\n", ogf, ocf);
			return;
		}
	}
	printf("Send HCI cmd OGF=0x%02X OCF=0x%04X finish\n", ogf, ocf);

	return;
}



static void cmd_host_name(int argc, char **argv)
{
	int i;
	int len;
	unsigned char name[MAX_BT_NAME_LEN];

	memset(name,0,MAX_BT_NAME_LEN);


	switch(argc) {
		case 0:
			//get name
			len = bt_get_host_name(name, MAX_BT_NAME_LEN);
			if (len < 0) {
				fprintf(stderr, "Get Local host name failed\n");
			} else {
				printf("%s\n",name);
			}
			break;
		case 1:
			//set name
			len = strlen(argv[1]);
			if (len >= MAX_BT_NAME_LEN) {
				fprintf(stderr, "Local host name must small than %d, current len is %d\n",
					MAX_BT_NAME_LEN, len);
				break;
			}
			strcpy(name, argv[1]);
			printf("Set local host name to %s\n", name);
			len = bt_set_host_name(name);
			if (len < 0) {
				fprintf(stderr, "Set Local host name failed\n");
			} else {
//				printf("Set Local host name success, len=%d, name=%s\n",
//					len, name);
			}
			break;
		default:
			printf("Usage : %s name				Get locat name\n",PROG_NAME);
			printf("Usage : %s name	[name]		Set locat name\n",PROG_NAME);
			break;
	}


	return;
}


static void cmd_host_mac(int argc, char **argv)
{
	int i, ret;
	unsigned char mac[6];

	memset(mac,0,6);


	switch(argc) {
		case 0:
			//get mac
			ret = bt_get_mac(mac);
			if (ret < 0) {
				fprintf(stderr, "Get Local host mac failed\n");
			} else {
//				printf("Read bt mac finish. \n");
				printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
					mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			}
			break;
		case 6:
			//set mac

			for (i=0; i<6; i++)
				mac[i] = strtoul(argv[i+1], NULL, 16);
//			printf("Set BT MAC to : %02X:%02X:%02X:%02X:%02X:%02X\n",
//				mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

			if (bt_set_mac(mac) < 0) {
				fprintf(stderr, "Set bt mac failed\n");
				return;
			}

			break;
		default:
			printf("Usage : %s mac						Get locat MAC\n",PROG_NAME);
			printf("Usage : %s mac	[mac1] ... [mac6]	Set locat MAC\n",PROG_NAME);
			break;
	}


	return;
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#define EXT_PARSER_PORT 4330

static pthread_t thread_id;
static int ext_parser_fd = -1;




static int parser_connect(int port)
{
    socklen_t           clilen;
    struct sockaddr_in  cliaddr, servaddr;
    int s, srvlen;
    int n = 1;
    int size_n;
    int result = 0;
	int sockfd;

    printf("Connecting to port %d\n", port);

    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
		return -1;
	}


    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port);

    srvlen = sizeof(servaddr);

    if( inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0){
		printf("inet_pton error for %s\n","127.0.0.1");
        return -1;
	}

    if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
		printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
        return -1;
	}


    return sockfd;
}

static void parser_thread(void* param)
{
    int fd;

    fd = parser_connect(EXT_PARSER_PORT);
	if (fd < 0)
		return;
    ext_parser_fd = fd;


	printf("Start receive. fd = %d\n",ext_parser_fd);

	uint8_t buf[1024];
	int n;
	int i;
	while (1) {
		if (ext_parser_fd < 0) {
			printf("Stop read from socket\n");
			return;
		}
		n = read(ext_parser_fd, buf, 1024);
		if (n > 0) {
			printf("Rec data = ");
			for (i=0; i<n; i++)
				printf("%02X ", buf[i]);
			printf("\n");
		}
	}
}





static void cmd_hcidump(int argc, char **argv)
{
	int c;

	printf("hcidump  test \n");
	/* always setup ext listener port */
	if (pthread_create(&thread_id, NULL,(void*)parser_thread,NULL)!=0){
		perror("pthread_create failed, exit hcidump\n");
	}

	printf("Begin hcidump, input q to exit\n");
	do{
		c = getchar();
		if (c == 'q') {
			printf("Exit hcidump\n");
			if (ext_parser_fd > 0)
				close(ext_parser_fd);
			ext_parser_fd = -1;

			return;
		}
	}while(1);
}
static void usage(void);

static void dousage(int argc, char **argv)
{
	usage();
}

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
////////////////////////////////////////////


static struct {
	char *cmd;
	char *cmd_option;
	void (*func)(int argc, char **argv);
	char *doc;
} command[] = {

//	{ "rba",		"", 	cmd_rba,    	"Read BD Address"},
//	{ "wba",		"",		cmd_wba,   	 	"Write BD Address"},
	{ "on",			"", 		cmd_on,				"Enable bluetooth"},
	{ "off",		"", 		cmd_off,			"Disable bluetooth"},

	{ "reset",		"",   		cmd_reset,			"Reset Target"},

	{ "name",		"[name]",   cmd_host_name,		"Get/Set local name"},
	{ "mac",		"[mac]",   	cmd_host_mac,		"Get/Set local MAC"},

	{ "piscan",		"",   		cmd_piscan,			"Enable Page and Inquiry scan"},
	{ "noscan",		"",   		cmd_noscan,			"Disable scan"},
	{ "iscan",		"",   		cmd_iscan,			"Enable Inquiry scan"},
	{ "pscan",		"",   		cmd_pscan,			"Enable Page scan"},


	{ "dut",		"",  		cmd_dut,			"DUT mode test"},
	{ "inq",		"[timeout dev_num] <mode>",  		cmd_inquiry,			"Start BT Inqiry. Default inquiry 16 8 2"},

	{ "tx",			"",  		cmd_tx,				"Start BT Tx continuous"},

	{ "cmd",		"",  		cmd_hcicmd,			"Send hci cmd"},

	{ "hcidump",	"",  		cmd_hcidump,		"Capture hcidump"},


	{ "duton",		"",  		cmd_duton,			"Enter DUT mode"},
	{ "dutoff",		"",  		cmd_dutoff,			"Exit DUT mode"},



	{ "help",		"", 		dousage,				"Help"},

#if 0
	{ "reset","      ",   cmd_reset,    "Reset Target"                },
	{ "rba","       ",  cmd_rba,    "Read BD Address"                },
	{ "wba","<bdaddr> ",   cmd_wba,    "Write BD Address"                },
	{ "edutm","       ",  cmd_edutm,    "Enter DUT Mode"                },
	{ "wsm","<mode>  ",   cmd_wsm,    "Write Scan Mode"                },
	{ "mb","       ",   cmd_mb,    "Enter Master Blaster Mode"                },
	{ "mbr","<address> <length>  ",   cmd_mbr,    "Block memory read"                },
	{ "peek","<address> <width>  ",   cmd_peek,    "Read Value of an Address"                },
	{ "poke","<address> <value> <mask> <width>  ",   cmd_poke,    "Write Value to an Address"                },
	{ "cwtx","<channel number> ",   cmd_cwtx,    "Enter Continuous wave Tx"                },
	{ "cwrx","<channel number> ",   cmd_cwrx,    "Enter Continuous wave Rx"                },
	{ "rpst","<length> <id>  ",   cmd_rpst,    "Read PS Tag"                },
	{ "wpst","<length> <id> <data> ",   cmd_wpst,    "Write PS Tag"                },
	{ "psr","       ",   cmd_psr,    "PS Reset"                },
	{ "setap","<storage medium> <priority>",   cmd_setap,    "Set Access Priority"                },
	{ "setam","<storage medium> <access mode>",   cmd_setam,    "Set Access Mode"               },
	{ "rpsraw","<offset> <length>  ",   cmd_rpsraw,    "Read Raw PS"                },
	{ "wpsraw","<offset> <length>  <data>",   cmd_wpsraw,    "Write Raw PS"                },
	{ "ssm","<disable|enable>         ", cmd_ssm, "Set Sleep Mode"      },
	{ "dtx","         ", cmd_dtx, "Disable TX"      },
	{ "dump","<option>         ", cmd_dump, "Display Host Controller Information"      },
	{ "rafh","<connection handle>         ", cmd_rafh, "Read AFH channel Map"      },
	{ "safh","<channel classification>         ", cmd_safh, "Set AFH Host Channel Classification"      },
	{ "wotp", "<address> <data> [length=1]", cmd_wotp, "Write Length (default 1) bytes of Data to OTP started at Address"      },
	{ "rotp", "<address> [length=1]", cmd_rotp, "Read Length (default 1) bytes of Data to OTP started at Address"	},
	{ "otp", "[dump|imp|exp|test|rpid|wpid|rvid|wvid|rba|wba|hid|cpw|pwridx|ledo] [file]; opt wba <BdAddr>",
		cmd_otp, "Misc OTP operation: dump/import otp content; imp file content into otp; test otp; otp wba <BdAddr>"	},
	{ "plb", "[1|0]", cmd_plb, "Enable/disable PCM CODEC loopback"	},
	{ "psw", "[1|0] [Frequency]", cmd_psw, "Enable/disable PCM sine wave playback at frequency (0..3700)"	},
	{ "lert", "<rx_channel>", cmd_lert, "Put unit in LE RX mode at rx_channel (0..39)"	},
	{ "lett", "<tx_channel> <length> <packet_payload>", cmd_lett, "Put unit in LE TX mode at tx_channel (0..39) with packet of given length (0..37) and packet_payload"	},
	{ "lete", "        ", cmd_lete, "End LE test"	},
	{ "tput-s", "[BD_Addr] [Judgment value] Logfile times data_size", cmd_tputs, "Throughput test - sender side"	},
	{ "tput-r", "        ", cmd_tputr, "Throughput test - receiver side"	},
	{ "btagent","<port number>", cmd_btagent, "BT Agent for IQFact" },
	{ "pinconntest", "        ", cmd_pinconntest, "Pin Connectivity Test"	},
	{ "hciinq", "        ", cmd_hciinq, "Inquiry start"	},
	{ "hciinqcnl", "        ", cmd_hciinqcnl, "Inquiry Cancel"	},
	{ "hcisetevtflt", "        ", cmd_hcisetevtflt, "Set Event Filter"	},
	{ "conn", "        ", cmd_createconnection, "ACL Connection Test" },
	{ "venspeccmd", "  ", cmd_venspeccmd, "Vendor Specific Command"	},
	{ "disc", "  ", cmd_disc, "HCI disconnect Command"	},
	{ "hciinvcmd1", "        ", cmd_hciinvcmd1, "Invalid HCI Command"	},
	{ "rawcmd", "    ", cmd_rawcmd, "RAW HCI Command ex) rawcmd ogf ocf <bytes>"	},
#endif
	{ NULL, NULL, NULL, NULL }
};
/*
	{ "get_id",   cmd_gid,    "Get Chip Identification Number"                },
*/
static void usage(void)
{
	int i;

	printf("btconfig - BTCONFIG Tool ver %s\n", BT_TOOL_VERSION);
	printf("Usage:\n");
	printf("Commands:\n");
	for (i = 0; command[i].cmd; i++)
		printf("\t%-8s %-20s\t%s\n", command[i].cmd,command[i].cmd_option,command[i].doc);
	printf("\n");
}

static struct option main_options[] = {
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};


int main(int argc, char *argv[])
{

	int opt, i, ret;
	int fd = -1;
	char prop[30];

/*
	while ((opt=getopt_long(argc, argv, "+i:h", main_options, NULL)) != -1) {
		switch (opt) {
		case 'h':
		default:
			usage();
			exit(0);
		}
	}

	argc -= optind;
	argv += optind;
	optind = 0;
	*/



	argv +=1;
	argc -=1;

	if (argc == 0) {
		get_host_info();
		return 0;
	}
/*
	if (bt_enable() < 0) {
		fprintf(stderr, "Open bluetooth failed, exit\n");
		return 0;
	}
*/

	for (i = 0; command[i].cmd; i++) {
		if (strcmp(command[i].cmd, argv[0]))
			continue;
		command[i].func(argc-1, argv++);
		break;
	}

//	bt_disable();
	return 0;


}


