#ifndef __LIBWHISPER__
#define __LIBWHISPER__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_ARCHIVES 8

struct wsp_archive_info {
  long offset;
  long seconds_per_point;
  long points;
};

struct wsp_header {
  long aggregation_type;
  long max_retention;
  float xff;
  long archive_count;
  struct wsp_archive_info archives[MAX_ARCHIVES];
};

int wsp_create() {
  return 0;
}

int wsp_info(FILE *fd, struct wsp_header *header) {
  fpos_t original_offset;
  struct wsp_archive_info *archive_info;
  uint8_t buf[16];
  uint32_t temp;

  if (fgetpos(fd, &original_offset) == -1) {
    // XXX: Handle EBADF/EINVAL
    return -1;
  }
  rewind(fd);

  if (fread(&buf, sizeof(uint8_t), 16, fd) == 0) {
    // XXX: Check if feof or ferror
    return -1;
  }

  header->aggregation_type = (buf[3]<<0) | (buf[2]<<8) | (buf[1]<<16) | (buf[0]<<24);
  header->max_retention = (buf[7]<<0) | (buf[6]<<8) | (buf[5]<<16) | (buf[4]<<24);
  temp = (buf[11]<<0) | (buf[10]<<8) | (buf[9]<<16) | (buf[8]<<24);
  header->xff = *(float*)&temp;
  header->archive_count = (buf[15]<<0) | (buf[14]<<8) | (buf[13]<<16) | (buf[12]<<24);
  // XXX: assert valid header values here?
  if (header->archive_count <= 0) {
    return -1;
  }

  for (int i = 0 ; i < header->archive_count ; i++) {
    if (fread(&buf, sizeof(uint8_t), 12, fd) <= 0) {
      return -1;
    }
    header->archives[i].offset = (buf[3]<<0) | (buf[2]<<8) | (buf[1]<<16) | (buf[0]<<24);
    header->archives[i].seconds_per_point = (buf[7]<<0) | (buf[6]<<8) | (buf[5]<<16) | (buf[4]<<24);
    header->archives[i].points = (buf[11]<<0) | (buf[10]<<8) | (buf[9]<<16) | (buf[8]<<24);
  }

  fsetpos(fd, &original_offset);

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

#endif
