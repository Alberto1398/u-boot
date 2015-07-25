/*
 * Copyright 2013 Actions Semi Inc.
 * Author: Actions Semi, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#ifndef QUICK_BOOT_H
#define QUICK_BOOT_H

extern u32 crc32_le(u32 crc, u8 const *p, size_t len);
extern void secondary_startup(void);
extern u32 stack_pointer[];
extern u32 pg_dir;
extern u32 FTL_Read(u32 lba, u32 len, void *buf);

extern unsigned char *get_quickboot_logo(void);
extern int check_snapshot_image(void);
extern void quickboot_init(void);
extern int do_bootsnapshot(void);

#endif
