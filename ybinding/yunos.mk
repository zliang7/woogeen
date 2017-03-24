LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := woogeen_jsni

LOCAL_CALL_BY_XMAKE_CONF := true

LOCAL_SRC_FILES := \
	src/bindings.cc \
	src/fileaudioframegenerator.cc \
	src/filevideoframegenerator.cc \
	src/yunosaudioframegenerator.cc \
	src/yunosaudioplayer.cc

#LOCAL_SRC_FILES += $(wildcard src/dummy/*.cc)

# Flags passed to both C and C++ files.
LOCAL_CXXFLAGS := -std=c++11 -Wno-unused-variable -Wno-unused-result

# Include paths placed before CFLAGS/CPPFLAGS
LOCAL_C_INCLUDES := \
	. \
	$(LOCAL_PATH)/include/jsni \
	$(XMAKE_ROOTFS)/usr/include \
	$(audioserver-includes) \
	$(base-includes) \
	$(corefoundation-includes)

LOCAL_REQUIRED_MODULES += base
ifneq ($(WOOGEEN_SDK_BIN),)
LOCAL_REQUIRED_MODULES += woogeen_bin
else
LOCAL_REQUIRED_MODULES += woogeen
endif

LOCAL_SHARED_LIBRARIES += libbase liblog libaudio libcore_foundation
LOCAL_LDFLAGS += -ljsni_async_stub -lwoogeen

LOCAL_COMPILE_SHELL_CMD := \
	mkdir -p $(XMAKE_ROOTFS)/usr/framework/woogeen \
	&& install -p $(LOCAL_PATH)/jsmodule/*.js $(XMAKE_ROOTFS)/usr/framework/woogeen \
	&& install -p $(LOCAL_PATH)/jsmodule/package.json $(XMAKE_ROOTFS)/usr/framework/woogeen

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := woogeen_audiocapture_test

LOCAL_CALL_BY_XMAKE_CONF := true

LOCAL_SRC_FILES := \
	src/yunosaudioframegenerator_unittest.cc

LOCAL_CXXFLAGS := -std=c++11 -Wno-unused-variable

LOCAL_C_INCLUDES := \
	$(XMAKE_ROOTFS)/usr/include \
	$(audioserver-includes) \
	$(corefoundation-includes) \
	$(gtest-includes)

LOCAL_REQUIRED_MODULES += base woogeen_jsni
LOCAL_SHARED_LIBRARIES += libbase liblog libaudio libcore_foundation libgtest libgtest_main woogeen_jsni

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := woogeen_audioplayer_test

LOCAL_CALL_BY_XMAKE_CONF := true

LOCAL_SRC_FILES := \
	src/yunosaudioplayer_unittest.cc

LOCAL_CXXFLAGS := -std=c++11 -Wno-unused-variable

LOCAL_C_INCLUDES := \
	$(XMAKE_ROOTFS)/usr/include \
	$(audioserver-includes) \
	$(corefoundation-includes) \
	$(gtest-includes)

LOCAL_REQUIRED_MODULES += base woogeen_jsni
LOCAL_SHARED_LIBRARIES += libbase liblog libaudio libcore_foundation libgtest libgtest_main woogeen_jsni

include $(BUILD_EXECUTABLE)
