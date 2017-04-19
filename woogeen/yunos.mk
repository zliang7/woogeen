my-dir ?= .
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libwoogeen

ninja-query = $(addprefix $(NINJA_OUTDIR)/, $(shell ninja -C $(NINJA_OUTDIR) -t query $(1)))
ninja-query-archive = $(filter %.a, $(call ninja-query, $(1)))
ninja-query-object = $(filter %.o, $(call ninja-query, $(1)))
NINJA_OUTDIR = $(LOCAL_PATH)/src/out/Release
NINJA_TARGETS = woogeen_sdk_base woogeen_sdk_conf #woogeen_sdk_p2p

LOCAL_SRC_FILES := $(foreach t, $(NINJA_TARGETS), $(call ninja-query-archive,$(t)))
ARCHIVE_FILES   := $(filter-out $(LOCAL_SRC_FILES), $(call ninja-query-archive,all))
LOCAL_SRC_FILES := $(foreach a, $(LOCAL_SRC_FILES), $(shell $(AR) -t $(a)))

LOCAL_LDFLAGS += \
    -Wl,--start-group $(ARCHIVE_FILES) -Wl,--end-group \
    -Wl,--version-script=$(LOCAL_PATH)/libwoogeen.map \
    -L$(LOCAL_PATH)/prebuilt -lsioclient -lboost_system \
    -lssl -lcrypto -lexpat -lstdc++ -lm -lpthread -ldl

include $(BUILD_SHARED_LIBRARY)

ifeq ($(BUILD_SHARED_LIBRARY),)
$(LOCAL_MODULE).so: $(LOCAL_SRC_FILES) $(ARCHIVE_FILES) $(MAKEFILE_LIST)
	$(CC) -shared -m32 -Wl,--no-undefined -o $@ $(LOCAL_SRC_FILES) $(LOCAL_LDFLAGS)
endif


include $(CLEAR_VARS)
LOCAL_MODULE := woogeen_conf_sample
LOCAL_REQUIRED_MODULES += libwoogeen libmmwebrtc
LOCAL_SRC_FILES := $(call ninja-query-object, $(LOCAL_MODULE))

LOCAL_SRC_FILES := $(subst $(LOCAL_MODULE).,,$(notdir $(LOCAL_SRC_FILES:.o=.cc)))
LOCAL_SRC_FILES := $(addprefix $(LOCAL_PATH)/src/talk/woogeen/samples/, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(subst conferencesample.cc,conferencesample.cpp,$(LOCAL_SRC_FILES))  # FIXME
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/src/talk/woogeen/include \
    $(LOCAL_PATH)/src/talk/woogeen/sdk/include/cpp \
    $(multimedia-webrtc-includes)
LOCAL_CXXFLAGS := -std=c++11 -g

LOCAL_SHARED_LIBRARIES += libwoogeen libmmwebrtc
LOCAL_LDFLAGS += -L$(LOCAL_PATH)/prebuilt -lasiotoken -lboost_system -lsioclient_tls -lpthread
include $(BUILD_EXECUTABLE)
