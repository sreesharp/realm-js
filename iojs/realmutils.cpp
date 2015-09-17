#import "realmutils.hpp"

#import <string>
#import <exception>

#import <v8.h> 

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
