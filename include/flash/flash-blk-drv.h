#ifndef __FLASH_BLK_DRV_H__
#define __FLASH_BLK_DRV_H__
#if 0
#include "ucos/spinlock.h"

struct act_blk_dev
{
    struct act_blk_ops *actr;
    struct gendisk *gd;             /* The gendisk structure */
    int devnum;
    unsigned long size;
    unsigned long off_size;
};

struct act_blk_ops
{
    /* blk device ID */
    char *name;
    unsigned int major;
    int minorbits;

    /* add/remove nandflash devparts,use gendisk */
    int (*add_dev)(struct act_blk_ops *actr, int part_num);
    int (*remove_dev)(struct act_blk_dev *dev);

    /* Block layer ioctls */
    int (*flush)(struct act_blk_dev *dev);

    /* Called with mtd_table_mutex held; no race with add/remove */
    int (*open)(struct act_blk_dev *dev);
    int (*release)(struct act_blk_dev *dev);

    /* synchronization variable */
    struct request_queue *rq;
    spinlock_t queue_lock;

    struct module *owner;
};
#endif
#endif
