#include <node.h>

#include "realmobject.h"

using namespace v8;

Persistent<Function> RealmObject::constructor;

<<<<<<< Updated upstream
RealmObject::RealmObject() {}
||||||| merged common ancestors
RealmObject::RealmObject(realm::Object& target):
        wrapped(target)
{
}
=======
RealmObject::RealmObject() : m_object(nullptr)
{
}
>>>>>>> Stashed changes

RealmObject::~RealmObject() {
    delete m_object;
}

Local<RealmObject> RealmObject::Create(realm::Object *target) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    EscapableHandleScope handle_scope(isolate);
    //Local<RealmObject> object = Object::New(isolate, target);
    return handle_scope.Escape(object);
}

void RealmObject::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, RealmObject::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "RealmObject"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "RealmObject"), tpl->GetFunction());
}

<<<<<<< Updated upstream
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
||||||| merged common ancestors
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
=======
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
    obj->m_object->set_property_value(nullptr, key, info.Data(), true);
}
>>>>>>> Stashed changes
