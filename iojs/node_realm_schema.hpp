/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential.
 */

#ifndef _NODE_REALM_SCHEMA_H_
#define _NODE_REALM_SCHEMA_H_

#include <string>

#include <node.h>
#include <node_object_wrap.h>

#include "object_store.hpp"

namespace realm {
    class Schema;
    using ObjectDefaults = std::map<std::string, v8::CopyablePersistentTraits<v8::Value>::CopyablePersistent>;
}

class RealmSchemaWrap : public node::ObjectWrap {
public:
    RealmSchemaWrap();

    static void Init(v8::Handle<v8::Object> exports);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

    // Utility methods
    static realm::Schema ParseSchema(v8::Isolate* iso, v8::Local<v8::Value> value, std::map<std::string, realm::ObjectDefaults> &defaults, std::map<std::string, v8::Local<v8::Value>> &prototypes);
    static v8::Local<v8::Value> PrototypeForClassName(const std::string &className);
    static realm::ObjectDefaults &DefaultsForClassName(const std::string &className);

private:
    ~RealmSchemaWrap();

    static v8::Persistent<v8::Function> constructor;

    realm::Schema* m_schema;
    bool m_owned;
};

#endif // __NODDE_REALM_SCHEMA_H_
