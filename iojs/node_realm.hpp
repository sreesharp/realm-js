/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential.
 */

#ifndef _NODE_REALM_H_
#define _NODE_REALM_H_

#include <node.h>
#include <node_object_wrap.h>

#include "shared_realm.hpp"

class RealmWrap : public node::ObjectWrap {
public:
    RealmWrap();

    static void Init(v8::Handle<v8::Object> exports);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void CreateObject(const v8::FunctionCallbackInfo<v8::Value>& args);

private:
    ~RealmWrap();

    static v8::Persistent<v8::Function> constructor;

    realm::SharedRealm m_realm;
};

#endif // __NODDE_REALM_H_
