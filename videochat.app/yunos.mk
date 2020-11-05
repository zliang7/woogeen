LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    manifest.json \
    res/* \
    lib/* \
    src/* \
    test/*

LOCAL_MODULE_PATH:=

LOCAL_REQUIRED_MODULES:=

LOCAL_MODULE:= videochatdemo

LOCAL_CERTIFICATE := system

LOCAL_PRIVILEGED_MODULE:= true
include $(BUILD_PACKAGE)
