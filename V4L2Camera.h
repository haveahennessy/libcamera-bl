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

#ifndef _V4L2CAMERA_H
#define _V4L2CAMERA_H

#define NB_BUFFER 2
#define DEFAULT_FRAME_RATE 15

#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <linux/videodev.h>
#include <utils/Log.h>


#include "media.h"
#include "v4l2-mediabus.h"
#include "v4l2-subdev.h"
#include <linux/videodev2.h>
#define LOG_FUNCTION_START    LOGD("%d: %s() ENTER", __LINE__, __FUNCTION__);
#define LOG_FUNCTION_EXIT    LOGD("%d: %s() EXIT", __LINE__, __FUNCTION__);

#define _OMAP_RESIZER_ 0

#ifdef _OMAP_RESIZER_
#include "saResize.h"
#endif //_OMAP_RESIZER_

namespace android {

struct vdIn {
	struct v4l2_capability cap;
	struct v4l2_format format;
	struct v4l2_buffer buf;
	struct v4l2_requestbuffers rb;
	void *mem[NB_BUFFER];
	bool isStreaming;
	int width;
	int height;
	int formatIn;
	int framesizeIn;
};

struct mdIn {
	int media_fd;
	int input_source;
	struct media_user_entity entity[20];
	int video;
	int ccdc;
	int resizer;
	int bugcam;
	int num_entities;
};

class V4L2Camera {

	public:
		V4L2Camera();
		~V4L2Camera();

		int Open (const char *device);
		int Configure(int width,int height,int pixelformat,int fps);
		void Close ();
		void reset_links(const char *device);
		int Open_media_device(const char *device);

		int BufferMap ();
		int init_parm();
		void Uninit ();

		int StartStreaming ();
		int StopStreaming ();

		void GrabPreviewFrame (void *previewBuffer);
		void GrabRawFrame(void *previewBuffer,unsigned int width, unsigned int height);
		void GrabJpegFrame (void *captureBuffer);
		int CreateJpegFromBuffer(void *rawBuffer,void *captureBuffer);
		int savePicture(unsigned char *inputBuffer, const char * filename);
		void convert(unsigned char *buf, unsigned char *rgb, int width, int height);

	private:
		struct vdIn *videoIn;
		struct mdIn *mediaIn;
		int camHandle;

		int nQueued;
		int nDequeued;

		int entity_dev_name(int id, char *name);
		int saveYUYVtoJPEG (unsigned char *inputBuffer, int width, int height, 
				unsigned char **outputBuffer, int quality);
};

}; // namespace android

#endif
