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

    // Methods
    NODE_SET_PROTOTYPE_METHOD(tpl, "push",     RealmArray::Push);
    NODE_SET_PROTOTYPE_METHOD(tpl, "pop",      RealmArray::Pop);
    NODE_SET_PROTOTYPE_METHOD(tpl, "shift",    RealmArray::Shift);
    NODE_SET_PROTOTYPE_METHOD(tpl, "unshift",  RealmArray::Unshift);
    NODE_SET_PROTOTYPE_METHOD(tpl, "splice",   RealmArray::Splice);


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

void RealmArray::Push(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

}

void RealmArray::Pop(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

}

void RealmArray::Shift(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

}

void RealmArray::Unshift(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

}

void RealmArray::Splice(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

}
