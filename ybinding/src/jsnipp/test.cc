#include "jsnipp.h"

using namespace jsnipp;

// native function
JSValue hello(JSObject, JSArray args) {
    return JSString("Hello, world");
}

// native object
class Test {
public:
    JSValue func(JSObject recv, JSArray args) {
        return JSNull();
    }
};

// native constructor
class Echo {
public:
    // constuctor
    Echo(JSObject, JSArray args) {
        if (args.length() > 0)
            prefix_ = JSString(args[0]);
        else
            prefix_ = "???: ";
    }
    // method
    JSValue echo(JSObject, JSArray args) {
        std::string str;
        if (args.length() > 0)
            str = JSString(args[0]);
        else
            str = "";
        return JSString(prefix_ + str);
    }
    // getter
    JSValue prefix(JSObject) {
        return JSString(prefix_);
    }
    // initializer
    static void setup(JSObject cls) {
        cls.setProperty("echo", JSNativeMethod<Echo, &Echo::echo>());
        cls.defineProperty("prefix", JSPropertyAccessor(JSNativeGetter<Echo, &Echo::prefix>()));
    }
private:
    std::string prefix_;
};

// JSNI Entry point
__attribute__ ((visibility("default")))
int JSNI_Init(JSNIEnv* env, JsValue exports) {
    JSValue::setup(env);
    JSObject obj(exports);

    // register native function
    obj.setProperty("hello", JSNativeFunction<hello>());

    // register native constructor
    obj.setProperty("Echo", JSNativeConstructor<Echo>(&Echo::setup));
    obj.setProperty("Echo2", JSNativeConstructor<Echo>({
        {"echo", JSNativeMethod<Echo, &Echo::echo>()},
        {"string", "hello"_js},
    }));

    // register native object
    Test* native = new Test();
    obj.setProperty("object", JSNativeObject<Test>(native, {
        {"func", JSNativeMethod<Test, &Test::func>()},
        {"str", "hello"_js},
        {"num", 1.1_js},
        {"bool", true_js},
        {"nil", null_js}
    }));

    // register array
    //auto array = JSArray { 1.1, 2.2, 3.3 };
    auto array = JSArray(0, true, 1.1, "hello");
    obj.setProperty("anArray", array);

    // register object
    JSObject object{
        {"asdf", "asdf"_js},
        {"1234", 1.1_js}
    };
    obj.setProperty("anObject", object);

    return JSNI_VERSION_1_1;
}


int main() {
    JSException<Error>("asdf");
    JSException<TypeError>::checkAndClear();
    return 0;
}
