#include <node.h>

#include "realmarray.h"

using namespace v8;

Persistent<Function> RealmArray::constructor;

RealmArray::RealmArray() {}

RealmArray::~RealmArray() {}

void RealmArray::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, RealmArray::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "RealmArray"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "RealmArray"), tpl->GetFunction());
}

void RealmArray::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (args.IsConstructCall()) {
        // Invoked as constructor: `new RealmArray(...)`
        args.GetReturnValue().Set(args.This());
    } else {
        // TODO: Invoked as plain function `RealmArray(...)`, turn into construct call.
    }
}
