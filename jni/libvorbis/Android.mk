LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../libogg/include $(LOCAL_PATH)/include

LOCAL_MODULE := libvorbis
LOCAL_CFLAGS += -O3

LOCAL_SRC_FILES := \
	mdct.c		\
	smallft.c	\
	block.c		\
	envelope.c	\
	window.c	\
	lsp.c		\
	lpc.c		\
	analysis.c	\
	synthesis.c	\
	psy.c		\
	info.c		\
	floor1.c	\
	floor0.c	\
	res0.c		\
	mapping0.c	\
	registry.c	\
	codebook.c	\
	sharedbook.c	\
	lookup.c	\
	bitrate.c	\
	vorbisfile.c	\
	vorbisenc.c

LOCAL_STATIC_LIBRARIES := libogg

include $(BUILD_STATIC_LIBRARY)

