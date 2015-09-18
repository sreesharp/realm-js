#include "realmobject.h"
#include "realmutils.hpp"
#include "realmschema.hpp"

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
    auto tpl = ObjectTemplate::New(isolate);

    //tpl->SetClassName(String::NewFromUtf8(isolate, "RealmObject"));
    tpl->SetInternalFieldCount(1);
    tpl->SetNamedPropertyHandler(RealmObject::Get, RealmObject::Set);

    Local<Object> obj = tpl->NewInstance();
    obj->SetInternalField(0, External::New(isolate, new RealmObject(target)));
    return obj;
}

void RealmObject::Get(v8::Local<v8::String> name,
        const v8::PropertyCallbackInfo<v8::Value>& info)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    realm::Object *object = GetObject(info.Holder());
    realm::Property *prop = object->object_schema.property_for_name(ToString(name));
    if (!prop) {
        return;
    }

    switch (prop->type) {
        case realm::PropertyTypeBool:
            info.GetReturnValue().Set(v8::Boolean::New(isolate, object->row.get_bool(prop->table_column)));
            break;
        case realm::PropertyTypeInt:
            info.GetReturnValue().Set(v8::Integer::New(isolate, object->row.get_int(prop->table_column)));
            break;
        case realm::PropertyTypeFloat:
            info.GetReturnValue().Set(v8::Number::New(isolate, object->row.get_float(prop->table_column)));
            break;
        case realm::PropertyTypeDouble:
            info.GetReturnValue().Set(v8::Number::New(isolate, object->row.get_double(prop->table_column)));
            break;
        case realm::PropertyTypeString:
            info.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, static_cast<std::string>(object->row.get_string(prop->table_column)).c_str()));
            break;
        case realm::PropertyTypeDate: {
            info.GetReturnValue().Set(v8::Date::New(isolate, object->row.get_datetime(prop->table_column).get_datetime()));
            break;
        }
        case realm::PropertyTypeData:
            info.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, static_cast<std::string>(object->row.get_binary(prop->table_column)).c_str()));
            break;
            /*
        case realm::PropertyTypeAny:
            *exception = RJSMakeError(ctx, "'Any' type not supported");
            return NULL;
            break;
        case realm::PropertyTypeObject: {
            auto linkObjectSchema = obj->realm->config().schema->find(prop->object_type);
            TableRef table = ObjectStore::table_for_object_type(obj->realm->read_group(), linkObjectSchema->name);
            if (obj->row.is_null_link(prop->table_column)) {
                return JSValueMakeNull(ctx);
            }
            return RJSObjectCreate(ctx, Object(obj->realm, *linkObjectSchema, table->get(obj->row.get_link(prop->table_column))));
            break;
        }
        case realm::PropertyTypeArray: {
            auto arrayObjectSchema = obj->realm->config().schema->find(prop->object_type);
            return RJSArrayCreate(ctx, new ObjectArray(obj->realm, *arrayObjectSchema, static_cast<LinkViewRef>(obj->row.get_linklist(prop->table_column))));
            break;
        }*/
            default:
            break;
    }
}

realm::Object *RealmObject::GetObject(Local<Object> self) {
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    return static_cast<RealmObject *>(wrap->Value())->m_object;
}

using NullType = std::nullptr_t;
using ValueType = Local<Value>;
using Accessor = realm::NativeAccessor<ValueType, NullType>;

void RealmObject::Set(Local<String> name, v8::Local<v8::Value> value,
        const PropertyCallbackInfo<Value>& info)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    std::string key = *(String::Utf8Value(name));
    GetObject(info.Holder())->set_property_value<ValueType, NullType>(nullptr, key, value, true);

    //    RealmObject* obj = ObjectWrap::Unwrap<RealmObject>(info.This());
}

namespace realm {

template<> bool Accessor::dict_has_value_for_key(NullType ctx, ValueType dict, const std::string &prop_name) {
    return dict->ToObject()->Has(ToString(Isolate::GetCurrent(), prop_name.c_str()));
}

template<> ValueType Accessor::dict_value_for_key(NullType ctx, ValueType dict, const std::string &prop_name) {
    return dict->ToObject()->Get(ToString(Isolate::GetCurrent(), prop_name.c_str()));
}

template<> bool Accessor::has_default_value_for_property(NullType ctx, const ObjectSchema &object_schema, const std::string &prop_name) {
    ObjectDefaults &defaults = RealmSchema::DefaultsForClassName(object_schema.name);
    return defaults.find(prop_name) != defaults.end();
}

template<> ValueType Accessor::default_value_for_property(NullType ctx, const ObjectSchema &object_schema, const std::string &prop_name) {
    ObjectDefaults &defaults = RealmSchema::DefaultsForClassName(object_schema.name);
    return defaults[prop_name];
}

template<> bool Accessor::is_null(NullType ctx, ValueType &val) {
    return val->IsUndefined() || val->IsNull();
}

template<> bool Accessor::to_bool(NullType ctx, ValueType &val) {
    return *val->ToBoolean();
}

template<> long long Accessor::to_long(NullType ctx, ValueType &val) {
    return val->ToInteger()->Value();
}

template<> float Accessor::to_float(NullType ctx, ValueType &val) {
    return val->ToNumber()->Value();
}

template<> double Accessor::to_double(NullType ctx, ValueType &val) {
    return val->ToNumber()->Value();
}

template<> std::string Accessor::to_string(NullType ctx, ValueType &val) {
    return ToString(val);
}

template<> DateTime Accessor::to_datetime(NullType ctx, ValueType &val) {
    Isolate* isolate = Isolate::GetCurrent();

    Handle<v8::Object> obj = val->ToObject();
    Handle<Function> getTimeFunc = Handle<Function>::Cast(obj->Get(String::NewFromUtf8(isolate, "getTime")));

    Handle<Value> funcResult = getTimeFunc->Call(obj, 0, NULL);
    return DateTime(funcResult->ToInteger()->Value());
}

template<> size_t Accessor::to_object_index(NullType ctx, SharedRealm &realm, ValueType &val, std::string &type, bool try_update) {
    if (val->IsObject()) {
        RealmObject::GetObject(val->ToObject())->row.get_index();
    }

    auto object_schema = realm->config().schema->find(type);
    Object child = Object::create<ValueType>(ctx, realm, *object_schema, val, try_update);
    return child.row.get_index();
}

template<> size_t Accessor::array_size(NullType ctx, ValueType &val) {
    return ValidatedArrayLength(*val);
}

template<> ValueType Accessor::array_value_at_index(NullType ctx, ValueType &val, size_t index) {
    v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(val);
    return array->Get(index);
}

} // namespace realm


