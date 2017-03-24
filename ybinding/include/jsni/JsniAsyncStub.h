#ifndef JSNIASYNCSTUB_H
#define JSNIASYNCSTUB_H
#include <memory>
#include "JsniStub.h"

namespace jsni {
class JsniAsyncStubImpl;
class y_async_t;
class JsniAsyncStub : public JsniStub {
protected:
  JsniAsyncStub();
  void postMessage(void *);
  virtual void onMessage(void *) = 0;
  virtual ~JsniAsyncStub() {}

private:
  static void callback(y_async_t *);
  std::shared_ptr<JsniAsyncStubImpl> pimpl;
};
}

#endif
