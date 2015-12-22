/*
filename: le_rkm.c

this is the lenovo replay kernel message implement.

By RKM,the offline log in andorid can backup previous system logs,
such as kernel dmesg log, tz logs; it also can access current 
system lifetime's boot logs,such as sbl logs, lk logs, even tz logs.

Author:KerryXi
Date: Apr, 2014

Copyright Lenovo 2014
*/

#include <debug.h>
#include <printf.h>
#include <dev/uart.h>
#include <stdlib.h>
#include <string.h>
#include <libfdt.h>
#include <dev_tree.h>
#include <le_rkm.h>

static char* rkm_shadow_buf_addr=NULL;

#if WITH_DEBUG_LOG_BUF
static char* lk_log_buf_shadow_addr=NULL;
static char *cur_lk_log_addr=NULL;
static int cur_lk_log_len=0;

void rkm_init_lk_log_buf(char *addr,int len)
{
	cur_lk_log_addr = addr;
	cur_lk_log_len = len > (PAGE_SIZE*2) ? (PAGE_SIZE*2) : len;
}

void rkm_update_lk_log_shadow_buf()
{
	dprintf(SPEW,"readdy to fill lk_log_shadow_addr:0x%p\n",lk_log_buf_shadow_addr);
	memcpy(lk_log_buf_shadow_addr,cur_lk_log_addr,cur_lk_log_len);
}
#endif

static char *kernel_log_buf_shadow_addr=NULL;
static int cur_kernel_log_buf_len=0;
static rkm_log_buf_header_table_t kernel_log_buf_table;


int rkm_update_device_tree(void *fdt)
{
	int ret = 0;
	uint32_t offset;

	/* Get offset of the rkm_log node */
	ret = fdt_path_offset(fdt, "/rkm_log");
	if (ret < 0)
	{
		dprintf(CRITICAL, "Could not find rkm_log node.\n");
		return ret;
	}

	offset = ret;
#if WITH_DEBUG_LOG_BUF
	/* Adding the bootloader_log to the rkm_log node */
	dprintf(SPEW, "find rkm_log node:lk buf=0x%08x, size=%d \n",
			(uint32_t)lk_log_buf_shadow_addr,
			cur_lk_log_len);

	ret = fdt_setprop_u32(fdt, offset, (const char*)"lk,lk_log_start", (uint32_t)lk_log_buf_shadow_addr);
	if (ret)
	{
		dprintf(CRITICAL, "ERROR: Cannot update rkm_log node [lk,lk_log_start]\n");
		return ret;
	}

	ret = fdt_setprop_u32(fdt, offset, (const char*)"lk,lk_log_size", (uint32_t)cur_lk_log_len);
	if (ret)
	{
		dprintf(CRITICAL, "ERROR: Cannot update rkm_log node [lk,lk_log_size]\n");
		return ret;
	}

#endif
	dprintf(SPEW, " rkm_log node:kernel buf=0x%08x, size=%d \n",
			(uint32_t)kernel_log_buf_shadow_addr,
			cur_kernel_log_buf_len);
	/* Adding the last kmsg log to the rkm_log node */
	ret = fdt_setprop_u32(fdt, offset, (const char*)"kernel,log_buf_start", (uint32_t)kernel_log_buf_shadow_addr );
	if (ret)
	{
		dprintf(CRITICAL, "ERROR: Cannot update rkm_log node [kernel,log_buf_start]\n");
		return ret;
	}

	ret = fdt_setprop_u32(fdt, offset, (const char*)"kernel,log_buf_size", (uint32_t)cur_kernel_log_buf_len);
	if (ret)
	{
		dprintf(CRITICAL, "ERROR: Cannot update rkm_log node [kernel,log_buf_size]\n");
		return ret;
	}

	dprintf(CRITICAL, "rkm_log:lk buf size=%d, kernel buf size=%d\n",cur_lk_log_len, cur_kernel_log_buf_len);
	return ret;
}

static void rkm_dump_kernel_log_buf_head(rkm_log_buf_header_table_t *h)
{
	dprintf(SPEW,"rkm kernel log buf header:\n");
	dprintf(SPEW,"    magic1     = 0x%08x\n",h->magic1);
	dprintf(SPEW,"    magic2     = 0x%08x\n",h->magic2);
	dprintf(SPEW,"    version    = 0x%08x\n",h->version);
	dprintf(SPEW,"    bss_start  = 0x%08x\n",h->bss_start);
	dprintf(SPEW,"    bss_stop   = 0x%08x\n",h->bss_stop);
	dprintf(SPEW,"    log_buf_pa = 0x%08x\n",h->log_buf_pa);
	dprintf(SPEW,"    log_buf_len= 0x%08x\n",h->log_buf_len);
}


static inline int rkm_check_kernel_head(rkm_log_buf_header_table_t *h)
{
	if ((h->magic1 == RKM_LOG_BUF_HEADER_TABLE_MAGIC1)
	  && (h->magic2 == RKM_LOG_BUF_HEADER_TABLE_MAGIC2)
	  && (h->version == RKM_LOG_BUF_HEADER_TABLE_VERSION)
	  && (h->bss_start > RKM_SCAN_KERNEL_LOG_BUF_START_OFFSET)
	  && (h->bss_start < RKM_SCAN_KERNEL_LOG_BUF_END)
	  && (h->bss_stop > RKM_SCAN_KERNEL_LOG_BUF_START_OFFSET)
	  && (h->bss_stop < RKM_SCAN_KERNEL_LOG_BUF_END)
	  && (h->bss_start < h->bss_stop)
	  && (h->log_buf_pa > RKM_SCAN_KERNEL_LOG_BUF_START_OFFSET)
	  && (h->log_buf_pa < RKM_SCAN_KERNEL_LOG_BUF_END)
	  && (h->log_buf_len > 0)
	  && (h->log_buf_len < (MAX_RKM_LOG_BUF_LEN * PAGE_SIZE)))
		return 1;
	else
		return 0;

}

static void rkm_scan_kernel_log_buf()
{
	unsigned int *p;	
	rkm_log_buf_header_table_t *h=NULL;
	char *log_buf_ptr=NULL;
	int success=0;

	p = RKM_SCAN_KERNEL_LOG_BUF_START_OFFSET;
	while (p<RKM_SCAN_KERNEL_LOG_BUF_END) {
		if (*p == RKM_LOG_BUF_HEADER_TABLE_MAGIC1) {
			h = (rkm_log_buf_header_table_t *)p;
			if (rkm_check_kernel_head(h)) {
				dprintf(CRITICAL, "rkm find kernel log buf header at %p\n",h);
				success = 1;
				break;
			} else {
				dprintf(CRITICAL, "rkm:kernel log buf header error at %p\n",h);
				rkm_dump_kernel_log_buf_head(h);
			}
		}
		p += PAGE_SIZE / (sizeof(*p));
	}

	//init the size of data
	if (success) {
		log_buf_ptr = (char *)h->log_buf_pa;
		cur_kernel_log_buf_len = h->log_buf_len;
		memcpy(kernel_log_buf_shadow_addr,log_buf_ptr,cur_kernel_log_buf_len);
		dprintf(CRITICAL, "rkm backup kernel log buf from 0x%08x to 0x%08x,len=%d\n",
				log_buf_ptr, kernel_log_buf_shadow_addr,cur_kernel_log_buf_len);
	} else {
		cur_kernel_log_buf_len = 0;
		dprintf(CRITICAL,"lk can not scan rkm header\n");
	}
	
}

void rkm_int_base_address(char* addr)
{
	int base_addr=(int)(addr+DTB_PAD_SIZE);

	rkm_shadow_buf_addr = (char*)ROUNDUP(base_addr,PAGE_SIZE);
	kernel_log_buf_shadow_addr = rkm_shadow_buf_addr+RKM_LAST_KERNEL_LOG_BUF_OFFSET*PAGE_SIZE;


#if WITH_DEBUG_LOG_BUF
	lk_log_buf_shadow_addr = rkm_shadow_buf_addr+RKM_LK_BUF_LOG_OFFSET*PAGE_SIZE;
	//get the current lk log base address and length
	dprintf(CRITICAL,"fdt end:%p rkm shadow buf base:%p lk ptr:%p log_buf ptr:%p\n",
			addr, rkm_shadow_buf_addr,lk_log_buf_shadow_addr,kernel_log_buf_shadow_addr);
	lk_log_buf_init_rkm();
#endif

	//scan the system kernel log_buf magic value in memory
	rkm_scan_kernel_log_buf();
}

