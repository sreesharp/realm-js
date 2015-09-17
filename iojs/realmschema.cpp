
#include "realmschema.hpp"

using namespace v8;
using namespace realm;

RealmSchema::RealmSchema() {}

RealmSchema::~RealmSchema() {}

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

static std::map<std::string, ObjectDefaults> s_defaults;
ObjectDefaults &RealmSchema::DefaultsForClassName(const std::string &className) {
    return s_defaults[className];
}

static std::map<std::string, Local<Value>> s_prototypes;
Local<Value> RealmSchema::PrototypeForClassName(const std::string &className) {
    return s_prototypes[className];
}

size_t ValidatedArrayLength(Value *value) {
    if (!value->IsArray()) {
        throw std::runtime_error("value is not an array");
    }
    v8::Array *array = v8::Array::Cast(value);
    return array->Length();
}

realm::Property ParseProperty(Local<Object> jsonProperty) {
    realm::Property property;

    return property;
}

realm::ObjectSchema ParseObjectSchema(Local<Object> jsonObjectSchema) {
    Isolate *iso = Isolate::GetCurrent();

    Local<Object> prototypeObject;
    Local<Object> objectSchemaObject;

    Local<Value> prototypeValue = jsonObjectSchema->Get(String::NewFromUtf8(iso, "prototype"));
    if (!prototypeValue->IsUndefined()) {
        prototypeObject = prototypeValue->ToObject();
        objectSchemaObject = prototypeObject->Get(String::NewFromUtf8(iso, "schema"))->ToObject();
    }
    else {
        objectSchemaObject = jsonObjectSchema->Get(String::NewFromUtf8(iso, "schema"))->ToObject();
    }
    Local<Value> propertiesObject = objectSchemaObject->Get(String::NewFromUtf8(iso, "properties"));

    ObjectSchema objectSchema;
    ObjectDefaults defaults;
    objectSchema.name = *(String::Utf8Value(objectSchemaObject->Get(String::NewFromUtf8(iso, "name"))->ToString()));

    size_t numProperties = ValidatedArrayLength(*propertiesObject);
    v8::Array *properties = v8::Array::Cast(*propertiesObject);
    for (unsigned int p = 0; p < numProperties; p++) {
        Local<Object> property = properties->CloneElementAt(p);
        objectSchema.properties.emplace_back(ParseProperty(property));

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

realm::Schema RealmSchema::ParseSchema(Value *value) {
    std::vector<realm::ObjectSchema> schema;
    size_t length = ValidatedArrayLength(value);
    v8::Array *array = v8::Array::Cast(value);
    for (unsigned int i = 0; i < length; i++) {
        Local<Object> jsonObjectSchema = array->CloneElementAt(i);
        ObjectSchema objectSchema = ParseObjectSchema(jsonObjectSchema);
        schema.emplace_back(std::move(objectSchema));
    }

    return realm::Schema(schema);
}
