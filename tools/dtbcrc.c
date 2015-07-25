/*
 * (C) Copyright 2001
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef __ASSEMBLY__
#define	__ASSEMBLY__			/* Dirty trick to get only #defines	*/
#endif
#define	__ASM_STUB_PROCESSOR_H__	/* don't include asm/processor.		*/
#include <config.h>
#undef	__ASSEMBLY__

#define ALIGN(x,a)      (((x)+(a)-1UL)&~((a)-1UL))

extern uint32_t crc32 (uint32_t, const unsigned char *, unsigned int);

#define MAGIC_DTB_CODE 0xD1B0C0DE
int main (int argc, char **argv)
{
	uint32_t crc[2] = {MAGIC_DTB_CODE, 0};
	int32_t ret;
	size_t size;
	struct stat s;
	char *buf;
	FILE *fp;

	ret = stat(argv[1], &s);
	if (ret < 0) {
		printf("file (%s) fail to get size\n", argv[1]);
		return -1;
	}
	size = ALIGN(s.st_size, 32);
	fp = fopen(argv[1], "rb+");
	if (!fp) {
		printf("file (%s) fail to open\n", argv[1]);
		return -1;
	}

	buf = calloc(1, size + 8);
	if (!buf) {
		printf("file (%s) fail to alloc memory\n", argv[1]);
		fclose(fp);
		return -1;
	}
	fread(buf, 1, size, fp);
	fclose(fp);

	crc[1] = crc32 (0, buf, size);
	memcpy(buf + size, crc, 8);

	fp = fopen(argv[2], "wb+");
	fwrite(buf, 1, size + 8, fp);
	fclose(fp);
	printf ("%X\n", crc[1]);
	return EXIT_SUCCESS;
}
