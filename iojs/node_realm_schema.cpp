/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential.
 */

#include <node.h>

#include "object_store.hpp"
#include "schema.hpp"

#include "node_realm_schema.hpp"
#include "util.hpp"

using namespace v8;
using namespace realm;

Persistent<Function> RealmSchemaWrap::constructor;
RealmSchemaWrap::RealmSchemaWrap() {}
RealmSchemaWrap::~RealmSchemaWrap() {}

void RealmSchemaWrap::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, RealmSchemaWrap::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "RealmSchema"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "RealmSchema"), tpl->GetFunction());
}

void RealmSchemaWrap::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    if (args.IsConstructCall()) {
        if (args.Length() == 0) {
            RealmSchemaWrap* rsw = new RealmSchemaWrap();
            rsw->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
            return;
        }
        else {
            makeError(iso, "Wrong number of arguments");
            return;
        }
    } else {
        // FIXME: Invoked as plain function `RealmSchema(...)`, turn into construct call.
    }
}

static std::map<std::string, ObjectDefaults> s_defaults;
ObjectDefaults &RealmSchemaWrap::DefaultsForClassName(const std::string &className) {
    return s_defaults[className];
}

static std::map<std::string, Local<Value>> s_prototypes;
Local<Value> RealmSchemaWrap::PrototypeForClassName(const std::string &className) {
    return s_prototypes[className];
}

realm::Property ParseProperty(Isolate* iso, Local<Object> jsonProperty) {
    realm::Property prop;

    prop.name = ToString(jsonProperty->Get(ToString(iso, "name")));
    std::string type = ToString(jsonProperty->Get(ToString(iso, "type")));

    if (type == "RealmTypeBool") {
        prop.type = PropertyTypeBool;
    }
    else if (type == "RealmTypeInt") {
        prop.type = PropertyTypeInt;
    }
    else if (type == "RealmTypeFloat") {
        prop.type = PropertyTypeFloat;
    }
    else if (type == "RealmTypeDouble") {
        prop.type = PropertyTypeDouble;
    }
    else if (type == "RealmTypeString") {
        prop.type = PropertyTypeString;
    }
    else if (type == "RealmTypeDate") {
        prop.type = PropertyTypeDate;
    }
    else if (type == "RealmTypeData") {
        prop.type = PropertyTypeData;
    }
    else if (type == "RealmTypeObject") {
        prop.type = PropertyTypeObject;
        prop.object_type =  ToString(jsonProperty->Get(ToString(iso, "objectType")));
        prop.is_nullable = true;
    }
    else if (type == "RealmTypeArray") {
        prop.type = PropertyTypeArray;
        prop.object_type = ToString(jsonProperty->Get(ToString(iso, "objectType")));
    }
    else {
        prop.type = PropertyTypeObject;
        prop.object_type = type;
        prop.is_nullable = true;
    }
    return prop;
}

realm::ObjectSchema ParseObjectSchema(Isolate* iso, Local<Object> jsonObjectSchema) {
    Local<Object> prototypeObject;
    Local<Object> objectSchemaObject;

    Local<Value> prototypeValue = jsonObjectSchema->Get(String::NewFromUtf8(iso, "prototype"));
    if (!prototypeValue->IsUndefined()) {
        prototypeObject = prototypeValue->ToObject();
        objectSchemaObject = prototypeObject->Get(String::NewFromUtf8(iso, "schema"))->ToObject();
    }
    else {
        objectSchemaObject = jsonObjectSchema->ToObject();
    }
    Local<Value> propertiesObject = objectSchemaObject->Get(String::NewFromUtf8(iso, "properties"));

    ObjectSchema objectSchema;
    ObjectDefaults defaults;
    objectSchema.name = *(String::Utf8Value(objectSchemaObject->Get(String::NewFromUtf8(iso, "name"))->ToString()));

    size_t numProperties = ValidatedArrayLength(*propertiesObject);
    v8::Array *properties = v8::Array::Cast(*propertiesObject);
    for (unsigned int p = 0; p < numProperties; p++) {
        Local<Object> property = Local<Object>::Cast(properties->Get(p));
        objectSchema.properties.emplace_back(ParseProperty(iso, property));

        Local<Value> defaultValue = property->Get(String::NewFromUtf8(iso, "default"));
        if (!defaultValue->IsUndefined()) {
            defaults.emplace(objectSchema.properties.back().name, defaultValue);
        }
    }
    s_defaults.emplace(objectSchema.name, std::move(defaults));

    Local<Value> primaryValue = objectSchemaObject->Get(String::NewFromUtf8(iso, "primaryKey"));
    if (!primaryValue->IsUndefined()) {
        objectSchema.primary_key = *(String::Utf8Value(primaryValue->ToString()));
        Property *property = objectSchema.primary_key_property();
        if (!property) {
            throw std::runtime_error("Missing primary key property '" + objectSchema.primary_key + "'");
        }
        property->is_primary = true;
    }

    // store prototype
    if (*prototypeObject) {
        s_prototypes[objectSchema.name] = std::move(prototypeObject);
    }

    return objectSchema;
}

realm::Schema RealmSchemaWrap::ParseSchema(Isolate* iso, Value *value) {
    std::vector<realm::ObjectSchema> schema;
    size_t length = ValidatedArrayLength(value);
    v8::Array *array = v8::Array::Cast(value);
    for (unsigned int i = 0; i < length; i++) {
        Local<Object> jsonObjectSchema = Local<Object>::Cast(array->Get(i));
        ObjectSchema objectSchema = ParseObjectSchema(iso, jsonObjectSchema);
        schema.push_back(objectSchema);
    }

    return realm::Schema(schema);
}
