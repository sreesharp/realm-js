/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential.
 */

#ifndef _NODE_REALM_H_
#define _NODE_REALM_H_

#include <node.h>
#include <node_object_wrap.h>

#include "shared_realm.hpp"

#include "node_realm_schema.hpp"

#include <map>

extern std::map<std::string, realm::ObjectDefaults> &NodeDefaults(realm::Realm *realm);
extern std::map<std::string, v8::Local<v8::Value>> &NodePrototypes(realm::Realm *realm);

class RealmWrap : public node::ObjectWrap {
public:
    RealmWrap();

    static void Init(v8::Handle<v8::Object> exports);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
    static v8::Handle<v8::Object> Create(v8::Isolate* , realm::SharedRealm*);
    static void CreateObject(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Write(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void DeleteAll(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void Delete(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void Objects(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void AddListener(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void RemoveListener(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void RemoveAllListeners(const v8::FunctionCallbackInfo<v8::Value>& args);
	
private:
    ~RealmWrap();

    static v8::Persistent<v8::Function> constructor;

    realm::SharedRealm m_realm;
};

#endif // __NODDE_REALM_H_
