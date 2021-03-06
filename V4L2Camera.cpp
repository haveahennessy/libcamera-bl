/*
**
** Copyright (C) 2009 0xlab.org - http://0xlab.org/
** Copyright 2008, The Android Open Source Project
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

#define LOG_TAG "V4L2Camera"
#include <utils/Log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>

#include <linux/videodev.h>
extern int version;

extern "C" {
    #include <jpeglib.h>
}
#define MEDIA_DEVICE "/dev/media0"
#define ENTITY_VIDEO_RSZ_NAME		"OMAP3 ISP resizer"
#define ENTITY_VIDEO_RSZ_OUT_NAME	"OMAP3 ISP resizer output"
#define ENTITY_VIDEO_CCDC_OUT_NAME      "OMAP3 ISP CCDC output"
#define ENTITY_CCDC_NAME                "OMAP3 ISP CCDC"
#define ENTITY_TVP514X_NAME             "tvp514x 3-005c"
#define ENTITY_MT9T111_NAME             "mt9t111 2-003c"
#define ENTITY_MT9V113_NAME             "mt9v113 2-003c"
#define ENTITY_BUGCAM_NAME             "bug_camera_subdev 3-0038"
#define IMG_WIDTH_VGA           1024
#define IMG_HEIGHT_VGA          768
#define DEF_PIX_FMT             V4L2_PIX_FMT_UYVY

#include "V4L2Camera.h"
#include "jpeg-dest.h"

namespace android {

V4L2Camera::V4L2Camera ()
    : nQueued(0), nDequeued(0)
{
	videoIn = (struct vdIn *) calloc (1, sizeof (struct vdIn));
	mediaIn = (struct mdIn *) calloc (1, sizeof (struct mdIn));
	mediaIn->input_source=1;
	camHandle = -1;
}

V4L2Camera::~V4L2Camera()
{
	free(videoIn);
	free(mediaIn);
}

int V4L2Camera::Open(const char *device)
{
	int ret = 0;
	int ccdc_fd, rsz_fd, tvp_fd;
	struct v4l2_subdev_pad_format fmt;
	char subdev[20];
	LOG_FUNCTION_START
	do
	{
		ret = entity_dev_name(mediaIn->video, subdev);
		if (ret < 0)
			return -1;
		LOGD("Output Dev Node: %s %d", subdev, mediaIn->video);
		if ((camHandle = open(subdev, O_RDWR)) == -1) {
			LOGE("ERROR opening V4L interface: %s", strerror(errno));
			reset_links(MEDIA_DEVICE);
			return -1;
		}
		/* Kernel version diff */
		ret = entity_dev_name(mediaIn->ccdc, subdev);
		if (ret < 0)
			return -1;
		LOGD("CCDC Dev Node: %s %d", subdev, mediaIn->ccdc);
		ccdc_fd = open(subdev, O_RDWR);
		if(ccdc_fd == -1) {
			LOGE("Error opening ccdc device");
			close(camHandle);
			reset_links(MEDIA_DEVICE);
			return -1;
		}
		fmt.pad = 0;
		fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
		fmt.format.code = V4L2_MBUS_FMT_YUYV16_1X16;
		fmt.format.width = IMG_WIDTH_VGA;
		fmt.format.height = IMG_HEIGHT_VGA;
		//fmt.format.colorspace = V4L2_COLORSPACE_SMPTE170M;
		fmt.format.field = V4L2_FIELD_INTERLACED;
		ret = ioctl(ccdc_fd, VIDIOC_SUBDEV_S_FMT, &fmt);
		if(ret < 0)
		{
			LOGE("Failed to set format on pad %s", strerror(errno));
		}
		memset(&fmt, 0, sizeof(fmt));
		fmt.pad = 1;
		fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
		fmt.format.code = V4L2_MBUS_FMT_YUYV16_1X16;
		fmt.format.width = IMG_WIDTH_VGA;
		fmt.format.height = IMG_HEIGHT_VGA;
		//fmt.format.colorspace = V4L2_COLORSPACE_SMPTE170M;
		fmt.format.field = V4L2_FIELD_INTERLACED;
		ret = ioctl(ccdc_fd, VIDIOC_SUBDEV_S_FMT, &fmt);
		if(ret) {
			LOGE("Failed to set format on pad");
		}
		close(ccdc_fd);

		ret = entity_dev_name(mediaIn->resizer, subdev);
		if (ret < 0)
			return -1;
		LOGD("Resizer Dev Node: %s %d", subdev, mediaIn->ccdc);
		rsz_fd = open(subdev, O_RDWR);
		if(ccdc_fd == -1) {
			LOGE("Error opening ccdc device");
			close(camHandle);
			reset_links(MEDIA_DEVICE);
			return -1;
		}
		fmt.pad = 0;
		fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
		fmt.format.code = V4L2_MBUS_FMT_YUYV16_1X16;
		fmt.format.width = IMG_WIDTH_VGA;
		fmt.format.height = IMG_HEIGHT_VGA;
		//fmt.format.colorspace = V4L2_COLORSPACE_SMPTE170M;
		fmt.format.field = V4L2_FIELD_INTERLACED;
		ret = ioctl(rsz_fd, VIDIOC_SUBDEV_S_FMT, &fmt);
		if(ret < 0)
		{
			LOGE("Failed to set format on pad %s", strerror(errno));
		}
		memset(&fmt, 0, sizeof(fmt));
		fmt.pad = 1;
		fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
		fmt.format.code = V4L2_MBUS_FMT_YUYV16_1X16;
		fmt.format.width = 640;
		fmt.format.height = 480;
		//fmt.format.colorspace = V4L2_COLORSPACE_SMPTE170M;
		fmt.format.field = V4L2_FIELD_INTERLACED;
		ret = ioctl(rsz_fd, VIDIOC_SUBDEV_S_FMT, &fmt);
		if(ret) {
			LOGE("Failed to set format on pad");
		}
		close(rsz_fd);

		ret = ioctl (camHandle, VIDIOC_QUERYCAP, &videoIn->cap);
		if (ret < 0) {
			LOGE("Error opening device: unable to query device.");
			break;
		}

		if ((videoIn->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
			LOGE("Error opening device: video capture not supported.");
			ret = -1;
			break;
		}

		if (!(videoIn->cap.capabilities & V4L2_CAP_STREAMING)) {
			LOGE("Capture device does not support streaming i/o");
			ret = -1;
			break;
		}
	}
    while(0);

	LOG_FUNCTION_EXIT
    return ret;
}

int V4L2Camera::Open_media_device(const char *device)
{

	int ret = 0;
	int index = 0;
	unsigned int i;
	struct media_user_link link;
	struct media_user_links links;
	int input_v4l;

	LOG_FUNCTION_START
	/*opening the media device*/
	mediaIn->media_fd = open(device, O_RDWR);
	if(mediaIn->media_fd <= 0)
	{
		LOGE("ERROR opening media device: %s",strerror(errno));
		return -1;
	}
	/*enumerate_all_entities*/
	do {
		mediaIn->entity[index].id = index | MEDIA_ENTITY_ID_FLAG_NEXT;
		ret = ioctl(mediaIn->media_fd, MEDIA_IOC_ENUM_ENTITIES, &mediaIn->entity[index]);
		if (ret < 0) {
			break;
		}
		else {
			if (!strcmp(mediaIn->entity[index].name, ENTITY_VIDEO_RSZ_OUT_NAME))
				mediaIn->video =  mediaIn->entity[index].id;
			else if (!strcmp(mediaIn->entity[index].name, ENTITY_VIDEO_RSZ_NAME))
				mediaIn->resizer =  mediaIn->entity[index].id;
			else if (!strcmp(mediaIn->entity[index].name, ENTITY_CCDC_NAME))
				mediaIn->ccdc =  mediaIn->entity[index].id;
			else if (!strcmp(mediaIn->entity[index].name, ENTITY_BUGCAM_NAME))
			{
				mediaIn->bugcam =  mediaIn->entity[index].id;
				mediaIn->input_source=1;
			}

		}
		LOGD("Entity Name: %s %d", mediaIn->entity[index].name, mediaIn->entity[index].id);
		index++;
	}while(ret==0);

	if ((ret < 0) && (index <= 0)) {
		LOGE("Failed to enumerate entities ret val is %d",ret);
		close(mediaIn->media_fd);
		return -1;
	}
	mediaIn->num_entities = index;

	/*setup_media_links*/
	for(index = 0; index < mediaIn->num_entities; index++) {
		links.entity = mediaIn->entity[index].id;
		links.pads = (struct media_user_pad *) malloc((sizeof( struct media_user_pad)) * (mediaIn->entity[index].pads));
		links.links = (struct media_user_link *) malloc((sizeof(struct media_user_link)) * mediaIn->entity[index].links);
		ret = ioctl(mediaIn->media_fd, MEDIA_IOC_ENUM_LINKS, &links);
		if (ret < 0) {
			LOGE("ERROR  while enumerating links/pads");
			break;
		}
		else {
			if(mediaIn->entity[index].pads)
				LOGD("pads for entity %d=", mediaIn->entity[index].id);
				for(i = 0 ; i < mediaIn->entity[index].pads; i++) {
					LOGD("(%d %s) ", links.pads->index,(links.pads->type & MEDIA_PAD_TYPE_INPUT) ?"INPUT" : "OUTPUT");
					links.pads++;
				}
			for(i = 0; i < mediaIn->entity[index].links; i++) {
				LOGD("[%d:%d]===>[%d:%d]",links.links->source.entity,links.links->source.index,links.links->sink.entity,links.links->sink.index);
				if(links.links->flags & MEDIA_LINK_FLAG_ACTIVE)
					LOGD("\tACTIVE\n");
				else
					LOGD("\tINACTIVE \n");
				links.links++;
			}
		}
	}

	input_v4l = mediaIn->bugcam;

	LOGD("Input source %d", mediaIn->input_source);
	memset(&link, 0, sizeof(link));
	link.flags |=  MEDIA_LINK_FLAG_ACTIVE;
	link.source.entity = input_v4l;
	link.source.index = 0;

	link.source.type = MEDIA_PAD_TYPE_OUTPUT;
	link.sink.entity = mediaIn->ccdc;
	link.sink.index = 0;
	link.sink.type = MEDIA_PAD_TYPE_INPUT;

	ret = ioctl(mediaIn->media_fd, MEDIA_IOC_SETUP_LINK, &link);
	if(ret) {
		LOGE("Failed to enable link bewteen entities %s", strerror(errno));
		close(mediaIn->media_fd);
		return -1;
	}
	memset(&link, 0, sizeof(link));
	link.flags |=  MEDIA_LINK_FLAG_ACTIVE;
	link.source.entity = mediaIn->ccdc;
	link.source.index = 1;
	link.source.type = MEDIA_PAD_TYPE_OUTPUT;
	link.sink.entity = mediaIn->resizer;
	link.sink.index = 0;
	link.sink.type = MEDIA_PAD_TYPE_INPUT;
	ret = ioctl(mediaIn->media_fd, MEDIA_IOC_SETUP_LINK, &link);
	if(ret){
		LOGE("Failed to enable ccdc-resizer link %s", strerror(errno));
		close(mediaIn->media_fd);
		return -1;
	}

	memset(&link, 0, sizeof(link));
	link.flags |=  MEDIA_LINK_FLAG_ACTIVE;
	link.source.entity = mediaIn->resizer;
	link.source.index = 1;
	link.source.type = MEDIA_PAD_TYPE_OUTPUT;
	link.sink.entity = mediaIn->video;
	link.sink.index = 0;
	link.sink.type = MEDIA_PAD_TYPE_INPUT;
	ret = ioctl(mediaIn->media_fd, MEDIA_IOC_SETUP_LINK, &link);
	if(ret){
		LOGE("Failed to enable resizer-resizer output link %s", strerror(errno));
		close(mediaIn->media_fd);
		return -1;
	}
	/*close media device*/
	close(mediaIn->media_fd);
	LOG_FUNCTION_EXIT
		return 0;
}
int V4L2Camera::Configure(int width,int height,int pixelformat,int fps)
{
	int ret = 0;
	LOG_FUNCTION_START

	struct v4l2_streamparm parm;

	videoIn->width = width;
	videoIn->height = height;
	videoIn->framesizeIn =((width * height) << 1);
	videoIn->formatIn = pixelformat;
	videoIn->format.fmt.pix.width =width;
	videoIn->format.fmt.pix.height =height;
	videoIn->format.fmt.pix.pixelformat = pixelformat;

    	videoIn->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	do
	{
		ret = ioctl(camHandle, VIDIOC_S_FMT, &videoIn->format);
		if (ret < 0) {
			LOGE("Open: VIDIOC_S_FMT Failed: %s", strerror(errno));
			break;
		}
		LOGD("CameraConfigure PreviewFormat: w=%d h=%d", videoIn->format.fmt.pix.width, videoIn->format.fmt.pix.height);

	}while(0);

    LOG_FUNCTION_EXIT
    return ret;
}
int V4L2Camera::BufferMap()
{
    int ret;
    LOG_FUNCTION_START
    /* Check if camera can handle NB_BUFFER buffers */
    videoIn->rb.type 	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
    videoIn->rb.memory 	= V4L2_MEMORY_MMAP;
    videoIn->rb.count 	= NB_BUFFER;

    ret = ioctl(camHandle, VIDIOC_REQBUFS, &videoIn->rb);
    if (ret < 0) {
        LOGE("Init: VIDIOC_REQBUFS failed: %s", strerror(errno));
        return ret;
    }

    for (int i = 0; i < NB_BUFFER; i++) {

        memset (&videoIn->buf, 0, sizeof (struct v4l2_buffer));

        videoIn->buf.index = i;
        videoIn->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        videoIn->buf.memory = V4L2_MEMORY_MMAP;

        ret = ioctl (camHandle, VIDIOC_QUERYBUF, &videoIn->buf);
        if (ret < 0) {
            LOGE("Init: Unable to query buffer (%s)", strerror(errno));
            return ret;
        }

        videoIn->mem[i] = mmap (0,
               videoIn->buf.length,
               PROT_READ | PROT_WRITE,
               MAP_SHARED,
               camHandle,
               videoIn->buf.m.offset);

        if (videoIn->mem[i] == MAP_FAILED) {
            LOGE("Init: Unable to map buffer (%s)", strerror(errno));
            return -1;
        }

        ret = ioctl(camHandle, VIDIOC_QBUF, &videoIn->buf);
        if (ret < 0) {
            LOGE("Init: VIDIOC_QBUF Failed");
            return -1;
        }

        nQueued++;
    }

    LOG_FUNCTION_EXIT
    return 0;
}
void V4L2Camera::reset_links(const char *device)
{
	struct media_user_link link;
	struct media_user_links links;
	int ret, index;
	unsigned int i;
	/*reset the media links*/
	mediaIn->media_fd= open(device, O_RDWR);
	for(index = 0; index < mediaIn->num_entities; index++)
	{
		links.entity = mediaIn->entity[index].id;
		links.pads = (struct media_user_pad *)malloc(sizeof( struct media_user_pad) * mediaIn->entity[index].pads);
		links.links = (struct media_user_link *)malloc(sizeof(struct media_user_link) * mediaIn->entity[index].links);
		ret = ioctl(mediaIn->media_fd, MEDIA_IOC_ENUM_LINKS, &links);
		if (ret < 0) {
			LOGD("Error while enumeration links/pads - %d\n", ret);
			break;
		}
		else {
			LOGD("Inside else");
			for(i = 0; i < mediaIn->entity[index].links; i++) {
				link.source.entity = links.links->source.entity;
				link.source.index = links.links->source.index;
				link.source.type = MEDIA_PAD_TYPE_OUTPUT;
				link.sink.entity = links.links->sink.entity;
				link.sink.index = links.links->sink.index;
				link.sink.type = MEDIA_PAD_TYPE_INPUT;
				link.flags = (link.flags & ~MEDIA_LINK_FLAG_ACTIVE) | (link.flags & MEDIA_LINK_FLAG_IMMUTABLE);
				ret = ioctl(mediaIn->media_fd, MEDIA_IOC_SETUP_LINK, &link);
				if(ret)
					break;
				links.links++;
			}
		}
	}
	close (mediaIn->media_fd);
}

void V4L2Camera::Close ()
{
	LOG_FUNCTION_START
	close(camHandle);
	camHandle = -1;

	LOG_FUNCTION_EXIT
	return;
}

int V4L2Camera::init_parm()
{
	int ret;
	int framerate;
	struct v4l2_streamparm parm;

	LOG_FUNCTION_START
		framerate = DEFAULT_FRAME_RATE;

	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(camHandle, VIDIOC_G_PARM, &parm);
	if(ret != 0) {
		LOGE("VIDIOC_G_PARM fail....");
		return ret;
	}

	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = framerate;
	ret = ioctl(camHandle, VIDIOC_S_PARM, &parm);
	if(ret != 0) {
		LOGE("VIDIOC_S_PARM  Fail....");
		return -1;
	}
	LOG_FUNCTION_EXIT
		return 0;
}

void V4L2Camera::Uninit()
{
	int ret;

	LOG_FUNCTION_START

		videoIn->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	videoIn->buf.memory = V4L2_MEMORY_MMAP;

	/* Dequeue everything */
	int DQcount = nQueued - nDequeued;

	for (int i = 0; i < DQcount-1; i++) {
		ret = ioctl(camHandle, VIDIOC_DQBUF, &videoIn->buf);
		if (ret < 0)
			LOGE("Uninit: VIDIOC_DQBUF Failed");
	}
	nQueued = 0;
	nDequeued = 0;

	/* Unmap buffers */
	for (int i = 0; i < NB_BUFFER; i++)
		if (munmap(videoIn->mem[i], videoIn->buf.length) < 0)
			LOGE("Uninit: Unmap failed");

	LOG_FUNCTION_EXIT
		return;
}

int V4L2Camera::StartStreaming ()
{
	enum v4l2_buf_type type;
	int ret;

	LOG_FUNCTION_START
		if (!videoIn->isStreaming) {
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

			ret = ioctl (camHandle, VIDIOC_STREAMON, &type);
			if (ret < 0) {
				LOGE("StartStreaming: Unable to start capture: %s", strerror(errno));
				return ret;
			}

			videoIn->isStreaming = true;
		}

	LOG_FUNCTION_EXIT
		return 0;
}

int V4L2Camera::StopStreaming ()
{
	enum v4l2_buf_type type;
	int ret;

	LOG_FUNCTION_START
		if (videoIn->isStreaming) {
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

			ret = ioctl (camHandle, VIDIOC_STREAMOFF, &type);
			if (ret < 0) {
				LOGE("StopStreaming: Unable to stop capture: %s", strerror(errno));
				return ret;
			}

			videoIn->isStreaming = false;
		}

	LOG_FUNCTION_EXIT
		return 0;
}

void V4L2Camera::GrabPreviewFrame (void *previewBuffer)
{
}

void V4L2Camera::GrabRawFrame(void *previewBuffer,unsigned int width, unsigned int height)
{
	int ret = 0;
	int DQcount = 0;

	videoIn->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	videoIn->buf.memory = V4L2_MEMORY_MMAP;


	DQcount = nQueued - nDequeued;
	if(DQcount == 0)
	{
		LOGE("postGrabRawFrame: Drop the frame");
		ret = ioctl(camHandle, VIDIOC_QBUF, &videoIn->buf);
		if (ret < 0) {
			LOGE("postGrabRawFrame: VIDIOC_QBUF Failed");
			return;
		}
	}

	/* DQ */
	ret = ioctl(camHandle, VIDIOC_DQBUF, &videoIn->buf);
	if (ret < 0) {
		LOGE("GrabRawFrame: VIDIOC_DQBUF Failed");
		return;
	}
	nDequeued++;

	if(videoIn->format.fmt.pix.width != width || \
			videoIn->format.fmt.pix.height != height)
	{
		//do resize
		//LOGE("Resizing required");
	}
	else
	{
		memcpy(previewBuffer, videoIn->mem[videoIn->buf.index], (size_t) videoIn->buf.bytesused);
	}

	ret = ioctl(camHandle, VIDIOC_QBUF, &videoIn->buf);
	if (ret < 0) {
		LOGE("postGrabRawFrame: VIDIOC_QBUF Failed");
		return;
	}

	nQueued++;
}


void V4L2Camera::GrabJpegFrame (void *captureBuffer)
{
}

int V4L2Camera::CreateJpegFromBuffer(void *rawBuffer,void *captureBuffer)
{
	FILE *output;
	FILE *input;
	int fileSize;
	unsigned char * jpeg_buf;
	int ret;

	LOG_FUNCTION_START

	do{
		LOGD("savePicture: %d x %d ", videoIn->width, videoIn->height);
		fileSize = saveYUYVtoJPEG((unsigned char *)rawBuffer, videoIn->width, videoIn->height, &jpeg_buf, 100);
	}while(0);

	memcpy(captureBuffer, jpeg_buf, fileSize);
	free(jpeg_buf);
	LOG_FUNCTION_EXIT
	return fileSize;
}

int V4L2Camera::entity_dev_name (int id, char *name)
{
	int ret = 0;
	char target[1024];
	char sysname[32];
	char *p;

	sprintf(sysname, "/sys/dev/char/%u:%u", mediaIn->entity[id - 1].v4l.major,
			mediaIn->entity[id - 1].v4l.minor);
	ret = readlink(sysname, target, sizeof(target));
	if (ret < 0)
		return ret;
	target[ret] = '\0';

	p = strrchr(target, '/');
	if (p == NULL)
		return -1;

	return sprintf(name, "/dev/%s", p + 1);

}

int V4L2Camera::saveYUYVtoJPEG (unsigned char *inputBuffer, int width, int height, 
	unsigned char **outputBuffer, int quality)
{
	int bytesPerPixel = 2;

	struct jpeg_compress_struct comp;
	struct jpeg_error_mgr error;
	struct jpeg_buffer_dest dst;

	comp.err = jpeg_std_error(&error);
	jpeg_create_compress(&comp);
	jpeg_buffer_dest (&comp, &dst);
	comp.image_width = width;
	comp.image_height = height;
	comp.input_components = 3;
	comp.in_color_space = JCS_YCbCr;
	jpeg_set_defaults(&comp);
	jpeg_set_quality(&comp, 90, TRUE);
	jpeg_start_compress(&comp, TRUE);

	unsigned char * yuvBuf = (unsigned char *) malloc(width * 3);
	if(yuvBuf < 0) {
		return 0;
	}

	for (int i = 0; i < height; ++i) {
		/*
		   From http://v4l2spec.bytesex.org/spec-single/v4l2.html#V4L2-PIX-FMT-YUYV

		   Example 2-1. V4L2_PIX_FMT_YUYV 4 ? 4 pixel image

		   Byte Order. Each cell is one byte.

		   start + 0:	Y'00	Cb00	Y'01	Cr00	Y'02	Cb01	Y'03	Cr01
		   start + 8:	Y'10	Cb10	Y'11	Cr10	Y'12	Cb11	Y'13	Cr11
		   start + 16:	Y'20	Cb20	Y'21	Cr20	Y'22	Cb21	Y'23	Cr21
		   start + 24:	Y'30	Cb30	Y'31	Cr30	Y'32	Cb31	Y'33	Cr31

		   Example 2-1. V4L2_PIX_FMT_UYVY 4 ? 4 pixel image

		   Byte Order. Each cell is one byte.

		   start + 0:	Cb00	Y'00	Cr00	Y'01	Cb01	Y'02	Cr01	Y'03
		   start + 8:	Cb10	Y'10	Cr10	Y'11	Cb11	Y'12	Cr11	Y'13
		   start + 16:	Cb20	Y'20	Cr20	Y'21	Cb21	Y'22	Cr21	Y'23
		   start + 24:	Cb30	Y'30	Cr30	Y'31	Cb31	Y'32	Cr31	Y'33
		 */
		//convert a scanline from YUYV to YCbCr
		unsigned char * yuvPtr = yuvBuf;
		const unsigned char * yuyvPtr = inputBuffer;

		while(yuvPtr < (unsigned char *) (yuvBuf + (width * 3))) {
			yuvPtr[0] = yuyvPtr[0];
			yuvPtr[1] = yuyvPtr[1];
			yuvPtr[2] = yuyvPtr[3];
			yuvPtr[3] = yuyvPtr[2];
			yuvPtr[4] = yuyvPtr[1];
			yuvPtr[5] = yuyvPtr[3];

			yuyvPtr += 4;
			yuvPtr += 6;
		}

		jpeg_write_scanlines(&comp, &yuvBuf, 1);
		inputBuffer += (width * 2);
	}

	free(yuvBuf);
	jpeg_finish_compress(&comp);
	jpeg_destroy_compress(&comp);

	*outputBuffer = dst.buf;
	size_t dataWritten = dst.used;
	return dataWritten;
}


void V4L2Camera::convert(unsigned char *buf, unsigned char *rgb, int width, int height)
{

}



}; // namespace android
