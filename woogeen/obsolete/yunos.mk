LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := woogeen

LOCAL_CALL_BY_XMAKE_CONF := true

LOCAL_CMD_MAKE := \
    cd src; \
    cp -f talk/woogeen/patches/libsrtp.gyp chromium/src/third_party/libsrtp/libsrtp.gyp; \
    cp -f talk/woogeen/patches/usrsctp.gyp chromium/src/third_party/usrsctp/usrsctp.gyp; \
    export CFLAGS="`echo $${CFLAGS} | sed 's,-fno-omit-frame-pointer , ,g'`" ;\
    export CFLAGS="`echo $${CFLAGS} | sed 's, -fno-omit-frame-pointer, ,g'`" ;\
    export CXXFLAGS="`echo $${CXXFLAGS} | sed 's,-fno-omit-frame-pointer , ,g'`" ;\
    export CXXFLAGS="`echo $${CXXFLAGS} | sed 's, -fno-omit-frame-pointer, ,g'`" ;\
    GYP_DEFINES="clang=0 openssl_is_boringssl=0 ssl_root=$(openssl-includes)/.. use_sysroot=0 use_ozone=1 use_gtk=0 include_internal_audio_device=0 include_tests=0 include_examples=0 include_woogeen_samples=0 target_arch=ia32" webrtc/build/gyp_webrtc; \
    (cd chromium/src/third_party/libsrtp; git checkout libsrtp.gyp); \
    (cd chromium/src; git checkout third_party/usrsctp/usrsctp.gyp); \
    sh ../export.sh talk/woogeen/sdk/include/cpp; \
    ninja -j$${OPT_CPU_NUM} -C out/Release; \
    sh ../export.sh -r talk/woogeen/sdk/include/cpp

LOCAL_CMD_MAKE_INSTALL := \
    echo "install woogeen"; \
    cp -pr src/talk/woogeen/sdk/include/cpp/woogeen $${buildroot}$${_includedir}; \
    install -p -D src/out/Release/lib/libwoogeen.so $${buildroot}$${_libdir}/libwoogeen.so

LOCAL_CMD_MAKE_CLEAN := \
    ninja -C src/out/Release -t clean; \
    ninja -C src/out/Debug -t clean

LOCAL_CMD_MAKE_DISTCLEAN := \
    rm -rf src/out

include $(BUILD_MODULE)
