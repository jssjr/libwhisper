#ifndef __LIBWHISPER__
#define __LIBWHISPER__

#include <stdio.h>
#include <stdint.h>

struct wsp_header {
  long aggregation_type;
  long max_retention;
  float xff;
  long archive_count;
};

struct wsp_archive_info {
  long offset;
  long seconds_per_point;
  long points;
};

int wsp_create() {
  return 0;
}

int wsp_info(FILE *fd) {
  fpos_t original_offset;
  struct wsp_header header;
  struct wsp_archive_info archive_info;
  int r;
  uint8_t buf[16];
  uint32_t temp;

  fgetpos(fd, &original_offset); // XXX: Handle EBADF/EINVAL
  rewind(fd);

  printf("parsing header\n");
  if (fread(&buf, sizeof(uint8_t), 16, fd) <= 0) {
    return -1;
  }

  header.aggregation_type = (buf[3]<<0) | (buf[2]<<8) | (buf[1]<<16) | (buf[0]<<24);
  header.max_retention = (buf[7]<<0) | (buf[6]<<8) | (buf[5]<<16) | (buf[4]<<24);
  temp = (buf[11]<<0) | (buf[10]<<8) | (buf[9]<<16) | (buf[8]<<24);
  header.xff = *(float*)&temp;
  header.archive_count = (buf[15]<<0) | (buf[14]<<8) | (buf[13]<<16) | (buf[12]<<24);

  printf("> aggregation type: %lu\n", header.aggregation_type);
  printf("> max retention:    %lu\n", header.max_retention);
  printf("> xfilesfactor:     %f\n",  header.xff);
  printf("> archive count:    %lu\n", header.archive_count);

  // XXX: assert valid header values here?
  if (header.archive_count <= 0) {
    return -1;
  }

  for (int i = 0 ; i < header.archive_count ; i++) {
    printf("parsing archive %d\n", i+1);
    if (fread(&buf, sizeof(uint8_t), 12, fd) <= 0) {
      return -1;
    }
    archive_info.offset = (buf[3]<<0) | (buf[2]<<8) | (buf[1]<<16) | (buf[0]<<24);
    archive_info.seconds_per_point = (buf[7]<<0) | (buf[6]<<8) | (buf[5]<<16) | (buf[4]<<24);
    archive_info.points = (buf[11]<<0) | (buf[10]<<8) | (buf[9]<<16) | (buf[8]<<24);

    printf("> offset:            %lu\n", archive_info.offset);
    printf("> seconds per point: %lu\n", archive_info.seconds_per_point);
    printf("> points:            %lu\n", archive_info.points);
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
  my_file = fopen("test/mem-free.wsp", "rb");
  wsp_info(my_file);
  fclose(my_file);
  return 0;
}

#endif
