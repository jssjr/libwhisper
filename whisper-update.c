#include "whisper.h"

#include <getopt.h>

int main(int argc, char **argv) {
  int num_datapoints;
  int index;
  char *token;
  double timestamp;
  double value;
  struct wsp_datapoint *datapoints;

  static struct option longopts[] = {
    { "version", no_argument, NULL, 'v' },
    { NULL, 0,  NULL, 0 }
  };
  char *path;


  int opt;

  while ((opt = getopt_long(argc, argv, "v", longopts, NULL)) != -1) {
    switch (opt) {
      case 'v':
        printf("whisper-update %s\n", GIT_SHA);
        return 0;
      default:
        printf("Usage: %s [--version] path timestamp:value [timestamp:value]\n", argv[0]);
        return 1;
    }
  }

  if (optind >= argc) {
    printf("Usage: %s [--version] path timestamp:value [timestamp:value]\n", argv[0]);
    return 1;
  }

  path = argv[optind++];

  num_datapoints = argc - optind;
  if ((datapoints = calloc(num_datapoints, sizeof(struct wsp_datapoint))) == NULL) {
    return -1;
  }

  for (int i=0; optind < argc; i++) {
    index = optind++;

    token = strtok(argv[index], ":");
    if (sscanf(token, "%lf", &timestamp) <= 0) {
      printf("Error parsing token. `%s` is not a valid epoch timestamp\n", token);
      return 1;
    }

    token = strtok(NULL, ":");
    if (token == NULL) {
      printf("Paramters must be in the form `timestamp:value`. `%s` is not.\n", argv[index]);
      return 1;
    }

    if (sscanf(token, "%lf", &value) <= 0) {
      printf("Error parsing token. `%s` is not a valid value\n", token);
      return 1;
    }

    datapoints[i].timestamp = (time_t)timestamp;
    datapoints[i].value = value;
  }

  return wsp_update_many(path, datapoints, num_datapoints);
}
