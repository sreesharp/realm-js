#include <node.h>

#include "realm.h"

using namespace v8;

Persistent<Function> Realm::constructor;

Realm::Realm() {}

Realm::~Realm() {}

void Realm::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, Realm::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "Realm"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    NODE_SET_PROTOTYPE_METHOD(tpl, "objects",     Realm::Objects);
    NODE_SET_PROTOTYPE_METHOD(tpl, "create",      Realm::Create);
    NODE_SET_PROTOTYPE_METHOD(tpl, "delete",      Realm::Delete);
    NODE_SET_PROTOTYPE_METHOD(tpl, "deleteAll",   Realm::DeleteAll);
    NODE_SET_PROTOTYPE_METHOD(tpl, "write",       Realm::Write);
    // FIXME: addNotification

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "Realm"), tpl->GetFunction());
}

void Realm::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (args.IsConstructCall()) {
        // Invoked as constructor: `new Realm(...)`
        args.GetReturnValue().Set(args.This());
    } else {
        // TODO: Invoked as plain function `Realm(...)`, turn into construct call.
    }
}

void Realm::Objects(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

}

void Realm::Create(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

}

void Realm::Delete(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

}

void Realm::DeleteAll(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

}

void Realm::Write(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

}
