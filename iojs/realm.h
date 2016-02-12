#ifndef _REALM_H
#define _REALM_H

#include <node.h>
#include <node_object_wrap.h>

#include "shared_realm.hpp"

class RealmIO : public node::ObjectWrap {
public:
    RealmIO();

    static void Init(v8::Handle<v8::Object> exports);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

    // Methods
    static void Objects(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Create(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Delete(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void DeleteAll(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Write(const v8::FunctionCallbackInfo<v8::Value>& args);

    // Getters and setters
    static void DefaultPathGetter(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
    static void DefaultPathSetter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
    static void SchemaVersionGetter(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
    static void SchemaVersionSetter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
    static void PathGetter(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
    static void PathSetter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

private:
    ~RealmIO();

    static v8::Persistent<v8::Function> constructor;

    std::string defaultPath =  "./default.realm"; // FIXME: also set in Realm.js
    std::string path;
    uint64_t schemaVersion = 0;

    realm::SharedRealm realm;

};

#endif