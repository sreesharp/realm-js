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

private:
    ~RealmIO();

    static v8::Persistent<v8::Function> constructor;

    realm::SharedRealm realm;

};

#endif