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
            args.GetReturnValue().SetUndefined();
            return;
        }
    } else {
        // FIXME: Invoked as plain function `RealmSchema(...)`, turn into construct call.
    }
}

realm::Property ParseProperty(Isolate* iso, Local<Value> propertyAttributes, std::string propertyName, ObjectDefaults &objectDefaults) {
    realm::Property prop;
    prop.name = propertyName;

    Local<Object> propertyObject;
    std::string type;

    if (propertyAttributes->IsObject()) {
        propertyObject = propertyAttributes->ToObject();
        if (!propertyObject->Has(ToString(iso, "type"))) {
            throw new std::runtime_error("Does not have 'type' attribute");
        }
        type = ToString(propertyObject->Get(ToString(iso, "type")));
        if (propertyObject->Has(ToString(iso, "optional"))) {
            Local<Value> optionalValue = propertyObject->Get(ToString(iso, "optional"));
            if (optionalValue->IsBoolean()) {
                prop.is_nullable = optionalValue->BooleanValue();
            }
            else {
                throw new std::runtime_error("'optional' designation expected to be of type boolean");
            }
        }
    }
    else {
        type = ToString(propertyAttributes->ToString());
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
            throw new std::runtime_error("List property must specify 'objectType'");
        }
        prop.type = PropertyTypeArray;
        if (propertyObject->Has(ToString(iso, "objectType"))) {
            prop.object_type = ToString(propertyObject->Get(ToString(iso, "objectType"))->ToString());
        }
        else {
            throw new std::runtime_error("Does not have 'objectType'");
        }
    }
    else {
        prop.type = PropertyTypeObject;
        prop.is_nullable = true;

        // The type could either be 'object' or the name of another object type in the same schema.
        if (type == "object") {
            if (propertyObject->IsUndefined() || propertyObject->IsNull()) {
                throw std::runtime_error("Object property must specify 'objectType'");
            }
            prop.object_type = ToString(propertyObject->Get(ToString(iso, "objectType")));
        }
        else {
            prop.object_type = type;
        }
    }

    if (!propertyObject->IsUndefined() || !propertyObject->IsNull()) {
        if (propertyObject->Has(ToString(iso, "default"))) {
            Local<Value> defaultValue = propertyObject->Get(ToString(iso, "default"));
            // FIXME: JSValueProtect(ctx, defaultValue)
            Persistent<Value> p_object;
            p_object.Reset(iso, defaultValue);
            objectDefaults.emplace(prop.name, defaultValue);
        }
    }

    return prop;
}

realm::ObjectSchema ParseObjectSchema(Isolate* iso, Local<Object> objectSchemaObject, std::map<std::string, realm::ObjectDefaults> &defaults, std::map<std::string, Local<Value>> &prototypes) {
    Local<Object> prototypeObject;

    if (objectSchemaObject->Has(ToString(iso, "prototype"))) {
        Local<Value> prototypeValue = objectSchemaObject->Get(ToString(iso, "prototype"));
        if (!prototypeValue->IsObject()) {
            throw new std::runtime_error("Not an object.");
        }
        prototypeObject = prototypeValue->ToObject();
        if (!prototypeObject->Has(ToString(iso, "schema"))) {
            throw new std::runtime_error("Realm object prototype must have a 'schema' property.");
        }
        objectSchemaObject = prototypeObject->Get(ToString(iso, "schema"))->ToObject();
    }
    else {
        if (objectSchemaObject->Has(ToString(iso, "schema"))) {
            Local<Value> subSchemaValue = objectSchemaObject->Get(ToString(iso, "schema"));
            objectSchemaObject = subSchemaValue->ToObject();
        }
    }

    ObjectDefaults objectDefaults;
    ObjectSchema objectSchema;
    objectSchema.name = ToString(objectSchemaObject->Get(ToString(iso, "name")));

    if (!objectSchemaObject->Has(ToString(iso, "properties"))) {
        throw new std::runtime_error("ObjectSchema must have a 'properties' object.");
    }
    Local<Value> propertiesObject = objectSchemaObject->Get(ToString(iso, "properties"));
    if (propertiesObject->IsArray()) {
        v8::Array* array = v8::Array::Cast(*propertiesObject);
        size_t numProperties = array->Length();
        for (size_t i = 0; i < numProperties; i++) {
            Local<Object> propertyObject = array->Get((unsigned int)i)->ToObject();
            std::string propertyName = ToString(propertyObject->Get(ToString(iso, "name")));
            objectSchema.properties.emplace_back(ParseProperty(iso, propertyObject, propertyName, objectDefaults));
        }
    }
    else {
        Local<v8::Array> propertyNames = propertiesObject->ToObject()->GetPropertyNames();
        size_t propertyCount = propertyNames->Length();
        for (size_t i = 0; i < propertyCount; i++) {
            Local<Value> propertyName = propertyNames->Get((unsigned int)i);
            Local<Value> propertyValue = propertiesObject->ToObject()->Get(propertyName->ToString());
            objectSchema.properties.emplace_back(ParseProperty(iso, propertyValue, ToString(propertyName), objectDefaults));
        }
    }

    if (objectSchemaObject->Has(ToString(iso, "primaryKey"))) {
        Local<Value> primaryValue = objectSchemaObject->Get(ToString(iso, "primaryKey"));
        objectSchema.primary_key = ToString(primaryValue->ToString());
        Property *property = objectSchema.primary_key_property();
        if (!property) {
            throw std::runtime_error("Missing primary key property '" + objectSchema.primary_key + "'");
        }
        property->is_primary = true;
    }

    // store prototype so that objects of this type will have their prototype set to this prototype object.
    if (*prototypeObject) {
        // FIXME: protect prototypeObject
        Persistent<Object> p_object;
        p_object.Reset(iso, prototypeObject);
        prototypes[objectSchema.name] = std::move(prototypeObject);
    }

    defaults.emplace(objectSchema.name, std::move(objectDefaults));

    return objectSchema;
}

realm::Schema RealmSchemaWrap::ParseSchema(Isolate* iso, Local<Value> value, std::map<std::string, realm::ObjectDefaults> &defaults, std::map<std::string, Local<Value>> &prototypes) {
    std::vector<realm::ObjectSchema> schema;
    if (!value->IsArray()) {
        throw std::runtime_error("Not an array.");
    }
    v8::Array *array = v8::Array::Cast(*value);
    size_t length = array->Length();
    for (unsigned int i = 0; i < length; i++) {
        Local<Object> jsonObjectSchema = array->Get(i)->ToObject();  // FIXME: is jsonObjectSchema an Object?
        ObjectSchema objectSchema = ParseObjectSchema(iso, jsonObjectSchema, defaults, prototypes);
        schema.emplace_back(std::move(objectSchema));
    }

    return realm::Schema(schema);
}
