/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential.
 */

#ifndef _NODE_REALM_OBJECT_H_
#define _NODE_REALM_OBJECT_H_

#include <node.h>
#include <node_object_wrap.h>

#include "object_accessor.hpp"

class RealmObjectWrap : public node::ObjectWrap {
public:
    RealmObjectWrap();

    static void Init(v8::Handle<v8::Object> exports);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
    static v8::Handle<v8::Object> Create(v8::Isolate* iso, realm::Object* object);

    static void Get(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
    static void Set(v8::Local<v8::String>, v8::Local<v8::Value>, const v8::PropertyCallbackInfo<v8::Value>&);

    static realm::Object* GetObject(v8::Local<v8::Object> self);
private:
    ~RealmObjectWrap();

    static v8::Persistent<v8::Function> constructor;

    realm::Object* m_object;
};

#endif // __NODDE_REALM_OBJECT_H_
