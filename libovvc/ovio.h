#ifndef OVIO_H
#define OVIO_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * This file contains wrappers for IO functions
 * names  should mimic stdio functions since they
 * are basically aiming to offer the same interface
 */

typedef struct OVIOStream OVIOStream;

typedef struct OVIO {
    int (*const close)(struct OVIO*);
    size_t (*const read)(void *, struct OVIO*);
    int (*const eof)(struct OVIO*);
    size_t size;
} OVIO;

typedef struct OVFileIO {
    struct OVIO super;
    FILE* file;
} OVFileIO;

typedef struct OVBuffIO {
    struct OVIO super;
    uint8_t* buff;
    bool eof;
    int error;
} OVBuffIO;

OVFileIO* ovio_new_fileio(const char* path, const char* mode);
OVBuffIO* ovio_new_buffio(const uint8_t* buff, size_t size);

OVIOStream *ovio_stream_open(OVIO *io);

void ovio_stream_close(OVIOStream *io_str);

size_t ovio_stream_read(const uint8_t **dst_buff, OVIOStream *const io_str);

int ovio_stream_eof(OVIOStream *const io_str);

size_t ovio_stream_buff_size(OVIOStream* const io_str);

#endif
