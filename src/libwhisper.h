#ifndef __WHISPER_LIBWHISPER_H_
#define __WHISPER_LIBWHISPER_H_

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

int wsp_info(FILE *fd, struct wsp_header *header);
int wsp_update();
int wsp_update_many();
int wsp_fetch();
int wsp_file_fetch();
int wsp_aggregation_methods();
int wsp_set_aggregation_method();
int wsp_parse_retention_data();
int wsp_validate_archive_list();

#endif
