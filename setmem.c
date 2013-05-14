/* ----------------------------------------------------------------------- *
 *   
 *   Copyright 2005 H. Peter Anvin - All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 53 Temple Place Ste 330,
 *   Boston MA 02111-1307, USA; either version 2 of the License, or
 *   (at your option) any later version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */

/*
 * setmem.c
 *
 * Utility to write to I/O memory
 */

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <inttypes.h>
#include <sysexits.h>
#include <sys/types.h>
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
	{0, 0, 0, 0}
};

const char *program;

void usage(void)
{
	fprintf(stderr, "Usage: %s [options] address value...\n"
		"  --help         -h  Print this help\n"
		"  --version      -V  Print current version\n"
		"  --quad         -q  Write quadword (qword)\n"
		"  --long         -l  Write longword (dword)\n"
		"  --dword        -d  Write longword (dword)\n"
		"  --short        -s  Write shortword (word)\n"
		"  --word         -w  Write shortword (word)\n"
		"  --byte         -b  Write byte (default)\n", program);
}

int main(int argc, char *argv[])
{
	uintptr_t reg, page_mask, start, len;
	uint64_t data;
	int fd;
	int c;
	int size = 1;
	void *map, *ptr;

	program = argv[0];

	while ((c =
		getopt_long(argc, argv, "hVqldswb", long_options,
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
		default:
			usage();
			exit(127);
		}
	}

	if (argc - optind < 1) {
		/* Should have at least two arguments */
		usage();
		exit(127);
	}

	reg = (uintptr_t) strtoumax(argv[optind++], NULL, 0);

	fd = open("/dev/mem", O_RDWR);
	if (fd < 0) {
		perror("setmem:/dev/mem");
		exit(EX_OSFILE);
	}

	page_mask = ~((uintptr_t) getpagesize() - 1);
	start = reg & page_mask;
	len = ((reg + size * (argc - optind) + ~page_mask) & page_mask) - start;

	map = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, start);
	if (map == MAP_FAILED) {
		perror("setmem:mmap");
		exit(EX_OSERR);
	}

	ptr = (char *)map + (reg & ~page_mask);

	while (optind < argc) {
		data = (uint64_t) strtoumax(argv[optind++], NULL, 0);

		switch (size) {
		case 1:
			*(volatile uint8_t *)ptr = data;
			break;
		case 2:
			*(volatile uint16_t *)ptr = data;
			break;
		case 4:
			*(volatile uint32_t *)ptr = data;
			break;
		case 8:
			*(volatile uint64_t *)ptr = data;
			break;
		default:
			abort();	/* Should never happen */
		}

		ptr = (char *)ptr + size;
	}

	close(fd);

	exit(0);
}
