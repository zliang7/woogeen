#ifndef _JSNISTUB_H_
#define _JSNISTUB_H_
#include <atomic>
#include <assert.h>
#include <jsni.h>
namespace jsni {
class JsniStub {
public:
  JsniStub() : use(1) { has_wraped = false; }
  void decount(JSNIEnv *jsni_env) {
    use--;
    if (use == 0)
      MakeWeak(jsni_env);
  }
  void incount(JSNIEnv *jsni_env) { use++; }
  virtual ~JsniStub() {}

  inline JsGlobalRef &GetGlobalRef() { return handle_; }

  inline void Wrap(JSNIEnv *jsni_env, JsObject &handle) {
    assert(handle.HasInternalField(jsni_env) == true);
    handle.SetInternalField(jsni_env, this);
    GetGlobalRef().Set(jsni_env, handle);
    has_wraped = true;
    MakeWeak(jsni_env);
  }

  inline void MakeWeak(JSNIEnv *jsni_env) {
    if (has_wraped)
      GetGlobalRef().SetWeakGCCallback(jsni_env, this, WeakCallback);
    else
      delete this;
  }

private:
  bool has_wraped;
  std::atomic_int use;
  JsGlobalRef handle_;
  static void WeakCallback(JSNIEnv *jsni_env, const GCCallbackInfo &info) {
    JsniStub *wrap = static_cast<JsniStub *>(info.GetInfo(jsni_env));
    wrap->handle_.Clear(jsni_env);
    delete wrap;
  }
};
}
#endif
