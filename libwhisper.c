#ifndef __LIBWHISPER__
#define __LIBWHISPER__

#include <stdio.h>

struct wsp_header {
  long aggregation_type;
  long max_retention;
  float xff;
  long archive_count;
};

int wsp_create() {
  return 0;
}

int wsp_info(FILE *fd) {
  fpos_t original_offset;
  struct wsp_header header;
  int r;
  char buf[16];

  fgetpos(fd, &original_offset); // XXX: Handle EBADF/EINVAL
  rewind(fd);

  if (fread(&buf, sizeof(buf), 1, fd) <= 0) {
    return -1;
  }
  header.aggregation_type = (buf[3]<<0) | (buf[2]<<8) | (buf[1]<<16) | (buf[0]<<24);
  header.max_retention = (buf[7]<<0) | (buf[6]<<8) | (buf[5]<<16) | (buf[4]<<24);
  header.xff = (buf[11]<<0) | (buf[10]<<8) | (buf[9]<<16) | (buf[8]<<24);
  header.archive_count = (buf[15]<<0) | (buf[14]<<8) | (buf[13]<<16) | (buf[12]<<24);

  printf("read %d objects\n", r);
  printf("> aggregation type: %lu\n", header.aggregation_type);
  printf("> max retention:    %lu\n", header.max_retention);
  printf("> xfilesfactor:     %f\n", header.xff);
  printf("> archive count:    %lu\n", header.archive_count);

  return 0;
}

int wsp_update() {
  return 0;
}

int wsp_update_many() {
  return 0;
}

int wsp_fetch() {
  return 0;
}

int wsp_file_fetch() {
  return 0;
}

int wsp_aggregation_methods() {
  return 0;
}

int wsp_set_aggregation_method() {
  return 0;
}

int wsp_parse_retention_data() {
  return 0;
}

int wsp_validate_archive_list() {
  return 0;
}

// Main (tests)
int main(int argc, char **argv) {
  FILE *my_file;
  my_file = fopen("test/mem-free.wsp", "rb");
  wsp_info(my_file);
  return 0;
}

#endif
