#ifndef _REALMUTILS_H_
#define _REALMUTILS_H_

#import <string>
#import <exception>
#import <v8.h>


void makeError(v8::Isolate *, std::string);
void makeError(v8::Isolate *, std::exception);
std::size_t ValidatedArrayLength(v8::Value *);
std::string ToString(v8::Local<v8::String>);
std::string ToString(v8::Local<v8::Value>);
v8::Local<v8::String> ToString(v8::Isolate *, const char *);
bool oneFunctionArgument(v8::Isolate *, const v8::FunctionCallbackInfo<v8::Value>&);

#endif