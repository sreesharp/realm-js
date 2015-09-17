#include "realmobject.h"

#include "object_store.hpp"
#include "object_accessor.hpp"

using namespace v8;

Persistent<Function> RealmObject::constructor;

Local<ObjectTemplate> RealmObject::s_template;

RealmObject::RealmObject(realm::Object *target) : m_object(target) {
}

RealmObject::~RealmObject() {
    delete m_object;
}

void RealmObject::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    s_template = ObjectTemplate::New(isolate);
    //s_template->SetClassName(String::NewFromUtf8(isolate, "RealmObject"));
    s_template->SetInternalFieldCount(1);

    s_template->SetNamedPropertyHandler(RealmObject::Get, RealmObject::Set);

    //constructor.Reset(isolate, s_template->GetFunction());
    //exports->Set(String::NewFromUtf8(isolate, "RealmResults"), s_template->GetFunction());
}

Local<Object> RealmObject::Create(realm::Object *target) {
    Isolate* isolate = Isolate::GetCurrent();
    Local<Object> obj = s_template->NewInstance();
    obj->SetInternalField(0, External::New(isolate, new RealmObject(target)));
    return obj;
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

    std::string key = *(String::Utf8Value(name));

    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    RealmObject *obj = static_cast<RealmObject *>(wrap->Value());
    obj->m_object->set_property_value(nullptr, key, info.Data(), true);

    //    RealmObject* obj = ObjectWrap::Unwrap<RealmObject>(info.This());

}


