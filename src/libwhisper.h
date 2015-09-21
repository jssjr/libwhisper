#ifndef __WHISPER_LIBWHISPER_H_
#define __WHISPER_LIBWHISPER_H_

#define WSP_MAX_ARCHIVES 8
#define WSP_DATAPOINT_SIZE 12
#define WSP_HEADER_SIZE 16
#define WSP_ARCHIVE_INFO_SIZE 12
#define WSP_READ_CHUNK_SIZE 1020
#define WSP_WRITE_CHUNK_SIZE 1020

struct wsp_datapoint {
  time_t timestamp;
  double value;
};

struct wsp_archive {
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

int wsp_create(char *path, struct wsp_archive *archives, float xff);
int wsp_info(FILE *fd, struct wsp_header *header);
int wsp_update(char *path, double value, time_t timestamp);
int wsp_update_many(char *path, struct wsp_datapoint *datapoints, int num_datapoints);
int wsp_file_update_many(FILE *fd, struct wsp_datapoint *datapoints, int num_datapoints);
int wsp_fetch(char *path, time_t from, time_t until, struct wsp_archive *ts);
int wsp_file_fetch();
int wsp_aggregation_methods();
int wsp_set_aggregation_method();
int wsp_parse_retention_data();
int wsp_validate_archive_list();

#endif
