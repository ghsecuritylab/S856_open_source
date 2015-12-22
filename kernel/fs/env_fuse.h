// porting by liuxu7@lenovo.com, for limit size, begin
#ifndef __FS_ENV_FUSE_H
#define __FS_ENV_FUSE_H

#define LIMIT_DATA_SIZE
#define LIMIT_SDCARD_SIZE
#ifdef LIMIT_SDCARD_SIZE
#define DATA_FREE_SIZE_TH_DEFAULT (50*1024*1024)  // 50MB fuse free for data
#endif
extern long long data_free_size_th;
#define CHECK_1TH  (10 * 1024 * 1024)
#define CHECK_2TH  (5 * 1024 * 1024)
extern long long store;
//EXPORT_SYMBOL(data_free_size_th);

#endif
// porting by liuxu7@lenovo.com, for limit size, end
