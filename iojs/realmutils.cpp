#import "realmutils.hpp"

#import <string>
#import <exception>

#import <v8.h> 

#include <object_schema.hpp>
#include <property.hpp>

void makeError(v8::Isolate *iso, std::string mesg) {
    iso->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(iso, mesg.c_str())));
}

void makeError(v8::Isolate *iso, std::exception exception) {
    makeError(iso, exception.what());
}

std::size_t ValidatedArrayLength(v8::Value *value) {
    if (!value->IsArray()) {
        throw std::runtime_error("value is not an array");
    }
    v8::Array *array = v8::Array::Cast(value);
    return array->Length();
}

std::string ToString(v8::Local<v8::String> v8String) {
    return *(v8::String::Utf8Value(v8String));
}

std::string ToString(v8::Local<v8::Value> v8Value) {
    return *(v8::String::Utf8Value(v8Value->ToString()));
}

v8::Local<v8::String> ToString(v8::Isolate *iso, const char *string) {
    return v8::String::NewFromUtf8(iso, string);
}

bool noArgument(v8::Isolate* iso, const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() > 0) {
        makeError(iso, "No arguments expected.");
        return false;
    }
    return true;
}

bool oneArgument(v8::Isolate* iso, const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() != 0) {
        makeError(iso, "One argument expected.");
        return false;
    }
    return true;
}

bool oneFunctionArgument(v8::Isolate *iso, const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() == 1) {
        if (args[0]->IsFunction()) {
            return true;
        }
        makeError(iso, "Function expected.");
    }
    else {
        makeError(iso, "One argument expected.");
    }
    return false;
}


std::string ValidatedStringForValue(v8::Local<v8::Value> value, const char* name) {
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

v8::Local<v8::Object> DictForPropertyArray(v8::Isolate* iso, realm::ObjectSchema &object_schema, v8::Local<v8::Array> array) {
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
