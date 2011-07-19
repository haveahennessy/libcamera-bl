#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <jpeglib.h>
#include "jpeg-dest.h"


static void jpeg_buffer_dest_init(j_compress_ptr cinfo)
{
        struct jpeg_buffer_dest *dst = (struct jpeg_buffer_dest*)cinfo->dest;
        dst->used = 0;
        dst->sz = cinfo->image_width
                * cinfo->image_height
                * cinfo->input_components / 8;  /* 1/8th of raw size
*/
        dst->buf = (JOCTET*) malloc(dst->sz * sizeof *dst->buf);
	memset(dst->buf, 0, dst->sz);
        dst->off = dst->buf;
        dst->jdst.next_output_byte = dst->off;
        dst->jdst.free_in_buffer = dst->sz;
        return;
}

static boolean jpeg_buffer_dest_empty(j_compress_ptr cinfo)
{
        struct jpeg_buffer_dest *dst = (struct jpeg_buffer_dest *)cinfo->dest;
        dst->sz *= 2;
        dst->used = dst->off - dst->buf;
        dst->buf = (JOCTET*) realloc(dst->buf, dst->sz * sizeof *dst->buf);
        dst->off = dst->buf + dst->used;
        dst->jdst.next_output_byte = dst->off;
        dst->jdst.free_in_buffer = dst->sz - dst->used;
        return TRUE;
}

static void jpeg_buffer_dest_term(j_compress_ptr cinfo)
{
        struct jpeg_buffer_dest *dst = (struct jpeg_buffer_dest *)cinfo->dest;
        dst->used += dst->sz - dst->jdst.free_in_buffer;
        dst->off = dst->buf + dst->used;
        return;
}

void jpeg_buffer_dest(j_compress_ptr cinfo, struct
jpeg_buffer_dest *dst)
{
        dst->jdst.init_destination = jpeg_buffer_dest_init;
        dst->jdst.empty_output_buffer = jpeg_buffer_dest_empty;
        dst->jdst.term_destination = jpeg_buffer_dest_term;
        cinfo->dest = (struct jpeg_destination_mgr *)dst;
        return;
}
