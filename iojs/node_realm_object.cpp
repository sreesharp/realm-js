/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential.
 */

#include <node.h>

#include "object_store.hpp"
#include "object_accessor.hpp"

#include "util.hpp"
#include "node_realm.hpp"
#include "node_realm_object.hpp"
#include "node_realm_schema.hpp"

using namespace v8;

Persistent<Function> RealmObjectWrap::constructor;
RealmObjectWrap::RealmObjectWrap() : m_object(0) {}
RealmObjectWrap::~RealmObjectWrap() {}

void RealmObjectWrap::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, RealmObjectWrap::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "RealmObject"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    tpl->InstanceTemplate()->SetNamedPropertyHandler(RealmObjectWrap::Get, RealmObjectWrap::Set);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "RealmObject"), tpl->GetFunction());
}

void RealmObjectWrap::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);
    if (args.IsConstructCall()) {
        // Invoked as constructor: `new RealmResults(...)`
        try {
            ValidateArgumentCount(args.Length(), 0);
            RealmObjectWrap* row = new RealmObjectWrap();       
            row->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
            return;
        }
        catch (std::exception& ex) {
            makeError(iso, ex.what());
            args.GetReturnValue().SetUndefined();
            return;
        }
    }
    else {
        // FIXME: Invoked as plain function `RealmObjects(...)`, turn into construct call.
    }
}

// Create a new Realm object. Typically done by realm.createObject()
Handle<Object> RealmObjectWrap::Create(Isolate* ctx, realm::Object* object) {
    EscapableHandleScope scope(ctx);
    
    Local<Value> prototype = NodePrototypes(object->realm().get())[object->get_object_schema().name];
    Local<Function> cons = Local<Function>::New(ctx, constructor);
    Handle<Object> obj = cons->NewInstance(0, nullptr);
    
    obj->SetPrototype(prototype);
    RealmObjectWrap* row = RealmObjectWrap::Unwrap<RealmObjectWrap>(obj);
    row->m_object = object;
    
    return scope.Escape(obj);
}

using NullType = std::nullptr_t;
using ValueType = Local<Value>;
using IsolateType = Isolate*;
using Accessor = realm::NativeAccessor<ValueType, IsolateType>;

void RealmObjectWrap::Get(Local<String> name, const PropertyCallbackInfo<Value>& info) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        RealmObjectWrap* row = ObjectWrap::Unwrap<RealmObjectWrap>(info.This());
        auto object = row->m_object;
        info.GetReturnValue().Set(object->get_property_value<ValueType, IsolateType>(iso, ToString(name)));
    } catch (realm::InvalidPropertyException &ex) {
        // getters and setters for nonexistent properties in JS should always return undefined
    } catch (std::exception &ex) {
        makeError(iso, ex.what());
        info.GetReturnValue().SetUndefined();
    }
}

void RealmObjectWrap::Set(Local<String> name, Local<Value> value, const PropertyCallbackInfo<Value>& info) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        RealmObjectWrap* row = ObjectWrap::Unwrap<RealmObjectWrap>(info.This());
        auto object = row->m_object;
        object->set_property_value<ValueType, IsolateType>(iso, ToString(name), value, true);
    } catch (std::exception& ex) {
        makeError(iso, ex);
    }
    info.GetReturnValue().SetUndefined();
}

namespace realm {

template<> bool Accessor::dict_has_value_for_key(IsolateType ctx, ValueType dict, const std::string &prop_name) {
    Local<v8::Object> object = ValidatedValueToObject(ctx, dict);
    return object->Has(ToString(ctx, prop_name.c_str()));
}

template<> ValueType Accessor::dict_value_for_key(IsolateType ctx, ValueType dict, const std::string &prop_name) {
    Local<v8::Object> object = ValidatedValueToObject(ctx, dict);
    return dict->ToObject()->Get(ToString(ctx, prop_name.c_str()));
}

template<> bool Accessor::has_default_value_for_property(IsolateType ctx, Realm* realm, const ObjectSchema &object_schema, const std::string &prop_name) {
    ObjectDefaults &defaults = RealmSchemaWrap::DefaultsForClassName(object_schema.name);
    return defaults.find(prop_name) != defaults.end();
}

template<> ValueType Accessor::default_value_for_property(IsolateType ctx, Realm* realm, const ObjectSchema &object_schema, const std::string &prop_name) {
    ObjectDefaults &defaults = RealmSchemaWrap::DefaultsForClassName(object_schema.name);
    return defaults[prop_name];
}

template<> bool Accessor::is_null(IsolateType ctx, ValueType &val) {
    return val->IsUndefined() || val->IsNull();
}

template<> ValueType Accessor::null_value(IsolateType ctx) {
    return Null(ctx);
}

template<> bool Accessor::to_bool(IsolateType ctx, ValueType &val) {
    if (!val->IsBoolean()) {
        throw std::runtime_error("Property expected to be of type boolean");
    }
    return val->ToBoolean()->Value();
}

template<> ValueType Accessor::from_bool(IsolateType ctx, bool b) {
    return Boolean::New(ctx, b);
}

template<> long long Accessor::to_long(IsolateType ctx, ValueType &val) {
    return val->ToInteger()->Value();
}

template<> ValueType Accessor::from_long(IsolateType ctx, long long l) {
    return Integer::New(ctx, l);
}

template<> float Accessor::to_float(IsolateType ctx, ValueType &val) {
    if (!val->IsNumber()) { // FIXME: better check for floats
        throw std::runtime_error("Property expected to be of type float");
    }
    return val->ToNumber()->Value();
}

template<> ValueType Accessor::from_float(IsolateType ctx, float f) {
    return Number::New(ctx, f);
}

template<> double Accessor::to_double(IsolateType ctx, ValueType &val) {
    if (!val->IsNumber()) { // FIXME: better check for doubles
        throw std::runtime_error("Property expected to be of type double");
    }
    return val->ToNumber()->Value();
}

template<> ValueType Accessor::from_double(IsolateType ctx, double d) {
    return Number::New(ctx, d);
}

template<> std::string Accessor::to_string(IsolateType ctx, ValueType &val) {
    return ToString(val);
}

template<> ValueType Accessor::from_string(IsolateType ctx, StringData s) {
    return ToString(ctx, s.data());
}

template<> DateTime Accessor::to_datetime(IsolateType ctx, ValueType &val) {
    Handle<v8::Object> obj = val->ToObject();
    Handle<Function> getTimeFunc = Handle<Function>::Cast(obj->Get(String::NewFromUtf8(ctx, "getTime")));

    Handle<Value> funcResult = getTimeFunc->Call(obj, 0, NULL);
    return DateTime(funcResult->ToInteger()->Value());
}

template<> size_t Accessor::to_object_index(IsolateType ctx, SharedRealm realm, ValueType &val, const std::string &type, bool try_update) {
    if (val->IsObject() && val->ToObject()->InternalFieldCount() > 0) {
        RealmObjectWrap::GetObject(val->ToObject())->row().get_index();
    }

    auto object_schema = realm->config().schema->find(type);
    Object child = Object::create<ValueType, IsolateType>(ctx, realm, *object_schema, val, try_update);
    return child.row().get_index();
}

template<> size_t Accessor::list_size(IsolateType ctx, ValueType &val) {
    return ValidatedArrayLength(*val);
}

template<> ValueType Accessor::list_value_at_index(IsolateType ctx, ValueType &val, size_t index) {
    Local<v8::Array> array = Local<v8::Array>::Cast(val);
    return array->Get(index);
}

template<> ValueType Accessor::from_list(IsolateType ctx, List list) {
    // FIXME: implement
}

} // namespace realm
