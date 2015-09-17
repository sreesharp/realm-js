#include <node.h>

#include "realmobject.h"

using namespace v8;

Persistent<Function> RealmObject::constructor;

RealmObject::RealmObject() {}

RealmObject::~RealmObject() {}

void RealmObject::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, RealmObject::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "RealmObject"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "RealmObject"), tpl->GetFunction());
}

void RealmObject::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (args.IsConstructCall()) {
        // Invoked as constructor: `new RealmObject(...)`
        args.GetReturnValue().Set(args.This());
    } else {
        // TODO: Invoked as plain function `RealmObject(...)`, turn into construct call.
    }
}