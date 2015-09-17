#include "realmobject.h"

#include "object_store.hpp"
#include "object_accessor.hpp"

using namespace v8;

Persistent<Function> RealmObject::constructor;

RealmObject::RealmObject(realm::Object& target):
        wrapped(target)
{
}

RealmObject::~RealmObject() {}

void RealmObject::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<ObjectTemplate> result = ObjectTemplate::New(isolate);
    result->SetInternalFieldCount(0);
    result->SetNamedPropertyHandler(RealmObject::Get, RealmObject::Set);
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

void RealmObject::Get(v8::Local<v8::String> name,
        const v8::PropertyCallbackInfo<v8::Value>& info)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

}

void RealmObject::Set(Local<String> name, v8::Local<v8::Value> value,
        const PropertyCallbackInfo<Value>& info)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    RealmObject* obj = ObjectWrap::Unwrap<RealmObject>(info.This());
    std::string key = *(String::Utf8Value(name));
    obj->wrapped.set_property_value(nullptr, key, info.Data(), true);
}
