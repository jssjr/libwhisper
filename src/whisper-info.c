#include "whisper.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>

int main(int argc, char **argv) {

  FILE *fd;
  struct wsp_header header;
  int file_size;

  static const char *aggregation_types[] = { "", "average", "sum", "last", "max", "min" };

  static struct option longopts[] = {
    { "version", no_argument, NULL, 'v' },
    { NULL, 0,  NULL, 0 }
  };
  char *path;

  struct stat buf;
  int i, opt;

  while ((opt = getopt_long(argc, argv, "v", longopts, NULL)) != -1) {
    switch (opt) {
      case 'v':
        printf("whisper-info %s\n", GIT_SHA);
        return 0;
      default:
        printf("Usage: %s [--version] path\n", argv[0]);
        return 1;
    }
  }

  if (optind >= argc) {
    printf("Usage: %s [--version] path\n", argv[0]);
    return 1;
  }

  path = argv[optind];
  optind++;

  if ((fd = fopen(path, "rb")) == NULL) {
    perror(NULL);
    return 1;
  }

  if (wsp_info(fd, &header) == -1) {
    fprintf(stderr, "Corrupt whisper file\n");
    fclose(fd);
    return 1;
  }

  fstat(fileno(fd), &buf);
  file_size = buf.st_size;
  fclose(fd);

  printf("maxRetention: %lu\n", header.max_retention);
  printf("xFilesFactor: %f\n", header.xff);
  printf("aggregationMethod: %s\n", aggregation_types[header.aggregation_type]);
  printf("fileSize: %d\n\n", file_size);

  for (i=0 ; i < header.archive_count ; i++) {
    printf("Archive %d\n", i);
    printf("retention: %lu\n", header.archives[i].retention);
    printf("secondsPerPoint: %lu\n", header.archives[i].seconds_per_point);
    printf("points: %lu\n", header.archives[i].points);
    printf("size: %lu\n", header.archives[i].size);
    printf("offset: %lu\n\n", header.archives[i].offset);
  }

  return 0;
}
