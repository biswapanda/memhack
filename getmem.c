/* ----------------------------------------------------------------------- *
 *   
 *   Copyright 2005-2010 H. Peter Anvin - All Rights Reserved
 *   Copyright 2013 Intel Corporation; author: H. Peter Anvin
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 53 Temple Place Ste 330,
 *   Boston MA 02111-1307, USA; either version 2 of the License, or
 *   (at your option) any later version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */

#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "version.h"

struct option long_options[] = {
	{"help", 	0, 0, 'h'},
	{"version", 	0, 0, 'V'},
	{"quad", 	0, 0, 'q'},
	{"long", 	0, 0, 'l'},
	{"dword", 	0, 0, 'd'},
	{"short", 	0, 0, 's'},
	{"word", 	0, 0, 'w'},
	{"byte", 	0, 0, 'b'},
	{"memcpy", 	0, 0, 'm'},
	{0, 0, 0, 0}
};

const char *program;

void usage(void)
{
	fprintf(stderr, "Usage: %s [options] address bytes\n"
		"  --help         -h  Print this help\n"
		"  --version      -V  Print current version\n"
		"  --quad         -q  Read quadwords (qword)\n"
		"  --long         -l  Read longwords (dword)\n"
		"  --dword        -d  Read longwords (dword)\n"
		"  --short        -s  Read shortwords (word)\n"
		"  --word         -w  Read shortwords (word)\n"
		"  --byte         -b  Read bytes\n"
		"  --memcpy       -m  Use memcpy (default)\n", program);
}

int main(int argc, char *argv[])
{
	uintptr_t start, mapstart, len, left, maplen;
	uintptr_t page_mask;
	char *mem, *buffer, *dst;
	volatile char *ptr;
	int fd;
	int c;
	int size = 0;
	int exit_code = EX_OK;

	program = argv[0];

	while ((c =
		getopt_long(argc, argv, "hVqldswbm", long_options,
			    NULL)) != -1) {
		switch (c) {
		case 'h':
			usage();
			exit(0);
		case 'V':
			fprintf(stderr, "%s: version %s\n", program,
				VERSION_STRING);
			exit(0);
		case 'q':
			size = 8;
			break;
		case 'l':
		case 'd':
			size = 4;
			break;
		case 'w':
		case 's':
			size = 2;
			break;
		case 'b':
			size = 1;
			break;
		case 'm':
			size = 0;
			break;
		default:
			usage();
			exit(EX_USAGE);
		}
	}

	if (argc - optind < 1 || argc - optind > 2) {
		usage();
		exit(EX_USAGE);
	}

	start = (uintptr_t) strtoumax(argv[optind], NULL, 0);
	if (argc - optind >= 2)
		len = (uintptr_t) strtoumax(argv[optind + 1], NULL, 0);
	else if (size)
		len = size;
	else {
		usage();
		exit(EX_USAGE);
	}

	fd = open("/dev/mem", O_RDONLY);
	if (fd < 0) {
		perror("/dev/mem");
		exit_code = EX_OSFILE;
		goto exit_open_err;
	}

	page_mask = ~((uintptr_t) getpagesize() - 1);

	buffer = malloc(len);
	if (!buffer) {
		perror("malloc");
		exit_code = EX_OSERR;
		goto exit_malloc_err;
	}

	mapstart = start & page_mask;
	maplen = (len + (start - mapstart) + ~page_mask) & page_mask;

	mem = mmap(NULL, maplen, PROT_READ, MAP_SHARED, fd, mapstart);
	if (mem == MAP_FAILED) {
		perror("mmap");
		exit_code = EX_OSERR;
		goto exit_mmap_err;
	}

	ptr = mem + (start - mapstart);

	if (size == 0) {
		memcpy(buffer, (char *)ptr, len);
	} else {
		left = len;
		dst = buffer;

		while (left) {
			static const int next_lower_power_of_2[8] =
			    { 0, 1, 2, 2, 4, 4, 4, 4 };

			if (left < size)
				size = next_lower_power_of_2[left];

			switch (size) {
			case 1:
				*(uint8_t *) dst = *(volatile uint8_t *)ptr;
				break;
			case 2:
				*(uint16_t *) dst = *(volatile uint16_t *)ptr;
				break;
			case 4:
				*(uint32_t *) dst = *(volatile uint32_t *)ptr;
				break;
			case 8:
				*(uint64_t *) dst = *(volatile uint64_t *)ptr;
				break;
			default:
				abort();
			}
			dst += size;
			ptr += size;
			left -= size;
		}
	}

	fwrite(buffer, len, 1, stdout);

exit_mmap_err:
	free(buffer);
exit_malloc_err:
	close(fd);
exit_open_err:
	exit(exit_code);
}
