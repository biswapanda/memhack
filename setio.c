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
#include <sys/types.h>
#include <sys/io.h>

#include "version.h"

struct option long_options[] = {
	{"help", 	0, 0, 'h'},
	{"version", 	0, 0, 'V'},
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
		"  --long         -l  Write longword (dword)\n"
		"  --dword        -d  Write longword (dword)\n"
		"  --short        -s  Write shortword (word)\n"
		"  --word         -w  Write shortword (word)\n"
		"  --byte         -b  Write byte (default)\n", program);
}

int main(int argc, char *argv[])
{
	uintptr_t reg;
	uint64_t data;
	int c;
	int size = 1;

	program = argv[0];

	while ((c =
		getopt_long(argc, argv, "hVldswb", long_options, NULL)) != -1) {
		switch (c) {
		case 'h':
			usage();
			exit(0);
		case 'V':
			fprintf(stderr, "%s: version %s\n", program,
				VERSION_STRING);
			exit(0);
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

	if (optind > argc - 2) {
		/* Should have at least two arguments */
		usage();
		exit(127);
	}

	iopl(3);

	reg = (uintptr_t) strtoumax(argv[optind++], NULL, 0);

	while (optind < argc) {
		data = (uint64_t) strtoumax(argv[optind++], NULL, 0);

		switch (size) {
		case 1:
			outb(data, reg);
			break;
		case 2:
			outw(data, reg);
			break;
		case 4:
			outl(data, reg);
			break;
		default:
			abort();	/* Should never happen */
		}
	}

	exit(0);
}
