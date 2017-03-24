my-dir ?= .
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libwoogeen

ninja-query = $(addprefix $(NINJA_OUTDIR)/, $(filter %.a, $(shell ninja -C $(NINJA_OUTDIR) -t query $(1))))
NINJA_OUTDIR = $(LOCAL_PATH)/src/out/Release
NINJA_TARGETS = woogeen_sdk_base woogeen_sdk_conf #woogeen_sdk_p2p

LOCAL_SRC_FILES := $(foreach t, $(NINJA_TARGETS), $(call ninja-query,$(t)))
ARCHIVE_FILES   := $(filter-out $(LOCAL_SRC_FILES), $(call ninja-query,all))
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
