TOP_LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_LOCAL_PATH)
include $(LOCAL_PATH)/libzip/Android.mk

include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_LOCAL_PATH)
include $(LOCAL_PATH)/libpng/Android.mk

include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_LOCAL_PATH)
include $(LOCAL_PATH)/libfreetype/Android.mk

include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_LOCAL_PATH)
include $(LOCAL_PATH)/libopenal/Android.mk

include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_LOCAL_PATH)
include $(LOCAL_PATH)/libogg/Android.mk

include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_LOCAL_PATH)
include $(LOCAL_PATH)/libvorbis/Android.mk


include $(CLEAR_VARS)
LOCAL_PATH := $(TOP_LOCAL_PATH)

LOCAL_MODULE    := libGame

LOCAL_C_INCLUDES := $(LOCAL_PATH)/libpng $(LOCAL_PATH)/libzip $(LOCAL_PATH)/libfreetype/include $(LOCAL_PATH)/libopenal/include $(LOCAL_PATH)/libopenal/OpenAL32/Include $(LOCAL_PATH)/libogg/include $(LOCAL_PATH)/libvorbis/include

LOCAL_LDLIBS    := -lz -lGLESv2 -llog -lOpenSLES

LOCAL_SRC_FILES := native.cpp \
					FBO.cpp VBO.cpp Render.cpp Shader.cpp Texture.cpp \
					JSONParser.cpp ResourceManager.cpp Font.cpp Chapter.cpp MainMenu.cpp \
					Link.cpp AI.cpp Planet.cpp HalfTree.cpp Genus.cpp Tree.cpp World.cpp \
					Button.cpp ChapterAbout.cpp Chapters.cpp CircleText.cpp platform.cpp \
					FormatText.cpp Settings.cpp Tutorial.cpp Sound.cpp 
					
LOCAL_STATIC_LIBRARIES := libzip libpng libfreetype libvorbis libogg libopenal 

include $(BUILD_SHARED_LIBRARY)
