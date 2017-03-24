#ifndef _NATIVE_H
#define _NATIVE_H

#include <jsni.h>
namespace jsni {

#define JsRegisterClass(MyClass) MyClass::_initializeClass(env, exports)

#define JSNI_INIT                                                              \
  extern "C" int JSNI_Init(jsni::JSNIEnv *env, const jsni::JsValue exports)

#define JsRegisterMethod(MyFunction)                                           \
  JSNI::RegisterMethod(env, exports, #MyFunction, MyFunction);

#define JsReturnValue(value) _JsReturnValue(env, arguments, (value))

#define JsReturnNull() arguments.SetReturnValue(env, JsValue::Null(env))

#define JsReturnUndefined()                                                    \
  arguments.SetReturnValue(env, JsValue::Undefined(env))

void _JsReturnValue(JSNIEnv *env, const NativeCallbackInfo &arguments,
                    int value) {
  arguments.SetReturnValue(env, JsNumber(env, value));
}

void _JsReturnValue(JSNIEnv *env, const NativeCallbackInfo &arguments,
                    double value) {
  arguments.SetReturnValue(env, JsNumber(env, value));
}

void _JsReturnValue(JSNIEnv *env, const NativeCallbackInfo &arguments,
                    bool value) {
  arguments.SetReturnValue(env, JsValue(env, value));
}

void _JsReturnValue(JSNIEnv *env, const NativeCallbackInfo &arguments,
                    NativeFunctionCallback value) {
  arguments.SetReturnValue(env, JsFunction(env, value));
}

void _JsReturnValue(JSNIEnv *env, const NativeCallbackInfo &arguments,
                    const JsValue &obj) {
  arguments.SetReturnValue(env, obj);
}

void _JsReturnValue(JSNIEnv *env, const NativeCallbackInfo &arguments,
                    const char *str) {
  arguments.SetReturnValue(env, JsString(env, str));
}

void _JsReturnValue(JSNIEnv *env, const NativeCallbackInfo &arguments,
                    const JsString &str) {
  arguments.SetReturnValue(env, str);
}

void _JsReturnValue(JSNIEnv *env, const PropertyInfo &arguments, int value) {
  arguments.SetReturnValue(env, JsValue(env, value));
}

void _JsReturnValue(JSNIEnv *env, const PropertyInfo &arguments, double value) {
  arguments.SetReturnValue(env, JsValue(env, value));
}

void _JsReturnValue(JSNIEnv *env, const PropertyInfo &arguments,
                    const char *value) {
  arguments.SetReturnValue(env, JsString(env, value));
}

#define JsCtor(className)                                                      \
  }                                                                            \
  static void New(JSNIEnv *env, const jsni::NativeCallbackInfo &arguments) {   \
    if (arguments.IsConstructCall(env)) {                                      \
      className *self = new className(env, arguments.Length(env), arguments);  \
      JsObject newObj = globalRefObj.ToLocal(env).AsObject(env);               \
      self->Wrap(env, newObj);                                                 \
      arguments.SetReturnValue(env, newObj);                                   \
    }                                                                          \
  }                                                                            \
  className(JSNIEnv *env, int argc, const jsni::NativeCallbackInfo &arguments) {

#define JsNativeFunc(funcName)                                                 \
  }                                                                            \
  void funcName(jsni::JSNIEnv *env, const jsni::NativeCallbackInfo &arguments) {

#define anyType void *

#define InitClass(className) JsGlobalRef className::globalRefObj;

#define JsMemberFunc(methodName)                                               \
  }                                                                            \
  static void _##methodName##Register(JSNIEnv *env) {                          \
    auto obj = globalRefObj.ToLocal(env).AsObject(env);                        \
    obj.SetProperty(env, #methodName, JsFunction(env, methodName));            \
  }                                                                            \
  static void methodName(JSNIEnv *env,                                         \
                         const jsni::NativeCallbackInfo &arguments) {          \
    GetThis(arguments);

#define GetThis(arguments)                                                     \
  if (!arguments.This(env).HasInternalField(env))                              \
    return;                                                                    \
  auto This = reinterpret_cast<decltype(_type)>(                               \
      arguments.This(env).GetInternalField(env));

#define JsniClass(className)                                                   \
  static JsGlobalRef globalRefObj;                                             \
  static className *_type;                                                     \
  \
public:                                                                        \
  static void _initializeClass(JSNIEnv *env, JsValue exports) {                \
    JsObject obj = JsObject::NewWithInternalField(env);                        \
    globalRefObj.Set(env, obj);                                                \
    JSNI::RegisterMethod(env, exports, #className, New);                       \
    _initializeMethod(env);                                                    \
  }                                                                            \
  static void _initializeMethod(JSNIEnv *env)

#define JsniMethod(methodName) _##methodName##Register(env);

#define JsniClassAccessor(valueName, getter, setter)                           \
  JsniAccessor(globalRefObj.ToLocal(env).AsObject(env), valueName, getter,     \
               setter)

#define JsniClassReadOnlyAccessor(valueName, getter)                           \
  JsniReadOnlyAccessor(globalRefObj.ToLocal(env).AsObject(env), valueName,     \
                       getter)

#define JsClassGtter(funcName)                                                 \
  }                                                                            \
  static void Getter(JSNIEnv *env, JsValue *name,                              \
                     const PropertyInfo &arguments) {                          \
    GetThis(arguments);

#define JsClassSetter(funcName)                                                \
  }                                                                            \
  static void Setter(JSNIEnv *env, JsValue *name, JsValue value,               \
                     const PropertyInfo &arguments) {                          \
    GetThis(arguments);

#define JsniAccessor(obj, jsname, getter, setter)                              \
  obj.SetAccessor(env, jsname, getter, setter)

#define JsniReadOnlyAccessor(obj, jsname, getter)                              \
  obj.SetAccessor(env, jsname, getter)

#define JsRegisterAccessor(valueName, getter, setter)                          \
  JsniAccessor(exports.AsObject(env), valueName, getter, setter);

#define JsRegisterReadOnlyAccessor(valueName, getter)                          \
  JsniReadOnlyAccessor(exports.AsObject(env), valueName, getter);

#define JsGetter(funcName)                                                     \
  }                                                                            \
  static void funcName(JSNIEnv *env, JsValue *name,                            \
                       const PropertyInfo &arguments) {

#define JsSetter(funcName)                                                     \
  }                                                                            \
  static void funcName(JSNIEnv *env, JsValue *name, JsValue value,             \
                       const PropertyInfo &arguments) {
}
#endif
