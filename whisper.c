#ifndef __LIBWHISPER__
#define __LIBWHISPER__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define WSP_MAX_ARCHIVES 8
#define WSP_DATAPOINT_SIZE 12
#define WSP_HEADER_SIZE 16
#define WSP_ARCHIVE_INFO_SIZE 12

#include <endian.h>

#if __BYTE_ORDER == __BIG_ENDIAN
/* No translation needed for big endian system */
#define Swap2Bytes(val) val
#define Swap4Bytes(val) val
#define Swap8Bytes(val) val
#else
/* Swap 2 byte, 16 bit values: */

#define Swap2Bytes(val) \
 ( (((val) >> 8) & 0x00FF) | (((val) << 8) & 0xFF00) )

/* Swap 4 byte, 32 bit values: */

#define Swap4Bytes(val) \
 ( (((val) >> 24) & 0x000000FF) | (((val) >>  8) & 0x0000FF00) | \
   (((val) <<  8) & 0x00FF0000) | (((val) << 24) & 0xFF000000) )

/* Swap 8 byte, 64 bit values: */

#define Swap8Bytes(val) \
 ( (((val) >> 56) & 0x00000000000000FF) | (((val) >> 40) & 0x000000000000FF00) | \
   (((val) >> 24) & 0x0000000000FF0000) | (((val) >>  8) & 0x00000000FF000000) | \
   (((val) <<  8) & 0x000000FF00000000) | (((val) << 24) & 0x0000FF0000000000) | \
   (((val) << 40) & 0x00FF000000000000) | (((val) << 56) & 0xFF00000000000000) )
#endif

struct wsp_timeseries {
  time_t from;
  time_t until;
  time_t step;
  double *values;
};

struct wsp_archive_info {
  long offset;
  long seconds_per_point;
  long points;
  long retention;
  long size;
};

struct wsp_header {
  long aggregation_type;
  long max_retention;
  float xff;
  long archive_count;
  struct wsp_archive_info archives[WSP_MAX_ARCHIVES];
};

int wsp_create() {
  return 0;
}

int wsp_info(FILE *fd, struct wsp_header *header) {
  fpos_t original_offset;
  uint8_t buf[WSP_HEADER_SIZE];
  uint32_t temp;
  int i;

  if (fgetpos(fd, &original_offset) == -1) {
    /* XXX: Handle EBADF/EINVAL */
    return -1;
  }
  rewind(fd);

  if (fread(&buf, sizeof(uint8_t), WSP_HEADER_SIZE, fd) == 0) {
    /* XXX: Check if feof or ferror */
    return -1;
  }

  header->aggregation_type = (buf[3]<<0) | (buf[2]<<8) | (buf[1]<<16) | (buf[0]<<24);
  header->max_retention = (buf[7]<<0) | (buf[6]<<8) | (buf[5]<<16) | (buf[4]<<24);
  temp = (buf[11]<<0) | (buf[10]<<8) | (buf[9]<<16) | (buf[8]<<24);
  header->xff = *(float*)&temp;
  header->archive_count = (buf[15]<<0) | (buf[14]<<8) | (buf[13]<<16) | (buf[12]<<24);
  /* XXX: assert valid header values here? */
  if (header->archive_count <= 0) {
    return -1;
  }

  for (i = 0 ; i < header->archive_count ; i++) {
    if (fread(&buf, sizeof(uint8_t), WSP_ARCHIVE_INFO_SIZE, fd) <= 0) {
      return -1;
    }
    header->archives[i].offset = (buf[3]<<0) | (buf[2]<<8) | (buf[1]<<16) | (buf[0]<<24);
    header->archives[i].seconds_per_point = (buf[7]<<0) | (buf[6]<<8) | (buf[5]<<16) | (buf[4]<<24);
    header->archives[i].points = (buf[11]<<0) | (buf[10]<<8) | (buf[9]<<16) | (buf[8]<<24);
    header->archives[i].retention = header->archives[i].seconds_per_point * header->archives[i].points;
    header->archives[i].size = header->archives[i].points * WSP_DATAPOINT_SIZE;
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

int _wsp_fetch_archive(FILE *fd, struct wsp_archive_info *archive, time_t from, time_t until, struct wsp_timeseries *ts) {
  long from_interval, until_interval, base_interval, current_interval;
  long from_offset, until_offset;
  long step;
  long points, point_time;
  uint8_t datapoint_buf[WSP_DATAPOINT_SIZE];
  uint8_t buf[WSP_DATAPOINT_SIZE*4096];
  uint64_t temp;
  int i;
  long td, pd, bd;

  from_interval = (from - (from % archive->seconds_per_point)) + archive->seconds_per_point;
  until_interval = (until - (until % archive->seconds_per_point)) + archive->seconds_per_point;

  fseek(fd, archive->offset, SEEK_SET);
  fread(&datapoint_buf, sizeof(uint8_t), WSP_DATAPOINT_SIZE, fd);

  base_interval = (datapoint_buf[3]<<0) | (datapoint_buf[2]<<8) | (datapoint_buf[1]<<16) | (datapoint_buf[0]<<24);

  step = archive->seconds_per_point;
  points = (until_interval - from_interval) / step;

  ts->from = from_interval;
  ts->until = until_interval;
  ts->step = step;

  if ((ts->values = calloc(points, sizeof(double))) == NULL) {
    return -1;
  }

  /* skip further reads if the file has never been updated and set values to 0 (none?) */
  if (base_interval == 0) {
    return 0;
  }

  td = from_interval - base_interval;
  pd = td / archive->seconds_per_point;
  bd = pd * WSP_DATAPOINT_SIZE;
  from_offset = archive->offset + (bd % archive->size);

  td = until_interval - base_interval;
  pd = td / archive->seconds_per_point;
  bd = pd * WSP_DATAPOINT_SIZE;
  until_offset = archive->offset + (bd % archive->size);

  if (fseek(fd, from_offset, SEEK_SET) == -1) {
    return -1;
  }
  if (from_offset < until_offset) {
    /* we don't wrap around the archive */
    if ((fread(&buf, sizeof(uint8_t), until_offset - from_offset, fd)) == 0) {
      return -1;
    }
  } else {
    /* we wrap around the archive and need second read */
    fread(&buf, sizeof(uint8_t), ((archive->offset + archive->size) - from_offset), fd);
    if (fseek(fd, archive->offset, SEEK_SET) == -1) {
      return -1;
    }
    fread(&buf[(archive->offset + archive->size) - from_offset], sizeof(uint8_t), \
        until_offset - archive->offset, fd);
  }

  current_interval = from_interval;
  for (i=0;i<points;i++) {
    point_time = (buf[3+(WSP_DATAPOINT_SIZE*i)]<<0) | \
                 (buf[2+(WSP_DATAPOINT_SIZE*i)]<<8) | \
                 (buf[1+(WSP_DATAPOINT_SIZE*i)]<<16) | \
                 (buf[0+(WSP_DATAPOINT_SIZE*i)]<<24);
    if (point_time == current_interval) {
      temp = ((uint64_t)(buf[11+(WSP_DATAPOINT_SIZE*i)])<<0) | \
             ((uint64_t)(buf[10+(WSP_DATAPOINT_SIZE*i)])<<8) | \
             ((uint64_t)(buf[9+(WSP_DATAPOINT_SIZE*i)])<<16) | \
             ((uint64_t)(buf[8+(WSP_DATAPOINT_SIZE*i)])<<24) | \
             ((uint64_t)(buf[7+(WSP_DATAPOINT_SIZE*i)])<<32) | \
             ((uint64_t)(buf[6+(WSP_DATAPOINT_SIZE*i)])<<40) | \
             ((uint64_t)(buf[5+(WSP_DATAPOINT_SIZE*i)])<<48) | \
             ((uint64_t)(buf[4+(WSP_DATAPOINT_SIZE*i)])<<56);
      ts->values[i] = *(double*)&temp;
    }
    current_interval += step;
  }

  return 0;
}

int wsp_fetch(char *path, time_t from, time_t until, struct wsp_timeseries *ts) {
  FILE *wsp_file;
  struct wsp_header header;
  time_t now;
  time_t oldest;
  int diff;
  int i;

  time(&now);

  wsp_file = fopen(path, "rb");

  if (wsp_info(wsp_file, &header) == -1) {
    return -1;
  }

  oldest = now - header.max_retention;

  if (from > until) {
    fprintf(stderr, "error: from time cannot be greater than until time");
    return -1;
  }

  /* range is in the future */
  if (from > now) {
    return 0;
  }

  /* range is beyond retention */
  if (until < oldest) {
    return 0;
  }

  /* range is partially beyond retention */
  if (from < oldest) {
    from = oldest;
  }

  /* range is partially in the future */
  if (until > now) {
    until = now;
  }

  diff = now - from;
  for (i = 0 ; i < header.archive_count ; i++) {
    if (header.archives[i].retention >= diff) {
      break;
    }
  }

  printf("using archive %d\n", i);
  _wsp_fetch_archive(wsp_file, &header.archives[i], from, until, ts);

  fclose(wsp_file);

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

/* Main (tests) */
int main(int argc, char **argv) {
  FILE *my_file;
  struct wsp_header header;
  struct wsp_timeseries ts;
  char *filename = "test/mem-free.wsp";
  int i;

  time_t from;
  time_t until;
  time_t pos;

  printf("whisper info for %s\n", filename);
  my_file = fopen(filename, "rb");

  if (wsp_info(my_file, &header) == -1) {
    return -1;
  }

  printf("> aggregation type: %lu\n", header.aggregation_type);
  printf("> max retention:    %lu\n", header.max_retention);
  printf("> xfilesfactor:     %f\n", header.xff);
  printf("> archive count:    %lu\n", header.archive_count);

  for (i=0 ; i < header.archive_count ; i++) {
    printf("> archive %d\n", i);
    printf(">> offset:            %lu\n", header.archives[i].offset);
    printf(">> seconds per point: %lu\n", header.archives[i].seconds_per_point);
    printf(">> points:            %lu\n", header.archives[i].points);
    printf(">> retention:         %lu\n", header.archives[i].retention);
    printf(">> size:              %lu\n", header.archives[i].size);
  }

  fclose(my_file);


  /*
  from = 0;
  time(&until);
  wsp_fetch(filename, from, until, &ts);
  for (i = 0, pos = ts.from ; pos < ts.until ; pos += ts.step, i++) {
    printf("%d\t%f\n", (int)pos, ts.values[i]);
  }
  */


  time(&from);
  from = from - 3600;
  time(&until);
  wsp_fetch(filename, from, until, &ts);
  for (i = 0, pos = ts.from ; pos < ts.until ; pos += ts.step, i++) {
    printf("%d\t%f\n", (int)pos, ts.values[i]);
  }
  free(ts.values);


  wsp_fetch(filename, 1427083080, 1427083220, &ts);
  for (i = 0, pos = ts.from ; pos < ts.until ; pos += ts.step, i++) {
    printf("%d\t%f\n", (int)pos, ts.values[i]);
  }
  free(ts.values);


  return 0;
}

#endif