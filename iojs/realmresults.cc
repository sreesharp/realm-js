#include <node.h>

#include "realmresults.h"

using namespace v8;

Persistent<Function> RealmResults::constructor;

RealmResults::RealmResults() {}

RealmResults::~RealmResults() {}

void RealmResults::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, RealmResults::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "RealmResults"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "RealmResults"), tpl->GetFunction());
}

void RealmResults::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (args.IsConstructCall()) {
        // Invoked as constructor: `new RealmResults(...)`
        args.GetReturnValue().Set(args.This());
    } else {
        // TODO: Invoked as plain function `RealmResults(...)`, turn into construct call.
    }
}
