#ifndef __LIBWHISPER__
#define __LIBWHISPER__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <endian.h>

#define WSP_MAX_ARCHIVES 8
#define WSP_DATAPOINT_SIZE 12
#define WSP_HEADER_SIZE 16
#define WSP_ARCHIVE_INFO_SIZE 12

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

#endif
