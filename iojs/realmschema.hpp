
#ifndef _REALMSCHEMA_H
#define _REALMSCHEMA_H

#include <node.h>
#include <node_object_wrap.h>

#include <memory>
#include "shared_realm.hpp"

namespace realm {
    class Schema;
    using ObjectDefaults = std::map<std::string, v8::Handle<v8::Value>>;
}


class RealmSchema : public node::ObjectWrap {
public:
    RealmSchema();

    static void Init(v8::Handle<v8::Object> exports);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

    // Methods

    // Util
    static realm::Schema ParseSchema(v8::Value *value);
    static v8::Local<v8::Value> PrototypeForClassName(const std::string &className);
    static realm::ObjectDefaults &DefaultsForClassName(const std::string &className);
    
private:
    ~RealmSchema();
    
    static v8::Persistent<v8::Function> constructor;
};

#endif /* schema_hpp */
