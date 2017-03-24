// JavaScript Native Interface Release License.
//
// Copyright (c) 2015-2016 Alibaba Group. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Alibaba Group nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: Jin Yue; Yu Chaojun

#ifndef _JSNI_H
#define _JSNI_H
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

namespace jsni {
// Everything here should be VM-neutral.

// TODO(jiny). Env should have function pointers and isolate both.
// So in call method call, it should be: env->functions->METHOD().

struct JSNINativeInterface;
typedef JSNINativeInterface JSNIEnv;


typedef void* JsLocal;
typedef void* JsGlobal;
typedef void* ContextRef;
typedef void* RuntimeRef;
typedef void* JsScope;

class JsValue;
class JsNumber;
class JsBoolean;
class JsObject;
class JsFunction;
class JsString;
class JsTypedArray;
class JsGlobalRef;

class JsRuntime;
class JsContext;
class LocalRefScope;

class GCCallbackInfo;
class NativeCallbackInfo;
class PropertyInfo;


typedef void (*GCCallback)(JSNIEnv* env, const GCCallbackInfo& info);
typedef void (*NativeFunctionCallback)(JSNIEnv* env, const NativeCallbackInfo& info);
typedef void (*GetterCallback)(JSNIEnv* env, JsValue *name, const PropertyInfo& info);
typedef void (*SetterCallback)(JSNIEnv* env, JsValue *name, JsValue val,
             const PropertyInfo& info);

typedef enum _JsTypedArrayType {
  // Not a JsArrayType
  JsArrayTypeNone,
  // An int8 array.
  JsArrayTypeInt8,
  // An uint8 array.
  JsArrayTypeUint8,
  // An uint8 clamped array.
  JsArrayTypeUint8Clamped,
  // An int16 array.
  JsArrayTypeInt16,
  // An uint16 array.
  JsArrayTypeUint16,
  // An int32 array.
  JsArrayTypeInt32,
  // An uint32 array.
  JsArrayTypeUint32,
  // A float32 array.
  JsArrayTypeFloat32,
  // A float64 array.
  JsArrayTypeFloat64
} JsTypedArrayType;


// JSNI class and methods.

class GCCallbackInfo {
 public:
  // Get the runtime.
  JsRuntime GetRuntime(JSNIEnv* jsni_env) const;

  // Get the pointer be set in Set(Weak)GCCallback.
  void* GetInfo(JSNIEnv* jsni_env) const;

  // Get the env.
  JSNIEnv* Env() const;
};


class NativeCallbackInfo {
 public:
  // Arguments number.
  int Length(JSNIEnv* jsni_env) const;

  // Get the arguments.
  // TODO(jiny). Be more nice.
  // JsValue operator[](JSNIEnv* jsni_env, int i) const;
  JsValue Get(JSNIEnv* jsni_env, int i) const;

  // Get the callee function.
  JsFunction Callee(JSNIEnv* jsni_env) const;

  // Get this object.
  JsObject This(JSNIEnv* jsni_env) const;

  JsObject Holder(JSNIEnv* jsni_env) const;

  bool IsConstructCall(JSNIEnv* jsni_env) const;

  JsValue Data(JSNIEnv* jsni_env) const;

  // Get runtime.
  JsRuntime GetRuntime(JSNIEnv* jsni_env) const;

  // Set return value.
  void SetReturnValue(JSNIEnv* jsni_env, JsValue ret) const;

  static const int kHolderIndex = 0;
  static const int kIsolateIndex = 1;
  static const int kReturnValueDefaultValueIndex = 2;
  static const int kReturnValueIndex = 3;
  static const int kDataIndex = 4;
  static const int kCalleeIndex = 5;
  static const int kContextSaveIndex = 6;
};


class PropertyInfo {
 public:
  JsObject This(JSNIEnv* jsni_env) const;
  JsObject Holder(JSNIEnv* jsni_env) const;
  JsValue Data(JSNIEnv* jsni_env) const;
  JsRuntime GetRuntime(JSNIEnv* jsni_env) const;

  // Get the env.
  JSNIEnv* Env() const;

  void SetReturnValue(JSNIEnv* jsni_env, JsValue ret) const;

  // This shouldn't be public, but the arm compiler needs it.
  static const int kArgsLength = 5;
  static const int kHolderIndex = 0;
  static const int kRuntimeIndex = 1;
  static const int kThisIndex = 2;
  static const int kReturnValueIndex = 3;
  static const int kDataIndex = 4;

  explicit PropertyInfo(void** values) : values_(values) {}
  void** values_;
};


class JsRuntime {
 public:
  explicit JsRuntime(RuntimeRef rt);

  // New a runtime.
  // Not Implemented.
  static JsRuntime New(JSNIEnv* jsni_env);

  // Dispose the runtime.
  // Not Implemented.
  void Dispose(JSNIEnv* jsni_env);

  static JsRuntime GetCurrent(JSNIEnv* jsni_env);

 private:
  RuntimeRef rt_;
  friend class JsContext;
  friend class LocalRefScope;
};


class JsContext {
 public:
  explicit JsContext(ContextRef context);

  static JsContext GetCurrentContext(JSNIEnv* jsni_env, JsRuntime runtime);

  ContextRef Get() const { return context_; }

  // Not need new yet.
  // static JsContext New(JsRuntime runtime);

 private:
  ContextRef context_;
};


// The superclass of all JavaScript values and objects.
class JsValue {
 public:
  static JsValue Undefined(JSNIEnv* jsni_env);

  static JsValue Null(JSNIEnv* jsni_env);

  bool IsEmpty(JSNIEnv* jsni_env) const;

  // Returns true if this value is the undefined value.
  bool IsUndefined(JSNIEnv* jsni_env) const;

  // Returns true if this value is the null value.
  bool IsNull(JSNIEnv* jsni_env) const;

  // Returns true if this value is a string.
  bool IsString(JSNIEnv* jsni_env) const;

  // Returns true if this value is a function.
  bool IsFunction(JSNIEnv* jsni_env) const;

  // Returns true if this value is an array.
  bool IsArray(JSNIEnv* jsni_env) const;

  // Returns true if this value is a object.
  bool IsObject(JSNIEnv* jsni_env) const;

  // Returns true if this value is a number.
  bool IsNumber(JSNIEnv* jsni_env) const;

  // Returns true if this value is a boolean.
  bool IsBoolean(JSNIEnv* jsni_env) const;

  // Returns true if this value is a TypedArray.
  bool IsTypedArray(JSNIEnv* jsni_env) const;

  // These are quick path for primitive convertion.
  // Convert JsValue to native int.
  int ToInt(JSNIEnv* jsni_env) const;

  // Convert JsValue to native double.
  double ToDouble(JSNIEnv* jsni_env) const;

  // Convert JsValue to native bool.
  bool ToBool(JSNIEnv* jsni_env) const;

  // Cast as Object.
  JsObject AsObject(JSNIEnv* jsni_env) const;

  // Cast as Function.
  JsFunction AsFunction(JSNIEnv* jsni_env) const;

  // Cast as String.
  JsString AsString(JSNIEnv* jsni_env) const;

  explicit JsValue(JSNIEnv* jsni_env, int val);

  explicit JsValue(JSNIEnv* jsni_env, double val);

  explicit JsValue(JSNIEnv* jsni_env, bool val);

  JsValue();

  JsLocal Get() { return val_; }

 private:
  JsLocal val_;
  explicit JsValue(JsLocal val);
  friend class JsGlobalRef;
  friend class JsNumber;
  friend class JsBoolean;
  friend class JsObject;
  friend class JsFunction;
  friend class JsString;
  friend class JsTypedArray;
  friend class LocalRefScope;
};


class JsNumber : public JsValue {
 public:
  JsNumber() {}
  explicit JsNumber(JSNIEnv* jsni_env, double value);
  explicit JsNumber(JSNIEnv* jsni_env, int32_t value);
  explicit JsNumber(JSNIEnv* jsni_env, uint32_t value);
 private:
  explicit JsNumber(JsLocal val);
  friend class JsGlobalRef;
};


class JsBoolean : public JsValue {
 public:
  JsBoolean() {}
  explicit JsBoolean(JSNIEnv* jsni_env, bool value);
 private:
  explicit JsBoolean(JsLocal val);
  friend class JsGlobalRef;
};


class JsObject : public JsValue {
 public:
  JsObject();

  // Create an object.
  // Use New to Create an Object.
  static JsObject New(JSNIEnv* jsni_env);

  // Create an object with an internal field.
  // Use New... to avoid the ambiguity of val_ assignment.
  static JsObject NewWithInternalField(JSNIEnv* jsni_env);

  // Returns true if this object has an internal field.
  bool HasInternalField(JSNIEnv* jsni_env);

  // Set a pointer field for this object.
  void SetInternalField(JSNIEnv* jsni_env, void* field);

  // Get the pointer field in this object.
  void* GetInternalField(JSNIEnv* jsni_env);

  // Return true if this object has the property name.
  bool HasProperty(JSNIEnv* jsni_env, const char* name);

  // Get the property.
  JsValue GetProperty(JSNIEnv* jsni_env, const char* name);

  // Set the property.
  bool SetProperty(JSNIEnv* jsni_env, const char* name, JsValue property);

  // Delete the property.
  bool DeleteProperty(JSNIEnv* jsni_env, const char* name);

  // Set the getter and setter function call back for
  // the object.
  bool SetAccessor(JSNIEnv* jsni_env, const char* name,
                   GetterCallback getter,
                   SetterCallback setter = 0);

  // Get the object's prototype.
  JsValue GetPrototype(JSNIEnv* jsni_env);

  // Set the object's prototype.
  bool SetPrototype(JSNIEnv* jsni_env, JsValue prototype);

 private:
  explicit JsObject(JsLocal val);
  friend class JsValue;
  friend class JsGlobalRef;
  friend class JsFunction;
  friend class JsTypedArray;
};


// TODO(jiny). JsObject has a constructor. So use JsValue?
class JsFunction : public JsObject {
 public:
  JsFunction() {}
  explicit JsFunction(JSNIEnv* jsni_env, NativeFunctionCallback nativeFunc);

  // Call the function.
  // argc is the argument number, and
  // argv is the argument array or the pointer to the
  // first argument.
  JsValue Call(JSNIEnv* jsni_env, JsValue recv, int argc,
               JsValue* argv);
 private:
  explicit JsFunction(JsLocal val);
  friend class JsValue;
  friend class JsGlobalRef;
};


class JsString : public JsValue {
 public:
  JsString() {}
  explicit JsString(JSNIEnv* jsni_env, const char* src);

  // Get utf8 value. The chars are made copied.
  // Must call ReleaseUtf8Chars to free space.
  char* GetUtf8Chars(JSNIEnv* jsni_env);

  // Release the utf8 string.
  static void ReleaseUtf8Chars(JSNIEnv* jsni_env, char * str);

  // Get 16-bits string. The chars are made copied.
  // Must call ReleaseChars to free space.
  uint16_t* GetChars(JSNIEnv* jsni_env);

  // Release 16-bits string.
  static void ReleaseChars(JSNIEnv* jsni_env, uint16_t* str);

  // Get 16-bits string length.
  size_t Length(JSNIEnv* jsni_env);

  // Get utf8 length.
  size_t Utf8Length(JSNIEnv* jsni_env);

 private:
  explicit JsString(JsLocal val);
  friend class JsGlobalRef;
  friend class JsValue;
};


// Only implement the uint8array.
// TODO(jiny): implement others.
class JsTypedArray : public JsObject {
 public:
  JsTypedArray() {}

  // Only uint8 array implemented now.
  JsTypedArray(JSNIEnv* jsni_env, JsTypedArrayType type, char * data, size_t length);

  // Return the specific type of TypedArray.
  JsTypedArrayType Type(JSNIEnv* jsni_env);

  // Get number of elements.
  size_t Length(JSNIEnv* jsni_env);

  // Get the pointer of the TypedArray.
  void *Data(JSNIEnv* jsni_env);
 private:
  explicit JsTypedArray(JsLocal val);
  friend class JsValue;
  friend class JsGlobalRef;
};


class JsGlobalRef {
 public:
  JsGlobalRef();

  // Set a value to global reference.
  void Set(JSNIEnv* jsni_env, const JsValue val);

  // Clear the global reference pointed to the value.
  void Clear(JSNIEnv* jsni_env);

  // Return true if the global reference is empty.
  // TODO.(IsEmpty and empty) Could be removed from ENV. Use val_ instead.
  bool IsEmpty(JSNIEnv* jsni_env) const;

  // Set the global reference to empty.
  void Empty(JSNIEnv* jsni_env);

  // Convert the global reference to a local value.
  JsValue ToLocal(JSNIEnv* jsni_env);

  // Not Implemented for now. Use SetWeakGCCallback.
  void SetGCCallback(JSNIEnv* jsni_env, void* args, GCCallback callback);

  // Set a callback when the global object is garbage collected.
  // Note: the time of gc is not guaranteed.
  void SetWeakGCCallback(JSNIEnv* jsni_env, void* args, GCCallback callback);

 private:
  JsGlobal val_;
};


// TODO(Jiny): Add escape method. So we need another pointer:
// escaope_slot to store it.
// Note: It will create a handle internal.
class LocalRefScope {
 public:
  // TODO. Remove runtime and isolate-internal in the API.
  // Because we can get them from the env.
  LocalRefScope(JSNIEnv* jsni_env, JsRuntime runtime);
  ~LocalRefScope();
  JsValue SaveRef(JSNIEnv* jsni_env, JsValue val);

  // Return 0 if success.
  // Not implemented.
  static int PushLocalScope(JSNIEnv* jsni_env, JsRuntime runtime);
  // Return a escope JsValue for the given val.
  // Not implemented.
  static JsValue PopLocalScope(JSNIEnv* jsni_env, JsValue val);
 private:
  // scope_ contains the information of local ref.
  JsScope scope_;
  JSNIEnv* jsni_env_;
};


// Exception.
class JsException {
 public:
  // Throw an exception for an error.
  static void ThrowError(JSNIEnv* jsni_env, const char* errmsg);

  // Throw an exception for a type error.
  static void ThrowTypeError(JSNIEnv* jsni_env, const char* errmsg);

  // Throw an exception for a range error.
  static void ThrowRangeError(JSNIEnv* jsni_env, const char* errmsg);
};


// Table of interface method pointers.
struct JSNINativeInterface {
  // TODO(jiny). Should have reserved.
  uint32_t (*GetVersion)();
  bool (*RegisterMethod)(JSNIEnv*, const JsValue,
                         const char*,
                         NativeFunctionCallback);

  // JsValue
  JsValue (*Undefined)();
  JsValue (*Null)();
  bool (JsValue::*IsEmpty)() const;
  bool (JsValue::*IsUndefined)() const;
  bool (JsValue::*IsNull)() const;
  bool (JsValue::*IsString)() const;
  bool (JsValue::*IsFunction)() const;
  bool (JsValue::*IsArray)() const;
  bool (JsValue::*IsObject)() const;
  bool (JsValue::*IsNumber)() const;
  bool (JsValue::*IsBoolean)() const;
  bool (JsValue::*IsTypedArray)() const;
  int (JsValue::*ToInt)() const;
  double (JsValue::*ToDouble)() const;
  bool (JsValue::*ToBool)() const;
  JsObject (JsValue::*AsObject)() const;
  JsFunction (JsValue::*AsFunction)() const;
  JsString (JsValue::*AsString)() const;
  JsValue (*JsValueNewInt)(int);
  JsValue (*JsValueNewDou)(double);
  JsValue (*JsValueNewBoo)(bool);

  // JsNumber
  JsNumber (*JsNumberNewDou)(double);
  JsNumber (*JsNumberNewInt)(int32_t);
  JsNumber (*JsNumberNewUint)(uint32_t);

  // JsBoolean
  JsBoolean (*JsBooleanNew)(bool value);

  // JsObject
  JsObject (*JsObjectNew)();
  JsObject (*JsObjectNewWithInternalField)();
  bool (JsObject::*HasInternalField)();
  void (JsObject::*SetInternalField)(void*);
  void* (JsObject::*GetInternalField)();
  bool (JsObject::*HasProperty)(const char*);
  JsValue (JsObject::*GetProperty)(const char*);
  bool (JsObject::*SetProperty)(const char*, JsValue);
  bool (JsObject::*DeleteProperty)(const char*);
  bool (JsObject::*SetAccessor)(const char*,
                   GetterCallback,
                   SetterCallback);
  JsValue (JsObject::*GetPrototype)();
  bool (JsObject::*SetPrototype)(JsValue);

  // JsFunction
  JsFunction (*JsFunctionNew)(NativeFunctionCallback);
  JsValue (JsFunction::*Call)(JsValue, int, JsValue*);

  // JsString
  JsString (*JsStringNew)(const char*); // static
  char* (JsString::*GetUtf8Chars)();
  void (*ReleaseUtf8Chars)(char*); // static
  uint16_t* (JsString::*GetChars)();
  void (*ReleaseChars)(uint16_t*); // static
  size_t (JsString::*JsStringLength)();
  size_t (JsString::*JsStringUtf8Length)();

  // JsTypedArray
  JsTypedArray (*JsTypedArrayNew)(JsTypedArrayType, char*, size_t);
  JsTypedArrayType (JsTypedArray::*Type)();
  size_t (JsTypedArray::*JsTypedArrayLength)();
  void* (JsTypedArray::*JsTypedArrayData)();

  // JsGlobalRef
  void (JsGlobalRef::*JsGlobalRefSet)(const JsValue);
  void (JsGlobalRef::*JsGlobalRefClear)();
  bool (JsGlobalRef::*JsGlobalRefIsEmpty)() const;
  void (JsGlobalRef::*JsGlobalRefEmpty)();
  JsValue (JsGlobalRef::*JsGlobalRefToLocal)();
  void (JsGlobalRef::*SetGCCallback)(void*, GCCallback);
  void (JsGlobalRef::*SetWeakGCCallback)(void*, GCCallback);

  // LocalRefScope
  void (LocalRefScope::*LocalRefScopeNew)(JsRuntime runtime);
  void (LocalRefScope::*LocalRefScopeDeconstruct)();
  JsValue (LocalRefScope::*SaveRef)(JsValue);

  // JsException
  void (*ThrowError)(const char*);
  void (*ThrowTypeError)(const char*);
  void (*ThrowRangeError)(const char*);

  // NativeCallbackInfo
  int (NativeCallbackInfo::*NativeCallbackInfoLength)() const;
  JsValue (NativeCallbackInfo::*NativeCallbackInfoGet)(int) const;
  JsFunction (NativeCallbackInfo::*NativeCallbackInfoCallee)() const;
  JsObject (NativeCallbackInfo::*NativeCallbackInfoThis)() const;
  JsObject (NativeCallbackInfo::*NativeCallbackInfoHolder)() const;
  bool (NativeCallbackInfo::*NativeCallbackInfoIsConstructCall)() const;
  JsValue (NativeCallbackInfo::*NativeCallbackInfoData)() const;
  JsRuntime (NativeCallbackInfo::*NativeCallbackInfoGetRuntime)() const;
  void (NativeCallbackInfo::*NativeCallbackInfoSetReturnValue)(JsValue) const;

  // GCCallbackInfo
  JsRuntime (GCCallbackInfo::*GCCallbackInfoGetRuntime)() const;
  void* (GCCallbackInfo::*GCCallbackInfoGetInfo)() const;
  JSNIEnv* (GCCallbackInfo::*GCCallbackInfoEnv)() const;

  // PropertyInfo
  JsObject (PropertyInfo::*PropertyInfoThis)() const;
  JsObject (PropertyInfo::*PropertyInfoHolder)() const;
  JsValue (PropertyInfo::*PropertyInfoData)() const;
  JsRuntime (PropertyInfo::*PropertyInfoGetRuntime)() const;
  JSNIEnv* (PropertyInfo::*PropertyInfoEnv)() const;
  void (PropertyInfo::*PropertyInfoSetReturnValue)(JsValue) const;

  // JsRuntime
  JsRuntime (*JsRuntimeGetCurrent)();

  // JsContext
  JsContext (*GetCurrentContext)(JsRuntime);
  // ..Many many function pointer.
};



/*===========================
 * Implementation.
=============================*/

// --- Static functions. ---
class JSNI {
 public:
  // Get the version of JSNI.
  static uint32_t GetVersion(JSNIEnv* jsni_env) {
    return jsni_env->GetVersion();
  }

  // Register native method.
  // TODO(jiny): To recv, maybe we should use (JsLocal)void* instead of JsValue.
  static bool RegisterMethod(JSNIEnv* jsni_env, const JsValue recv,
                             const char* name,
                             NativeFunctionCallback callback) {
    return jsni_env->RegisterMethod(jsni_env, recv, name, callback);
  }
};


// JsValue.
JsValue::JsValue(JSNIEnv* jsni_env, int val) {
    *this = jsni_env->JsValueNewInt(val);
}

JsValue::JsValue(JSNIEnv* jsni_env, double val) {
    *this = jsni_env->JsValueNewDou(val);
}

JsValue::JsValue(JSNIEnv* jsni_env, bool val) {
    *this = jsni_env->JsValueNewBoo(val);
}

JsValue::JsValue() : val_(0) {
}

JsValue JsValue::Undefined(JSNIEnv* jsni_env) {
  return jsni_env->Undefined();
}
// Returns true if this value is the null value.
JsValue JsValue::Null(JSNIEnv* jsni_env) {
  return jsni_env->Null();
}

bool JsValue::IsEmpty(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->IsEmpty)();
}

// Returns true if this value is the undefined value.
bool JsValue::IsUndefined(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->IsUndefined)();
}

// Returns true if this value is the null value.
bool JsValue::IsNull(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->IsNull)();
}

// Returns true if this value is a string.
bool JsValue::IsString(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->IsString)();
}

// Returns true if this value is a function.
bool JsValue::IsFunction(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->IsFunction)();
}

// Returns true if this value is an array.
bool JsValue::IsArray(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->IsArray)();
}

// Returns true if this value is a object.
bool JsValue::IsObject(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->IsObject)();
}

// Returns true if this value is a number.
bool JsValue::IsNumber(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->IsNumber)();
}

// Returns true if this value is a boolean.
bool JsValue::IsBoolean(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->IsBoolean)();
}

// Returns true if this value is a TypedArray.
bool JsValue::IsTypedArray(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->IsTypedArray)();
}

// These are quick path for primitive convertion.
// Convert JsValue to native int.
int JsValue::ToInt(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->ToInt)();
}

// Convert JsValue to native double.
double JsValue::ToDouble(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->ToDouble)();
}

// Convert JsValue to native bool.
bool JsValue::ToBool(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->ToBool)();
}

// Cast as Object.
JsObject JsValue::AsObject(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->AsObject)();
}

// Cast as Function.
JsFunction JsValue::AsFunction(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->AsFunction)();
}

// Cast as String.
JsString JsValue::AsString(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->AsString)();
}


// JsNumber
JsNumber::JsNumber(JSNIEnv* jsni_env, double val) {
  *this = jsni_env->JsNumberNewDou(val);
}

JsNumber::JsNumber(JSNIEnv* jsni_env, int32_t val) {
  *this = jsni_env->JsNumberNewInt(val);
}

JsNumber::JsNumber(JSNIEnv* jsni_env, uint32_t val) {
  *this = jsni_env->JsNumberNewUint(val);
}


// JsBoolean
JsBoolean::JsBoolean(JSNIEnv* jsni_env, bool val) {
  *this = jsni_env->JsBooleanNew(val);
}


// JsObject
JsObject::JsObject() {}

JsObject JsObject::New(JSNIEnv* jsni_env) {
  return jsni_env->JsObjectNew();
}

// Use New... to avoid the ambiguity of val_ assignment.
JsObject JsObject::NewWithInternalField(JSNIEnv* jsni_env) {
  return jsni_env->JsObjectNewWithInternalField();
}

bool JsObject::HasInternalField(JSNIEnv* jsni_env) {
  return (this->*jsni_env->HasInternalField)();
}

void JsObject::SetInternalField(JSNIEnv* jsni_env, void* field) {
  return (this->*jsni_env->SetInternalField)(field);
}

void* JsObject::GetInternalField(JSNIEnv* jsni_env) {
  return (this->*jsni_env->GetInternalField)();
}

bool JsObject::HasProperty(JSNIEnv* jsni_env, const char* name) {
  return (this->*jsni_env->HasProperty)(name);
}

JsValue JsObject::GetProperty(JSNIEnv* jsni_env, const char* name) {
  return (this->*jsni_env->GetProperty)(name);
}

bool JsObject::SetProperty(JSNIEnv* jsni_env, const char* name, JsValue property) {
  return (this->*jsni_env->SetProperty)(name, property);
}

bool JsObject::DeleteProperty(JSNIEnv* jsni_env, const char* name) {
  return (this->*jsni_env->DeleteProperty)(name);
}

bool JsObject::SetAccessor(JSNIEnv* jsni_env, const char* name,
                 GetterCallback getter,
                 SetterCallback setter) {
  return (this->*jsni_env->SetAccessor)(name, getter, setter);
}

JsValue JsObject::GetPrototype(JSNIEnv* jsni_env) {
  return (this->*jsni_env->GetPrototype)();
}

bool JsObject::SetPrototype(JSNIEnv* jsni_env, JsValue prototype) {
  return (this->*jsni_env->SetPrototype)(prototype);
}


// JsFunction
JsFunction::JsFunction(JSNIEnv* jsni_env, NativeFunctionCallback nativeFunc) {
  *this = jsni_env->JsFunctionNew(nativeFunc);
}

JsValue JsFunction::Call(JSNIEnv* jsni_env, JsValue recv, int argc,
             JsValue* argv) {
  return (this->*jsni_env->Call)(recv, argc, argv);
}


// JsString
JsString::JsString(JSNIEnv* jsni_env, const char* src) {
  *this = jsni_env->JsStringNew(src);
}

char* JsString::GetUtf8Chars(JSNIEnv* jsni_env) {
  return (this->*jsni_env->GetUtf8Chars)();
}

void JsString::ReleaseUtf8Chars(JSNIEnv* jsni_env, char* str) {
  return jsni_env->ReleaseUtf8Chars(str);
}

uint16_t* JsString::GetChars(JSNIEnv* jsni_env) {
  return (this->*jsni_env->GetChars)();
}

void JsString::ReleaseChars(JSNIEnv* jsni_env, uint16_t* str) {
  return jsni_env->ReleaseChars(str);
}

size_t JsString::Length(JSNIEnv* jsni_env) {
  return (this->*jsni_env->JsStringLength)();
}

size_t JsString::Utf8Length(JSNIEnv* jsni_env) {
  return (this->*jsni_env->JsStringUtf8Length)();
}


// JsTypedArray
JsTypedArray::JsTypedArray(JSNIEnv* jsni_env, JsTypedArrayType type,
                           char* data,
                           size_t length) {
  *this = jsni_env->JsTypedArrayNew(type, data, length);
}

JsTypedArrayType JsTypedArray::Type(JSNIEnv* jsni_env) {
  return (this->*jsni_env->Type)();
}

// In typed array, length is number of elements.
size_t JsTypedArray::Length(JSNIEnv* jsni_env) {
  return (this->*jsni_env->JsTypedArrayLength)();
}

void* JsTypedArray::Data(JSNIEnv* jsni_env) {
  return (this->*jsni_env->JsTypedArrayData)();
}


// JsGlobalRef
JsGlobalRef::JsGlobalRef() :val_(0) {}

void JsGlobalRef::Set(JSNIEnv* jsni_env, JsValue ref) {
  return (this->*jsni_env->JsGlobalRefSet)(ref);
}

void JsGlobalRef::Clear(JSNIEnv* jsni_env) {
  return (this->*jsni_env->JsGlobalRefClear)();
}

bool JsGlobalRef::IsEmpty(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->JsGlobalRefIsEmpty)();
}

// Set the global reference to empty.
void JsGlobalRef::Empty(JSNIEnv* jsni_env) {
  return (this->*jsni_env->JsGlobalRefEmpty)();
}

JsValue JsGlobalRef::ToLocal(JSNIEnv* jsni_env) {
  return (this->*jsni_env->JsGlobalRefToLocal)();
}

void JsGlobalRef::SetWeakGCCallback(JSNIEnv* jsni_env, void* args, GCCallback callback) {
  return (this->*jsni_env->SetWeakGCCallback)(args, callback);

}


// LocalRefScope
// TODO(Jiny). Dulplicate of jsni_env and runtime.
// But should be done after we fix jsni_env pointer.
LocalRefScope::LocalRefScope(JSNIEnv* jsni_env, JsRuntime runtime) {
  // Set jsni_env so decon can use it.
  jsni_env_ = jsni_env;
  (this->*jsni_env->LocalRefScopeNew)(runtime);
}

LocalRefScope::~LocalRefScope() {
  // Use internal jsni_env_.
  (this->*jsni_env_->LocalRefScopeDeconstruct)();
}

JsValue LocalRefScope::SaveRef(JSNIEnv* jsni_env, JsValue val) {
   return (this->*jsni_env->SaveRef)(val);
}

void JsException::ThrowError(JSNIEnv* jsni_env, const char* errmsg) {
  jsni_env->ThrowError(errmsg);
}

void JsException::ThrowTypeError(JSNIEnv* jsni_env, const char* errmsg) {
  jsni_env->ThrowTypeError(errmsg);
}

void JsException::ThrowRangeError(JSNIEnv* jsni_env, const char* errmsg) {
  jsni_env->ThrowRangeError(errmsg);
}


// NativeCallbackInfo
// Arguments number.
int NativeCallbackInfo::Length(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->NativeCallbackInfoLength)();
}

// Get the arguments.
JsValue NativeCallbackInfo::Get(JSNIEnv* jsni_env, int i) const{
  return (this->*jsni_env->NativeCallbackInfoGet)(i);
}

// Get the callee function.
JsFunction NativeCallbackInfo::Callee(JSNIEnv* jsni_env) const{
  return (this->*jsni_env->NativeCallbackInfoCallee)();
}

// Get this object.
JsObject NativeCallbackInfo::This(JSNIEnv* jsni_env) const{
  return (this->*jsni_env->NativeCallbackInfoThis)();
}

JsObject NativeCallbackInfo::Holder(JSNIEnv* jsni_env) const{
  return (this->*jsni_env->NativeCallbackInfoHolder)();
}

bool NativeCallbackInfo::IsConstructCall(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->NativeCallbackInfoIsConstructCall)();
}

JsValue NativeCallbackInfo::Data(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->NativeCallbackInfoData)();
}

// Get runtime.
JsRuntime NativeCallbackInfo::GetRuntime(JSNIEnv* jsni_env) const{
  return (this->*jsni_env->NativeCallbackInfoGetRuntime)();
}

// Set return value.
void NativeCallbackInfo::SetReturnValue(JSNIEnv* jsni_env, JsValue ret) const{
  return (this->*jsni_env->NativeCallbackInfoSetReturnValue)(ret);
}


// GCCallbackInfo
JsRuntime GCCallbackInfo::GetRuntime(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->GCCallbackInfoGetRuntime)();
}

void* GCCallbackInfo::GetInfo(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->GCCallbackInfoGetInfo)();
}


// PropertyInfo
JsObject PropertyInfo::This(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->PropertyInfoThis)();
}

JsObject PropertyInfo::Holder(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->PropertyInfoHolder)();
}

JsValue PropertyInfo::Data(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->PropertyInfoData)();
}

JsRuntime PropertyInfo::GetRuntime(JSNIEnv* jsni_env) const {
  return (this->*jsni_env->PropertyInfoGetRuntime)();
}

void PropertyInfo::SetReturnValue(JSNIEnv* jsni_env, JsValue ret) const {
  return (this->*jsni_env->PropertyInfoSetReturnValue)(ret);
}


// JsRuntime
JsRuntime::JsRuntime(RuntimeRef rt)
    : rt_(rt) {}

JsRuntime JsRuntime::GetCurrent(JSNIEnv* jsni_env) {
  return jsni_env->JsRuntimeGetCurrent();
}


// JsContext
JsContext::JsContext(ContextRef context)
    : context_(context) {}

JsContext JsContext::GetCurrentContext(JSNIEnv* jsni_env, JsRuntime runtime) {
  return jsni_env->GetCurrentContext(runtime);
}

}  // namespace jsni

// JSNI versions.
#define JSNI_VERSION_1_0 0x00010000

#ifdef  __cplusplus
extern "C" {
#endif
// This function is exported by loadable shared libs. Just declare here.
// TODO(jiny): To recv, maybe we should use (JsLocal)void* instead of JsValue.
// Return jsni version.
int JSNI_Init(jsni::JSNIEnv* jsni_env, const jsni::JsValue recv);

// This function is exported by loadable shared libs. Just declare here.
// Not implemented.
int JSNI_UnInit(jsni::JSNIEnv* jsni_env, const jsni::JsValue recv, void* reserved);
#ifdef  __cplusplus
}
#endif

#endif
