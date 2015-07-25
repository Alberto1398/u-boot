#include <linux/semaphore.h>

//#define GET_PARTITION_FROM_BREC

/* TV out must be run with interrupt enable mode. */

#define PART_OFFSET		1

#define BLK_ERR_MSG_ON
#ifdef  BLK_ERR_MSG_ON
#define dbg_err(fmt, args...) printk(fmt, ## args)
#else
#define dbg_err(fmt, ...)  ({})
#endif

#define REMAIN_SPACE	0
#define PART_FREE	0x55
#define PART_DUMMY	0xff
#define PART_READONLY	0x85
#define PART_WRITEONLY	0x86
#define PART_NO_ACCESS	0x87
#define PART_RESERVE	0x99


/****** sector is the unit *******/

#define BOOT_AREA_SIZE		(NandDevInfo[0].LogicOrganizePar->DataBlkNumPerZone*\
		NandDevInfo[0].LogicOrganizePar->ZoneNumPerDie*\
		NandDevInfo[0].LogicOrganizePar->PageNumPerLogicBlk*\
		NandDevInfo[0].LogicOrganizePar->SectorNumPerLogicPage)


#define PRINT_DEBUG(fmt, args...)	printk(KERN_EMERG fmt, ## args)

extern unsigned int part1_dev_num;
extern unsigned int part2_dev_num;

struct act_blk_ops;
struct list_head;
struct semaphore;
struct hd_geometry;

struct nand_partinfo {
	unsigned long partsize;
	unsigned long off_size;
	unsigned char type;
};
struct act_blk_dev {
	struct act_blk_ops *actr;
	struct list_head list;

	unsigned char heads;
	unsigned char sectors;
	unsigned short cylinders;

	int devnum;
	unsigned long size;
	unsigned long off_size;
	int readonly;
	int writeonly;
	int disable_access;
	void *blkcore_priv;
	struct task_struct *tsk;
	spinlock_t queue_lock;
	struct request_queue *rq;
	struct completion thread_exit;
};
struct act_blk_ops {
	/* blk device ID */
	char *name;
	int major;
	int minorbits;

	/* add/remove nandflash devparts,use gendisk */
	int (*add_dev)(struct act_blk_ops *actr, struct nand_partinfo *part);
	int (*remove_dev)(struct act_blk_dev *dev);

	/* Block layer ioctls */
	int (*getgeo)(struct act_blk_dev *dev, struct hd_geometry *geo);
	int (*flush)(struct act_blk_dev *dev);

	/* Called with mtd_table_mutex held; no race with add/remove */
	int (*open)(struct act_blk_dev *dev);
	int (*release)(struct act_blk_dev *dev);

	/* synchronization variable */
	struct completion thread_exit;
	int quit;
//	wait_queue_head_t thread_wq;
	struct task_struct *tsk;
	struct request_queue *rq;
	spinlock_t queue_lock;
	struct semaphore nand_mutex;

	struct list_head devs;
	struct module *owner;
};

