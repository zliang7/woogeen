LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= woogeen_base_headers_prebuilt
LOCAL_SRC_FILES:= \
    include/woogeen/base/audioplayerinterface.h \
    include/woogeen/base/clientconfiguration.h \
    include/woogeen/base/connectionstats.h \
    include/woogeen/base/deviceutils.h \
    include/woogeen/base/exception.h \
    include/woogeen/base/framegeneratorinterface.h \
    include/woogeen/base/globalconfiguration.h \
    include/woogeen/base/localcamerastreamparameters.h \
    include/woogeen/base/logging.h \
    include/woogeen/base/macros.h \
    include/woogeen/base/mediaformat.h \
    include/woogeen/base/stream.h \
    include/woogeen/base/videodecoderinterface.h \
    include/woogeen/base/videoencoderinterface.h \
    include/woogeen/base/videorendererinterface.h
LOCAL_MODULE_PATH:= usr/include/woogeen/base
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= woogeen_conf_headers_prebuilt
LOCAL_SRC_FILES:= \
    include/woogeen/conference/conferenceclient.h \
    include/woogeen/conference/conferenceexception.h \
    include/woogeen/conference/remotemixedstream.h \
    include/woogeen/conference/subscribeoptions.h \
    include/woogeen/conference/user.h
LOCAL_MODULE_PATH:= usr/include/woogeen/conference
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= woogeen_p2p_headers_prebuilt
LOCAL_SRC_FILES:= \
    include/woogeen/p2p/p2pexception.h \
    include/woogeen/p2p/p2psignalingchannelinterface.h \
    include/woogeen/p2p/p2psignalingreceiverinterface.h \
    include/woogeen/p2p/p2psignalingsenderinterface.h \
    include/woogeen/p2p/peerclient.h
LOCAL_MODULE_PATH:= usr/include/woogeen/p2p
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= woogeen_lib_prebuilt
LOCAL_SRC_FILES:= lib/libwoogeen.so
LOCAL_MODULE_PATH:= usr/lib
include $(BUILD_PREBUILT)
