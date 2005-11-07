#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  uintptr_t start, mapstart, len, maplen;
  char *mem, *buffer;
  int fd;

  if ( argc != 3 ) {
    fprintf(stderr, "Usage: %s start bytes\n", argv[0]);
    exit(1);
  }

  fd = open("/dev/mem", O_RDONLY);
  if ( fd < 0 ) {
    perror("/dev/mem");
    exit(1);
  }

  start = (uintptr_t)strtoumax(argv[1], NULL, 0);
  len   = (uintptr_t)strtoumax(argv[2], NULL, 0);

  buffer = malloc(len);
  if ( !buffer ) {
    perror("malloc");
    exit(1);
  }

  mapstart = start & ~0xffff;
  maplen   = (len + (start-mapstart) + 0xffff) & ~0xffff;

  mem = mmap(NULL, maplen, PROT_READ, MAP_SHARED, fd, mapstart);
  if ( mem == MAP_FAILED ) {
    perror("mmap");
    exit(1);
  }

  memcpy(buffer, mem+(start-mapstart), len);

  fwrite(buffer, len, 1, stdout);

  return 0;
}
  
  
