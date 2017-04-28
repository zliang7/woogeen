LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := woogeen_conf_sample
LOCAL_REQUIRED_MODULES += woogeen multimedia-webrtc

LOCAL_SRC_FILES := \
    yunosaudioframegenerator.cc \
    yunosaudioplayer.cc \
    conferencesampleobserver.cc \
    conferencesample.cc

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/src/talk/woogeen/sdk/include/cpp \
    $(multimedia-webrtc-includes) \
    $(audioserver-includes) \
    $(corefoundation-includes) \
    $(base-includes) \
    $(XMAKE_ROOTFS)/usr/include

LOCAL_CXXFLAGS := -std=c++11 -g -Wno-unused-variable -Wno-unused-result

LOCAL_SHARED_LIBRARIES += liblog libaudio libmmwebrtc libwoogeen
LOCAL_LDFLAGS += -L$(LOCAL_PATH)/../prebuilt -lasiotoken -lboost_system -lsioclient -lpthread

include $(BUILD_EXECUTABLE)
