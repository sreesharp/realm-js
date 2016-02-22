/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential.
 */

#ifndef _NODE_REALM_UTIL_H_
#define _NODE_REALM_UTIL_H_

#include <v8.h>

#include <string>
#include <exception>

static inline void makeError(v8::Isolate *ctx, std::string mesg) {
    // FIXME: TypeError isn't always the appropriate exception
    ctx->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(ctx, mesg.c_str())));
}

static inline void makeError(v8::Isolate *ctx, std::exception exception) {
    // FIXME: map C++ exceptions to V8/Node expceptions
    makeError(ctx, exception.what());
}

static inline std::string ToString(v8::Local<v8::String> v8string) {
    return *(v8::String::Utf8Value(v8string));
}

static inline std::string ToString(v8::Local<v8::Value> v8value) {
    return *(v8::String::Utf8Value(v8value->ToString()));
}

static inline v8::Local<v8::String> ToString(v8::Isolate *ctx, const char *str) {
    return v8::String::NewFromUtf8(ctx, str);
}

static inline std::size_t ValidatedArrayLength(v8::Value *value) {
    if (!value->IsArray()) {
        throw std::runtime_error("value is not an array");
    }
    v8::Array *array = v8::Array::Cast(value);
    return array->Length();
}

static inline void ValidataArgumentRange(std::size_t argc, std::size_t min, std::size_t max) {
    if (argc < min || argc > max) {
        throw std::invalid_argument("Invalid arguments");
    }
}

static inline void ValidateArgumentCountIsAtLeast(std::size_t argc, std::size_t expected) {
    if (argc < expected) {
        throw std::invalid_argument("Invalid arguments");
    }
}

static inline v8::Local<v8::Object> ValidatedValueToObject(v8::Isolate* ctx, v8::Local<v8::Value> value, const char *message = NULL) {
    v8::Local<v8::Object> object = value->ToObject();
    if (object->IsUndefined()) {
        throw std::runtime_error(message ?: "Value is not an object.");
    }
    return object;
}

static inline v8::Local<v8::Date> ValidatedValueToDate(v8::Isolate* ctx, v8::Local<v8::Value> value,
    const char *message = NULL) {
    if (!value->IsUndefined() || !value->IsDate()) {
        throw std::runtime_error(message ?: "Value is not a date.");
    }
    return value.As<v8::Date>();
}

static inline double ValidatedValueToNumber(v8::Isolate* ctx, v8::Local<v8::Value> value) {
    if (value->IsUndefined()) {
        throw std::invalid_argument("'undefined' is not a number");
    }
    if (!value->IsNumber()) {
        throw std::invalid_argument("is not a number");
    }
    return value->NumberValue();
}

static inline bool ValidatedValueToBool(v8::Local<v8::Value> value, const char* err = NULL) {
    if (!value->IsBoolean()) {
        if (err) {
            throw std::runtime_error(err);
        }
        else {
            throw std::runtime_error("Value is not a boolean.");
        }
    }
    return value->BooleanValue();
}

static inline std::string ValidatedStringForValue(v8::Isolate* ctx, v8::Local<v8::Value> value, const char* name = NULL) {
    if (!value->IsString()) {
        if (name) {
            throw std::invalid_argument((std::string)"'" + name + "' must be of type 'string'");
        }
        else {
            throw std::invalid_argument("Value must be of type 'string'");
        }
    }
    return ToString(value->ToString());
}


static inline v8::Local<v8::Value> ValidatedPropertyValue(v8::Isolate* ctx, v8::Local<v8::Value> value, v8::Local<v8::String> property) {
    if (value->IsObject()) {
        v8::Local<v8::Object> object = value->ToObject();
        if (object->Has(property)) {
            return object->Get(property);
        }
    }
    makeError(ctx, std::string("'") + ToString(property) + std::string("' does not exist"));
}

static inline v8::Local<v8::Object> ValidatedObjectProperty(v8::Isolate* ctx, v8::Local<v8::Object> object, v8::Local<v8::String> property, const char* err = NULL) {
    v8::Local<v8::Value> propertyValue = ValidatedPropertyValue(ctx, object, property);
    if (propertyValue->IsUndefined()) {
        throw std::runtime_error(err ?: "Object property '" + ToString(property) + "' is undefined");
    }
    return ValidatedValueToObject(ctx, propertyValue, err);
}

static inline std::string ValidatedStringProperty(v8::Isolate* ctx, v8::Local<v8::Value> object, v8::Local<v8::String> property) {
    if (object->IsObject()) {
        v8::Local<v8::Value> propertyValue = object->ToObject()->Get(property);
        return ToString(propertyValue);
    }
    throw std::runtime_error("Is not an object");
}

template<typename T>
T stot(const std::string s) {
    std::istringstream iss(s);
    T value;
    iss >> value;
    if (iss.fail()) {
        throw std::invalid_argument("Cannot convert string '" + s + "'");
    }
    return value;
}

static inline std::size_t ValidatedPositiveIndex(std::string indexStr) {
    long index = stot<long>(indexStr);
    if (index < 0) {
        throw std::out_of_range(std::string("Index ") + indexStr + " cannot be less than zero.");
    }
    return index;
}

static inline bool IsValueObjectOfType(v8::Isolate* ctx, v8::Local<v8::Value> value, v8::Local<v8::String> type) {
    // FIXME: implement
    //v8::Local<v8::Object> global = ctx->Global();

    return false;
}

static inline std::size_t ValidatedListLength(v8::Isolate* ctx, v8::Local<v8::Object> object) {
    v8::Local<v8::Value> length_value = object->Get(ToString(ctx, "length"));
    if (!length_value->IsNumber()) {
        throw std::runtime_error("Missing property 'length'");
    }

    return ValidatedValueToNumber(ctx, length_value);
}

static v8::Local<v8::Object> DictForPropertyArray(v8::Isolate* iso, realm::ObjectSchema &object_schema, v8::Local<v8::Array> array) {
    if (object_schema.properties.size() != array->Length()) {
        throw std::runtime_error("Array must contain values for all object properties.");
    }
    v8::Local<v8::Object> object = v8::Object::New(iso);

    for (unsigned int i = 0; i < object_schema.properties.size(); i++) {
        v8::Local<v8::String> nameStr = v8::String::NewFromUtf8(iso, object_schema.properties[i].name.c_str());
        v8::Local<v8::Value> value = array->Get(i);
        object->Set(nameStr, value);
    }
    return object;
}

#endif
