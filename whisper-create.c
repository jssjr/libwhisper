#include "whisper.h"

#include <getopt.h>

/* XXX: Implement:
 * - json formatting
 * - pretty printed dates
 */

int main(int argc, char **argv) {

  static struct option longopts[] = {
    { "version", no_argument, NULL, 'v' },
    { NULL, 0,  NULL, 0 }
  };
  char *path;

  int opt;

  while ((opt = getopt_long(argc, argv, "v", longopts, NULL)) != -1) {
    switch (opt) {
      case 'v':
        printf("whisper-fetch %s\n", GIT_SHA);
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

  return wsp_create(path, NULL, 1);
}
