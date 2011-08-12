LOCAL_PATH:= $(call my-dir)

ifneq ($(USE_CAMERA_STUB),true)
#
# libcamera
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    CameraHardware.cpp \
    V4L2Camera.cpp \
    jpeg-dest.cpp \
    media.c \
    converter.cpp

LOCAL_SHARED_LIBRARIES:= \
    libcutils \
    libui \
    libutils \
    libbinder \
    libjpeg \
    libcamera_client \
    libsurfaceflinger_client

LOCAL_C_INCLUDES += \
    external/jpeg

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= libcamera

include $(BUILD_SHARED_LIBRARY)
endif

include $(all-subdir-makefiles)
