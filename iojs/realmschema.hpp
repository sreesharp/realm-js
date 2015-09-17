
#ifndef _REALMSCHEMA_H
#define _REALMSCHEMA_H

#include <node.h>
#include <node_object_wrap.h>

#include <memory>
#include "shared_realm.hpp"

namespace realm {
    class Schema;
}

class RealmSchema : public node::ObjectWrap {
public:
    RealmSchema();

    static void Init(v8::Handle<v8::Object> exports);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

    // Methods

    // Util
    static realm::Schema ParseSchema(v8::Value &value);

private:
    ~RealmSchema();
    
    static v8::Persistent<v8::Function> constructor;
};

#endif /* schema_hpp */
