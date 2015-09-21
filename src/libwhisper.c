#include "whisper.h"

const char* wsp_aggregation_type_map[] = {
};

typedef int (*compfn)(const void*, const void*);

int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}

int wsp_create(char *path, struct wsp_archive *archives, float xff) {
  FILE *fd;

  if (access(path, W_OK ) == -1) {
    fprintf(stderr, "unable to create file\n");
    return 1;
  }
  fd = fopen(path, "wb");

  fclose(fd);
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
  memcpy(&header->xff, &temp, sizeof(uint32_t));
  header->archive_count = (buf[15]<<0) | (buf[14]<<8) | (buf[13]<<16) | (buf[12]<<24);
  /* XXX: assert valid header values here? */
  if (header->archive_count <= 0) {
    return -1;
  }

  for (i = 0 ; i < header->archive_count ; i++) {
    if (fread(&buf, sizeof(uint8_t), WSP_ARCHIVE_INFO_SIZE, fd) == 0) {
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

int wsp_update(char *path, double value, time_t timestamp) {
  struct wsp_datapoint datapoints[1];

  wsp_update_many(path, datapoints, 1);
  return 0;
}

int _datapoint_comp(const struct wsp_datapoint *a, const struct wsp_datapoint *b) {
  if (a->timestamp > b->timestamp) return 1;
  if (a->timestamp < b->timestamp) return 1;
  return 0;
}

int wsp_file_update_many(FILE *fd, struct wsp_datapoint *datapoints, int num_datapoints) {
  struct wsp_datapoint *datapoint;
  struct wsp_header header;
  time_t now;
  int age;

  time(&now);

  qsort(datapoints, num_datapoints, sizeof(struct wsp_datapoint), (compfn)_datapoint_comp);

  if (wsp_info(fd, &header) == -1) {
    return 1;
  }

  for (int i=0; i<num_datapoints; i++) {
    datapoint = &datapoints[i];

    printf("asked to update: %ld\t%f\n", datapoint->timestamp, datapoint->value);

    age = now - datapoint->timestamp;

  }
  return 0;
}

int wsp_update_many(char *path, struct wsp_datapoint *datapoints, int num_datapoints) {
  FILE *fd;
  int retcode;

  // XXX: Needs to flock around this

  if (access(path, W_OK ) == -1) {
    fprintf(stderr, "unable to open whisper file %s\n", path);
    return 1;
  }
  fd = fopen(path, "ab+");

  retcode = wsp_file_update_many(fd, datapoints, num_datapoints);

  fclose(fd);
  return retcode;
}

int _wsp_fetch_archive(FILE *fd, struct wsp_archive_info *archive, time_t from, time_t until, struct wsp_archive *ts) {
  long from_interval, until_interval, base_interval, current_interval;
  long from_offset, until_offset, buffer_offset;
  long step, read_size, chunk_size;
  long points, point_time;
  uint8_t datapoint_buf[WSP_DATAPOINT_SIZE];
  uint8_t buf[WSP_DATAPOINT_SIZE*WSP_READ_CHUNK_SIZE];
  uint64_t temp;
  int i, c;
  long td, pd, bd;

  from_interval = (from - (mod(from, archive->seconds_per_point))) + archive->seconds_per_point;
  until_interval = (until - (mod(until, archive->seconds_per_point))) + archive->seconds_per_point;

  fseek(fd, archive->offset, SEEK_SET);
  if (fread(&datapoint_buf, sizeof(uint8_t), WSP_DATAPOINT_SIZE, fd) == 0) {
    return -1;
  }

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
  from_offset = archive->offset + mod(bd, archive->size);

  td = until_interval - base_interval;
  pd = td / archive->seconds_per_point;
  bd = pd * WSP_DATAPOINT_SIZE;
  until_offset = archive->offset + mod(bd, archive->size);

  if (fseek(fd, from_offset, SEEK_SET) == -1) {
    return -1;
  }

  setvbuf(fd, (char*)NULL, _IOFBF, 0);

  current_interval = from_interval;
  i = 0;
  if (from_offset < until_offset) {
    /* no need to wrap, chunk it all */
    chunk_size = until_offset - from_offset;
  } else {
    /* wrap around the archive, so only chunk the fist part */
    chunk_size = archive->offset + archive->size - from_offset;
  }
  for (i = 0; i < points;) {
    if (chunk_size <= 0) {
      /* second pass */
      chunk_size = archive->offset + archive->size - from_offset;
      if (fseek(fd, archive->offset, SEEK_SET) == -1) {
        return -1;
      }
    }
    read_size = (chunk_size > WSP_READ_CHUNK_SIZE) ? WSP_READ_CHUNK_SIZE : chunk_size;
    chunk_size = chunk_size - read_size;
    if (fread(&buf, read_size, 1, fd) == 0) {
      return -1;
    }
    for (c=0;c<(read_size/WSP_DATAPOINT_SIZE);c++) {
      buffer_offset = WSP_DATAPOINT_SIZE*c;
      point_time = (buf[3+buffer_offset]<<0) | \
                   (buf[2+buffer_offset]<<8) | \
                   (buf[1+buffer_offset]<<16) | \
                   (buf[0+buffer_offset]<<24);
      if (point_time == current_interval) {
        temp = ((uint64_t)(buf[11+buffer_offset])<<0) | \
               ((uint64_t)(buf[10+buffer_offset])<<8) | \
               ((uint64_t)(buf[9+buffer_offset])<<16) | \
               ((uint64_t)(buf[8+buffer_offset])<<24) | \
               ((uint64_t)(buf[7+buffer_offset])<<32) | \
               ((uint64_t)(buf[6+buffer_offset])<<40) | \
               ((uint64_t)(buf[5+buffer_offset])<<48) | \
               ((uint64_t)(buf[4+buffer_offset])<<56);
        memcpy(&ts->values[i], &temp, sizeof(uint64_t));
      }
      i += c;
      current_interval += step;
    }
  }

  return 0;
}

int wsp_fetch(char *path, time_t from, time_t until, struct wsp_archive *ts) {
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
