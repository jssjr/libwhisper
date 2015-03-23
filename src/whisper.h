#ifndef __LIBWHISPER__
#define __LIBWHISPER__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <endian.h>

#define WSP_MAX_ARCHIVES 8
#define WSP_DATAPOINT_SIZE 12
#define WSP_HEADER_SIZE 16
#define WSP_ARCHIVE_INFO_SIZE 12
#define WSP_READ_CHUNK_SIZE 1020
#define WSP_GIT_SHA GIT_SHA

#if __BYTE_ORDER == __BIG_ENDIAN
#define Swap2Bytes(val) val
#define Swap4Bytes(val) val
#define Swap8Bytes(val) val
#else
#define Swap2Bytes(val) \
 ( (((val) >> 8) & 0x00FF) | (((val) << 8) & 0xFF00) )
#define Swap4Bytes(val) \
 ( (((val) >> 24) & 0x000000FF) | (((val) >>  8) & 0x0000FF00) | \
   (((val) <<  8) & 0x00FF0000) | (((val) << 24) & 0xFF000000) )
#define Swap8Bytes(val) \
 ( (((val) >> 56) & 0x00000000000000FF) | (((val) >> 40) & 0x000000000000FF00) | \
   (((val) >> 24) & 0x0000000000FF0000) | (((val) >>  8) & 0x00000000FF000000) | \
   (((val) <<  8) & 0x000000FF00000000) | (((val) << 24) & 0x0000FF0000000000) | \
   (((val) << 40) & 0x00FF000000000000) | (((val) << 56) & 0xFF00000000000000) )
#endif

union {
  uint8_t buf[4];
  float value;
} u_float;

union {
  uint8_t buf[8];
  double value;
} u_double;

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

int wsp_info(FILE *fd, struct wsp_header *header);

#endif
