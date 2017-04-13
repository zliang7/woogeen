#ifndef JSNI_
#define JSNI_

#include <string>
#include <cassert>
#include <jsni.h>

namespace jsnih {

using string = std::string;

inline JsValue stringLocalWrapper(JSNIEnv* env, jsnih::string value) {
  return env->NewStringFromUtf8(value.c_str(), -1);
}

inline JsValue numberLocalWrapper(JSNIEnv* env, double value) {
  return env->NewNumber(value);
}

inline JsValue booleanLocalWrapper(JSNIEnv* env, bool value) {
  return env->NewBoolean(value);
}

inline jsnih::string unwrapString2Utf8Chars(JSNIEnv* env, JsValue value) {
  return jsnih::string(env->GetStringUtf8Chars(value));
}

inline double unwrapNumber(JSNIEnv* env, JsValue value) {
  return env->ToDouble(value);
}

inline bool unwrapBoolean(JSNIEnv* env, JsValue value) {
  return env->ToBool(value);
}

inline void checkInputArgsCount(JSNIEnv* env, int actual, int expect) {
  if (actual != expect) {
    env->ThrowErrorException("Wrong number of arguments from JS input.");
  }
}

inline void throwInputArgsTypeException(JSNIEnv* env) {
  env->ThrowTypeErrorException("Wrong arguments type from JS input.");
}

} // namespace jsnih

#endif
