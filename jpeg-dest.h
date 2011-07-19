/*#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <jpeglib.h>*/

struct jpeg_buffer_dest {
        struct jpeg_destination_mgr jdst;
        JOCTET *buf;
        JOCTET *off;
        size_t sz;
        size_t used;
};

void jpeg_buffer_dest(j_compress_ptr cinfo, struct
jpeg_buffer_dest *dst);
