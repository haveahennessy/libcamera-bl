/*
**
** Copyright (C) 2009 0xlab.org - http://0xlab.org/
** Copyright 2008, The Android Open Source Project
**
** This work is based on yuvconverter project.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include "converter.h"

void
yuyv422_to_yuv420(unsigned char *bufsrc, unsigned char *bufdest, int width, int height)
{
    unsigned char *ptrsrcy1, *ptrsrcy2;
    unsigned char *ptrsrcy3, *ptrsrcy4;
    unsigned char *ptrsrccb1, *ptrsrccb2;
    unsigned char *ptrsrccb3, *ptrsrccb4;
    unsigned char *ptrsrccr1, *ptrsrccr2;
    unsigned char *ptrsrccr3, *ptrsrccr4;
    int srcystride, srcccstride;

    ptrsrcy1  = bufsrc ;
    ptrsrcy2  = bufsrc + (width<<1) ;
    ptrsrcy3  = bufsrc + (width<<1)*2 ;
    ptrsrcy4  = bufsrc + (width<<1)*3 ;

    ptrsrccb1 = bufsrc + 1;
    ptrsrccb2 = bufsrc + (width<<1) + 1;
    ptrsrccb3 = bufsrc + (width<<1)*2 + 1;
    ptrsrccb4 = bufsrc + (width<<1)*3 + 1;

    ptrsrccr1 = bufsrc + 3;
    ptrsrccr2 = bufsrc + (width<<1) + 3;
    ptrsrccr3 = bufsrc + (width<<1)*2 + 3;
    ptrsrccr4 = bufsrc + (width<<1)*3 + 3;

    srcystride  = (width<<1)*3;
    srcccstride = (width<<1)*3;

    unsigned char *ptrdesty1, *ptrdesty2;
    unsigned char *ptrdesty3, *ptrdesty4;
    unsigned char *ptrdestcb1, *ptrdestcb2;
    unsigned char *ptrdestcr1, *ptrdestcr2;
    int destystride, destccstride;

    ptrdesty1 = bufdest;
    ptrdesty2 = bufdest + width;
    ptrdesty3 = bufdest + width*2;
    ptrdesty4 = bufdest + width*3;

    ptrdestcb1 = bufdest + width*height;
    ptrdestcb2 = bufdest + width*height + (width>>1);

    ptrdestcr1 = bufdest + width*height + ((width*height) >> 2);
    ptrdestcr2 = bufdest + width*height + ((width*height) >> 2) + (width>>1);

    destystride  = (width)*3;
    destccstride = (width>>1);

    int i, j;

    for(j=0; j<(height/4); j++)
    {
        for(i=0;i<(width/2);i++)
        {
            (*ptrdesty1++) = (*ptrsrcy1);
            (*ptrdesty2++) = (*ptrsrcy2);
            (*ptrdesty3++) = (*ptrsrcy3);
            (*ptrdesty4++) = (*ptrsrcy4);

            ptrsrcy1 += 2;
            ptrsrcy2 += 2;
            ptrsrcy3 += 2;
            ptrsrcy4 += 2;

            (*ptrdesty1++) = (*ptrsrcy1);
            (*ptrdesty2++) = (*ptrsrcy2);
            (*ptrdesty3++) = (*ptrsrcy3);
            (*ptrdesty4++) = (*ptrsrcy4);

            ptrsrcy1 += 2;
            ptrsrcy2 += 2;
            ptrsrcy3 += 2;
            ptrsrcy4 += 2;

            (*ptrdestcb1++) = (*ptrsrccb1);
            (*ptrdestcb2++) = (*ptrsrccb3);

            ptrsrccb1 += 4;
            ptrsrccb3 += 4;

            (*ptrdestcr1++) = (*ptrsrccr1);
            (*ptrdestcr2++) = (*ptrsrccr3);

            ptrsrccr1 += 4;
            ptrsrccr3 += 4;

        }


        /* Update src pointers */
        ptrsrcy1  += srcystride;
        ptrsrcy2  += srcystride;
        ptrsrcy3  += srcystride;
        ptrsrcy4  += srcystride;

        ptrsrccb1 += srcccstride;
        ptrsrccb3 += srcccstride;

        ptrsrccr1 += srcccstride;
        ptrsrccr3 += srcccstride;


        /* Update dest pointers */
        ptrdesty1 += destystride;
        ptrdesty2 += destystride;
        ptrdesty3 += destystride;
        ptrdesty4 += destystride;

        ptrdestcb1 += destccstride;
        ptrdestcb2 += destccstride;

        ptrdestcr1 += destccstride;
        ptrdestcr2 += destccstride;

    }
}

void
yuyv422_to_yuv420sp(unsigned char *bufsrc, unsigned char *bufdest, int width, int height)
{
    unsigned char *ptrsrcy1, *ptrsrcy2;
    unsigned char *ptrsrcy3, *ptrsrcy4;
    unsigned char *ptrsrccb1, *ptrsrccb2;
    unsigned char *ptrsrccb3, *ptrsrccb4;
    unsigned char *ptrsrccr1, *ptrsrccr2;
    unsigned char *ptrsrccr3, *ptrsrccr4;
    int srcystride, srcccstride;

    ptrsrcy1  = bufsrc ;
    ptrsrcy2  = bufsrc + (width<<1) ;
    ptrsrcy3  = bufsrc + (width<<1)*2 ;
    ptrsrcy4  = bufsrc + (width<<1)*3 ;

    ptrsrccb1 = bufsrc + 1;
    ptrsrccb2 = bufsrc + (width<<1) + 1;
    ptrsrccb3 = bufsrc + (width<<1)*2 + 1;
    ptrsrccb4 = bufsrc + (width<<1)*3 + 1;

    ptrsrccr1 = bufsrc + 3;
    ptrsrccr2 = bufsrc + (width<<1) + 3;
    ptrsrccr3 = bufsrc + (width<<1)*2 + 3;
    ptrsrccr4 = bufsrc + (width<<1)*3 + 3;

    srcystride  = (width<<1)*3;
    srcccstride = (width<<1)*3;

    unsigned char *ptrdesty1, *ptrdesty2;
    unsigned char *ptrdesty3, *ptrdesty4;
    unsigned char *ptrdestcb1, *ptrdestcb2;
    unsigned char *ptrdestcr1, *ptrdestcr2;
    int destystride, destccstride;

    ptrdesty1 = bufdest;
    ptrdesty2 = bufdest + width;
    ptrdesty3 = bufdest + width*2;
    ptrdesty4 = bufdest + width*3;

    ptrdestcb1 = bufdest + width*height;
    ptrdestcb2 = bufdest + width*height + width;

    ptrdestcr1 = bufdest + width*height + 1;
    ptrdestcr2 = bufdest + width*height + width + 1;

    destystride  = (width)*3;
    destccstride = width;

    int i, j;

    for(j=0; j<(height/4); j++)
    {
        for(i=0;i<(width/2);i++)
        {
            (*ptrdesty1++) = (*ptrsrcy1);
            (*ptrdesty2++) = (*ptrsrcy2);
            (*ptrdesty3++) = (*ptrsrcy3);
            (*ptrdesty4++) = (*ptrsrcy4);

            ptrsrcy1 += 2;
            ptrsrcy2 += 2;
            ptrsrcy3 += 2;
            ptrsrcy4 += 2;

            (*ptrdesty1++) = (*ptrsrcy1);
            (*ptrdesty2++) = (*ptrsrcy2);
            (*ptrdesty3++) = (*ptrsrcy3);
            (*ptrdesty4++) = (*ptrsrcy4);

            ptrsrcy1 += 2;
            ptrsrcy2 += 2;
            ptrsrcy3 += 2;
            ptrsrcy4 += 2;

            (*ptrdestcb1) = (*ptrsrccb1);
            (*ptrdestcb2) = (*ptrsrccb3);
            ptrdestcb1 += 2;
            ptrdestcb2 += 2;

            ptrsrccb1 += 4;
            ptrsrccb3 += 4;

            (*ptrdestcr1) = (*ptrsrccr1);
            (*ptrdestcr2) = (*ptrsrccr3);
            ptrdestcr1 += 2;
            ptrdestcr2 += 2;

            ptrsrccr1 += 4;
            ptrsrccr3 += 4;

        }


        /* Update src pointers */
        ptrsrcy1  += srcystride;
        ptrsrcy2  += srcystride;
        ptrsrcy3  += srcystride;
        ptrsrcy4  += srcystride;

        ptrsrccb1 += srcccstride;
        ptrsrccb3 += srcccstride;

        ptrsrccr1 += srcccstride;
        ptrsrccr3 += srcccstride;


        /* Update dest pointers */
        ptrdesty1 += destystride;
        ptrdesty2 += destystride;
        ptrdesty3 += destystride;
        ptrdesty4 += destystride;

        ptrdestcb1 += destccstride;
        ptrdestcb2 += destccstride;

        ptrdestcr1 += destccstride;
        ptrdestcr2 += destccstride;

    }
}

static void __yuv2rgb(int y, int cb, int cr, int *r, int *g, int *b) {
  *r = (256*y          + 614*cr)/256;
  *g = (256*y -  77*cb - 179*cr)/256;
  *b = (256*y + 453*cb         )/256;
  if(*r > 255) *r = 255;
  if(*g > 255) *g = 255;
  if(*b > 255) *b = 255;
  if(*r < 0) *r = 0;
  if(*g < 0) *g = 0;
  if(*b < 0) *b = 0;
}

static inline void yuv_to_rgb16(unsigned char y,
                                unsigned char u,
                                unsigned char v,
                                unsigned char *rgb)
{
    register int r,g,b;
    int rgb16;


    r = (1192 * (y - 16) + 1634 * (v - 128) ) >> 10;
    g = (1192 * (y - 16) - 833 * (v - 128) - 400 * (u -128) ) >> 10;
    b = (1192 * (y - 16) + 2066 * (u - 128) ) >> 10;

/*
    r = y + ((359 * v) >> 8);
    g = y - ((88 * u + 183 * v) >> 8);
    b = y + ((454 * u) >> 8);
*/
    r = r > 255 ? 255 : r < 0 ? 0 : r;
    g = g > 255 ? 255 : g < 0 ? 0 : g;
    b = b > 255 ? 255 : b < 0 ? 0 : b;

    rgb16 = (int)(((r >> 3) << 11) | ((g >> 2) << 5)| ((b >> 3) << 0));

    *rgb = (unsigned char)(rgb16 & 0xFF);
    rgb++;
    *rgb = (unsigned char)((rgb16 & 0xFF00) >> 8);

}

void yuyv422_to_rgb(unsigned char *bufsrc, unsigned char *bufdest, int width, int height)
{
	int row, col;
	int y0, y1, cr, cb;
	int r, g, b;
	int w;

	unsigned short *outbuf = (unsigned short*)bufdest;

/*
	w = width * 2;
	for(row=0; row < height; row+=2) {
		for(col=0; col < width; col+=2) {
			y0 = (((int) *(bufsrc+0)) + ((int) *(bufsrc+2)) + ((int) *(bufsrc+w)) + ((int) *(bufsrc+w+2))) / 4;

			cb = (((int) *(bufsrc+1)) + ((int) *(bufsrc+w+1)))/2 - 128;
			cr = (((int) *(bufsrc+3)) + ((int) *(bufsrc+w+3)))/2 - 128;
			__yuv2rgb(y0, cb, cr, &r, &g, &b);
			*bufdest++ = ((unsigned int) alpha) << 24 |  (r << 16) | (g << 8) | b;
			bufsrc += 4;
		}
		bufsrc += w;
		//LOGD("row=%d ibuf=0x%x out=0x%x", row, bufsrc, bufdest);
	}

*/
	for(row=0; row < height; row++) {
		for(col=0; col < width; col+=2) {
			y0 = ((int) *bufsrc++);
			cb = ((int) *bufsrc++);
			y1 = ((int) *bufsrc++);
			cr = ((int) *bufsrc++);
			yuv_to_rgb16(y0, cb, cr, bufdest);
			bufdest+=2;
			yuv_to_rgb16(y0, cb, cr, bufdest);
			bufdest+=2;

		}
	}


}


