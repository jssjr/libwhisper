#include "whisper.h"

#include <getopt.h>

/* XXX: Implement:
 * - json formatting
 * - pretty printed dates
 */

int main(int argc, char **argv) {

  struct wsp_archive ts;
  time_t from, until, t;
  short pretty, json;

  static struct option longopts[] = {
    { "from", required_argument, NULL, 'f' },
    { "until", required_argument, NULL, 'u' },
    { "pretty", no_argument, NULL, 'p' },
    { "json", no_argument, NULL, 'j' },
    { "version", no_argument, NULL, 'v' },
    { NULL, 0,  NULL, 0 }
  };
  char *path;
  char time_str[30];

  int opt, pos;

  from = 0;
  time(&until);
  pretty = 0;
  json = 0;

  while ((opt = getopt_long(argc, argv, "f:u:pjv", longopts, NULL)) != -1) {
    switch (opt) {
      case 'f':
        from = atoi(optarg);
        break;
      case 'u':
        until = atoi(optarg);
        break;
      case 'p':
        pretty = 1;
        break;
      case 'j':
        json = 1;
        break;
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

  wsp_fetch(path, from, until, &ts);
  for (pos = ts.from ; pos < ts.until ; pos += ts.step) {
    if (json) {
    } else if (pretty) {
      t = pos;
      strftime(time_str, 100, "%c", localtime(&t));
      printf("%s\t%f\n", time_str, ts.values[(pos-ts.from)/ts.step]);
    } else {
      printf("%d\t%f\n", (int)pos, ts.values[(pos-ts.from)/ts.step]);
    }
  }
  free(ts.values);

  return 0;
}
