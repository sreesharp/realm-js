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

realm::Property ParseProperty(Isolate* iso, Local<Value> propertyAttributes, std::string propertyName, ObjectDefaults &objectDefaults) {
    realm::Property prop;

    prop.name = propertyName;

    Local<Object> propertyObject;
    std::string type;

    if (propertyAttributes->IsObject()) {
        propertyObject = ValidatedValueToObject(iso, propertyAttributes);
        type = ValidatedStringProperty(iso, propertyObject, ToString(iso, "type"));

        Local<Value> optionalValue = propertyObject->Get(ToString(iso, "optional"));
        if (!optionalValue->IsUndefined()) {
            if (!optionalValue->IsBoolean()) {
                throw std::runtime_error("'optional' designation expected to be of type boolean");
            }
            prop.is_nullable = ValidatedValueToBool(optionalValue);
        }
    }
    else {
        type = ValidatedStringForValue(iso, propertyAttributes);
    }

    if (type == "bool") {
        prop.type = PropertyTypeBool;
    }
    else if (type == "int") {
        prop.type = PropertyTypeInt;
    }
    else if (type == "float") {
        prop.type = PropertyTypeFloat;
    }
    else if (type == "double") {
        prop.type = PropertyTypeDouble;
    }
    else if (type == "string") {
        prop.type = PropertyTypeString;
    }
    else if (type == "date") {
        prop.type = PropertyTypeDate;
    }
    else if (type == "data") {
        prop.type = PropertyTypeData;
    }
    else if (type == "list") {
        if (propertyObject->IsUndefined() || propertyObject->IsNull()) {
            throw std::runtime_error("List property must specify 'objectType'");
        }
        prop.type = PropertyTypeArray;
        prop.object_type =  ValidatedStringProperty(iso, propertyObject, ToString(iso, "objectType"));
    }
    else {
        prop.type = PropertyTypeObject;
        prop.is_nullable = true;

        // The type could either be 'object' or the name of another object type in the same schema.
        if (type == "object") {
            if (propertyObject->IsUndefined() || propertyObject->IsNull()) {
                throw std::runtime_error("Object property must specify 'objectType'");
            }
            prop.object_type = ValidatedStringProperty(iso, propertyObject, ToString(iso, "objectType"));
        }
        else {
            prop.object_type = type;
        }
    }

    if (!propertyObject->IsUndefined()) {
        Local<Value> defaultValue = ValidatedPropertyValue(iso, propertyObject, ToString(iso, "default"));
        if (!defaultValue->IsUndefined()) {
            // FIXME: JSValueProtect(ctx, defaultValue)
            objectDefaults.emplace(prop.name, defaultValue);
        }
    }

    return prop;
}

realm::ObjectSchema ParseObjectSchema(Isolate* iso, Local<Object> jsonObjectSchema, std::map<std::string, realm::ObjectDefaults> &defaults, std::map<std::string, Local<Value>> &prototypes) {
    Local<Object> prototypeObject;
    Local<Object> objectSchemaObject;

    Local<Value> prototypeValue = jsonObjectSchema->Get(ToString(iso, "prototype"));
    if (!prototypeValue->IsUndefined()) {
        prototypeObject = prototypeValue->ToObject();
        objectSchemaObject = prototypeObject->Get(ToString(iso, "schema"))->ToObject();
    }
    else {
        objectSchemaObject = jsonObjectSchema->ToObject();
    }

    ObjectDefaults objectDefaults;
    ObjectSchema objectSchema;
    objectSchema.name = *(String::Utf8Value(objectSchemaObject->Get(ToString(iso, "name"))->ToString()));

    Local<Value> propertiesObject = objectSchemaObject->Get(ToString(iso, "properties"));
    if (propertiesObject->IsArray()) {
        v8::Array *properties = v8::Array::Cast(*propertiesObject);
        size_t numProperties = properties->Length();
        for (unsigned int i = 0; i < numProperties; i++) {
            Local<Object> propertyObject = Local<Object>::Cast(properties->Get(i));
            std::string propertyName = ValidatedStringProperty(iso, propertyObject, ToString(iso, "name"));
            objectSchema.properties.emplace_back(ParseProperty(iso, propertyObject, propertyName, objectDefaults));
        }
    }
    else {
        Local<v8::Array> propertyNames = propertiesObject->ToObject()->GetPropertyNames();
        size_t propertyCount = propertyNames->Length();
        for (size_t i = 0; i < propertyCount; i++) {
            Local<Value> propertyName = propertyNames->Get(i);
            Local<Value> propertyValue = ValidatedPropertyValue(iso, propertiesObject, propertyName->ToString());
            objectSchema.properties.emplace_back(ParseProperty(iso, propertyValue, ToString(propertyName), objectDefaults));
        }
    }

    Local<Value> primaryValue = objectSchemaObject->Get(ToString(iso, "primaryKey"));
    if (!primaryValue->IsUndefined()) {
        objectSchema.primary_key = ToString(primaryValue->ToString());
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

    defaults.emplace(objectSchema.name, std::move(objectDefaults));

    return objectSchema;
}

realm::Schema RealmSchemaWrap::ParseSchema(Isolate* iso, Local<Value> value, std::map<std::string, realm::ObjectDefaults> &defaults, std::map<std::string, Local<Value>> &prototypes) {
    std::vector<realm::ObjectSchema> schema;
    size_t length = ValidatedArrayLength(*value);
    v8::Array *array = v8::Array::Cast(*value);
    for (unsigned int i = 0; i < length; i++) {
        Local<Object> jsonObjectSchema = Local<Object>::Cast(array->Get(i));
        ObjectSchema objectSchema = ParseObjectSchema(iso, jsonObjectSchema, defaults, prototypes);
        schema.emplace_back(std::move(objectSchema));
    }

    return realm::Schema(schema);
}
