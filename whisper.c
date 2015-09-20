#include "whisper.h"

// Main (tests)
int main(int argc, char **argv) {
  FILE *my_file;
  struct wsp_header header;
  char *filename = "test/mem-free.wsp";

  printf("whisper info for %s\n", filename);
  my_file = fopen(filename, "rb");

  if (wsp_info(my_file, &header) == -1) {
    return -1;
  }

  printf("> aggregation type: %lu\n", header.aggregation_type);
  printf("> max retention:    %lu\n", header.max_retention);
  printf("> xfilesfactor:     %f\n",  header.xff);
  printf("> archive count:    %lu\n", header.archive_count);

  for (int i=0 ; i < header.archive_count ; i++) {
    printf("> offset:            %lu\n", header.archives[i].offset);
    printf("> seconds per point: %lu\n", header.archives[i].seconds_per_point);
    printf("> points:            %lu\n", header.archives[i].points);
  }

  fclose(my_file);
  return 0;
}
