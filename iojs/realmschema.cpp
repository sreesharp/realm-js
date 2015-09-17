
#include "realmschema.hpp"

using namespace v8;

RealmSchema::RealmSchema() {}

RealmSchema::~RealmSchema() {}

Persistent<Function> RealmSchema::constructor;

void RealmSchema::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, RealmSchema::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "Schema"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // FIXME: addNotification

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "Schema"), tpl->GetFunction());
}

void RealmSchema::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (args.IsConstructCall()) {
        // Invoked as constructor: `new Realm(...)`
        args.GetReturnValue().Set(args.This());
    } else {
        // TODO: Invoked as plain function `Realm(...)`, turn into construct call.
    }
}

realm::Schema RealmSchema::ParseSchema(const v8::Local<v8::Value> &value) {
    return realm::Schema(std::vector<realm::ObjectSchema>());
}
