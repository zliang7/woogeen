LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libwoogeen

ninja = ninja -C $(NINJA_OUTDIR)
ninja-query = $(filter-out obj.host/%, $(shell $(ninja) -t query $(1)))
ninja-query-archive = $(filter %.a, $(call ninja-query, $(1)))
ninja-query-object = $(filter %.o, $(call ninja-query, $(1)))
NINJA_OUTDIR  := $(LOCAL_PATH)/src/out/Debug
NINJA_TARGETS := woogeen_sdk_base woogeen_sdk_conf #woogeen_sdk_p2p
NINJA_TARGETS := $(foreach t, $(NINJA_TARGETS), $(call ninja-query-archive,$(t)))

LOCAL_SRC_OBJECTS := $(foreach t, $(NINJA_TARGETS), $(call ninja-query-object,$(t)))
LOCAL_SRC_OBJECTS := $(addprefix $(NINJA_OUTDIR)/, $(LOCAL_SRC_OBJECTS))

LOCAL_SRC_ARCHIVES := $(filter-out $(NINJA_TARGETS), $(call ninja-query-archive,all))
LOCAL_SRC_ARCHIVES := $(addprefix $(NINJA_OUTDIR)/, $(LOCAL_SRC_ARCHIVES))
LOCAL_ADDITIONAL_DEPENDENCIES += $(LOCAL_SRC_ARCHIVES)

LOCAL_LDFLAGS += \
    -Wl,--start-group $(LOCAL_SRC_ARCHIVES) -Wl,--end-group \
    -Wl,--version-script=$(LOCAL_PATH)/libwoogeen.map \
    -L$(LOCAL_PATH)/prebuilt -lsioclient -lboost_system \
    -lssl -lcrypto -lstdc++ -lm -lpthread -ldl

LOCAL_HOOK_CLEAN_CMDS := $(ninja) -t clean

$(LOCAL_SRC_OBJECTS) $(LOCAL_SRC_ARCHIVES): $(NINJA_OUTDIR)/yasm
	$(ninja) $(subst $(NINJA_OUTDIR)/,,$@)
$(NINJA_OUTDIR)/yasm:
	$(ninja) yasm

include $(BUILD_SHARED_LIBRARY)
