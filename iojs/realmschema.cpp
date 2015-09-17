
#include "realmschema.hpp"

using namespace v8;

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

size_t ValidatedArrayLength(Value &value) {
    if (!value.IsArray()) {
        throw std::runtime_error("value is not an array");
    }
    Array *array = Array::Cast(&value);
    return array->Length();
}

realm::objectSchema ParseObjectSchema(Local<Object> jsonObjectSchema) {
    Local<Object> prototypeObject = NULL;
    Local<Value> prototypeValue = jsonObjectSchema.Get("prototype");
    if (!prototypeValue.IsUndefined()) {
        prototypeObject = prototypeValue.ToObject();
        objectSchemaObject = prototypeObject.Get("schema");
    }
    else {
        objectSchemaObject = jsonObjectSchema.Get("schema");
    }
    Local<Value> propertiesObject = objectSchemaObject.Get("properties").ToObject();

    ObjectSchema objectSchema;
    ObjectDefaults defaults;
    objectSchema.name = objectSchemaObject.Get("name").ToString();

    size_t numProperties = ValidatedArrayLength(propertiesObject);
    Array *properties = Array::Cast(&propertiesObject);
    for (unsigned int p = 0; p < numProperties; p++) {
        Local<Object> property = properties.CloneElementAt(i);
        objectSchema.properties.emplace_back(ParseProperty(property));
/*
        static JSStringRef defaultString = JSStringCreateWithUTF8CString("default");
        JSValueRef defaultValue = JSObjectGetProperty(ctx, property, defaultString, NULL);
        if (!JSValueIsUndefined(ctx, defaultValue)) {
            JSValueProtect(ctx, defaultValue);
            defaults.emplace(objectSchema.properties.back().name, defaultValue);
        }*/
    }
/*    s_defaults.emplace(objectSchema.name, std::move(defaults));

    static JSStringRef primaryString = JSStringCreateWithUTF8CString("primaryKey");
    JSValueRef primaryValue = RJSValidatedPropertyValue(ctx, objectSchemaObject, primaryString);
    if (!JSValueIsUndefined(ctx, primaryValue)) {
        objectSchema.primary_key = RJSValidatedStringForValue(ctx, primaryValue);
        Property *property = objectSchema.primary_key_property();
        if (!property) {
            throw std::runtime_error("Missing primary key property '" + objectSchema.primary_key + "'");
        }
        property->is_primary = true;
    }*/

    // store prototype
    if (prototypeObject) {
        s_prototypes[objectSchema.name] = std::move(prototypeObject);
    }
    
    return objectSchema;
}

realm::Schema RealmSchema::ParseSchema(Value &value) {
    std::vector<realm::ObjectSchema> schema;
    size_t length = ValidatedArrayLength(value);
    Array *array = Array::Cast(&value);
    for (unsigned int i = 0; i < length; i++) {
        Local<Object> jsonObjectSchema = array.CloneElementAt(i);
        ObjectSchema objectSchema = ParseObjectSchema(jsonObjectSchema);
        schema.emplace_back(std::move(objectSchema));
    }

    return realm::Schema(schema);
}
