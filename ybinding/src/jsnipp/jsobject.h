/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#include <cassert>
#include <functional>
#include <map>
#include <string>

#include <jsni.h>

#include "jsvalue.h"
#include "jsprimitive.h"

namespace jsnipp {

typedef std::initializer_list<std::pair<std::string, JSValue>> JSPropertyList;

class JSPropertyDescriptor;

class JSObject : public JSValue {
public:
    JSObject(): JSValue(env_->NewObject()) {}
    JSObject(std::nullptr_t): JSValue(env_->NewNull()) {}
    JSObject(JSPropertyList list): JSObject() {
        for (auto& p: list) {
            setProperty(p.first.c_str(), p.second);
        }
    }

    JSObject(const JSValue& jsval);
    JSObject(JsValue jsval): JSValue(jsval) {
        assert(is_object());
    }

    JSObject prototype() const {
        return JSObject(env_->GetPrototype(jsval_));
    }

    JSString toString() const {
        JSValue jsfunc = getProperty("toString");
        return env_->CallFunction(jsfunc, jsval_, 0, NULL);
    }

    JSValue getProperty(const std::string& key) const {
        return from(env_->GetProperty(jsval_, key.c_str()));
    }
    bool setProperty(const std::string& key, JSValue jsval) {
        return env_->SetProperty(jsval_, key.c_str(), jsval);
    }
    bool hasProperty(const std::string& key) const {
        return env_->HasProperty(jsval_, key.c_str());
    }
    bool deleteProperty(const std::string& key) {
        return env_->DeleteProperty(jsval_, key.c_str());
    }
    void defineProperty(const std::string& name, JSPropertyDescriptor descriptor);

    JSValue operator [](const std::string& key) const {
        return getProperty(key);
    }

protected:
    JSObject(int cnt):
        JSValue(env_->NewObjectWithHiddenField(cnt)) {}
};

}

#include "jsfunction.h"

namespace jsnipp {

class JSPropertyDescriptor : public JSObject {
public:
    JSPropertyDescriptor(JSValue value, bool writable = false,
                         bool configurable = false, bool enumerable = false):
        JSPropertyDescriptor(configurable, enumerable) {
        setProperty("value", value);
        setProperty("writable", JSBoolean(writable));
    }
    JSPropertyDescriptor(JSFunction getter,
                         bool configurable = false, bool enumerable = false):
        JSPropertyDescriptor(configurable, enumerable) {
        setProperty("get", getter);
    }
    JSPropertyDescriptor(JSFunction getter, JSFunction setter,
                         bool configurable = false, bool enumerable = false):
        JSPropertyDescriptor(getter, configurable, enumerable) {
        setProperty("set", setter);
    }

private:
    JSPropertyDescriptor(bool configurable, bool enumerable): JSObject() {
        setProperty("configurable", JSBoolean(configurable));
        setProperty("enumerable", JSBoolean(enumerable));
    }
};

}

namespace jsnipp {

namespace {
constexpr int native_slot = 0;
}

template<class C>
class JSNativeObject : public JSObject {
public:
    JSNativeObject(C* native, bool managed, std::function<void(JSObject)> setup):
        JSObject(native_slot + 1) {
        reset(native, managed);
        if (setup)  setup(*this);
    }

    JSNativeObject(C* native, bool managed = true,
                   std::function<void(C&, JSObject)> setup = nullptr):
        JSNativeObject(native, managed, std::bind(setup, std::ref(*native), std::placeholders::_1)){}

    JSNativeObject(C* native, JSPropertyList list):
        JSNativeObject(native, true, std::bind([](JSPropertyList list, JSObject obj){
            for (auto& p: list)  obj.setProperty(p.first, p.second);
        }, list, std::placeholders::_1)){}

    JSNativeObject(JsValue jsval): JSObject(jsval) {
        assert(is_object() && env_->HiddenFieldCount(jsval_) > native_slot);
    }

    C* operator ->() const {
        return native();
    }
    C* native() const {
        uintptr_t ptr = reinterpret_cast<uintptr_t>(getPrivate(native_slot));
        return reinterpret_cast<C*>(ptr & ~1);
    }
    void reset(C* native, bool managed = true) {
#define is_managed(ptr) (!(ptr & 1) ? false : true)
        uintptr_t ptr = reinterpret_cast<uintptr_t>(getPrivate(native_slot));
        if (ptr) {
            C* old = reinterpret_cast<C*>(ptr & ~1);
            if (old != native && is_managed(ptr))
                delete old;
        }

        ptr = (uintptr_t)native | (uintptr_t)(managed && native);
        setPrivate(native_slot, (void*)ptr);
        if (is_managed(ptr)) {
            env_->SetGCCallback(jsval_, jsval_, [](JSNIEnv*, void* data){
                JSNativeObject<C> jsobj(reinterpret_cast<JsValue>(data));
                jsobj.reset(nullptr);
            });
        }
    }

    void* getPrivate(int index) const {
        if (index >= env_->HiddenFieldCount(jsval_))
            return nullptr;
        return env_->GetHiddenField(jsval_, index);
    }
    void setPrivate(int index, void* ptr) {
        assert(index < env_->HiddenFieldCount(jsval_));
        return env_->SetHiddenField(jsval_, index, ptr);
    }
};

template <class C>
using JSMethodType = JSValue (C::*)(JSObject, JSArray);

template <class C, JSMethodType<C> method>
class JSNativeMethod : public JSFunction {
public:
    JSNativeMethod() :
        JSFunction([](JSNIEnv* env, const CallbackInfo info){
            assert(env == env_);
            JSNativeObject<C> self(env->GetThis(info));
            C* native = self.native();
            if (native) {
                JsValue result = (native->*method)(self, info);
                env->SetReturnValue(info, result);
            } else {
                //TODO: throw an exception
            }
        }){}
};

template <class C>
using JSGetterType = JSValue (C::*)();

template <class C, JSGetterType<C> getter>
class JSNativeGetter : public JSFunction {
public:
    JSNativeGetter() :
        JSFunction([](JSNIEnv* env, const CallbackInfo info){
            assert(env == env_);
            JSNativeObject<C> self(env->GetThis(info));
            C* native = self.native();
            if (native) {
                env->SetReturnValue(info, (native->*getter)());
            } else {
                //TODO: throw an exception
            }
        }){}
};

template <class C>
using JSSetterType = void (C::*)(JSValue);

template <class C, JSSetterType<C> setter>
class JSNativeSetter : public JSFunction {
public:
    JSNativeSetter() :
        JSFunction([](JSNIEnv* env, const CallbackInfo info){
            assert(env == env_);
            JSNativeObject<C> self(env->GetThis(info));
            C* native = self.native();
            if (native) {
                (native->*setter)(env->GetArg(info, 0));
            } else {
                //TODO: throw an exception
            }
        }){}
};

// FYI: http://stackoverflow.com/questions/15148749/pointer-to-class-member-as-a-template-parameter
// for template argument deduction.

}
