#ifndef _REALMUTILS_H_
#define _REALMUTILS_H_

#include <string>
#include <exception>
#include <stdexcept>
#include <v8.h>

#include <object_schema.hpp>

void makeError(v8::Isolate *, std::string);
void makeError(v8::Isolate *, std::exception);
std::size_t ValidatedArrayLength(v8::Value *);
std::string ToString(v8::Local<v8::String>);
std::string ToString(v8::Local<v8::Value>);
v8::Local<v8::String> ToString(v8::Isolate *, const char *);

bool noArgument(v8::Isolate*, const v8::FunctionCallbackInfo<v8::Value>&);
bool oneArgument(v8::Isolate*, const v8::FunctionCallbackInfo<v8::Value>&);
bool oneFunctionArgument(v8::Isolate *, const v8::FunctionCallbackInfo<v8::Value>&);

inline bool ValidateArgumentRange(size_t argumentCount, size_t min, size_t max) {
    if (argumentCount < min || argumentCount > max) {
        throw std::invalid_argument("Invalid arguments");
    }
    return true;
}

static inline v8::Local<v8::Object> ValidatedValueToObject(v8::Local<v8::Value> value, const char *message = NULL) {
    v8::Local<v8::Object> object = value->ToObject();
    if (object->IsUndefined()) {
        throw std::runtime_error(message ?: "Value is not an object.");
    }
    return object;
}

static inline bool ValidatedValueToBool(v8::Local<v8::Value> value) {
    if (!value->IsBoolean()) {
        throw std::runtime_error("Value is not a boolean.");
    }
    return value->BooleanValue();
}

std::string ValidatedStringForValue(v8::Local<v8::Value>, const char*);
v8::Local<v8::Object> DictForPropertyArray(v8::Isolate *, realm::ObjectSchema &, v8::Local<v8::Array>);

#endif
