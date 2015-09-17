#ifndef _REALMOBJECT_H
#define _REALMOBJECT_H

#include <cstddef>

#include <node.h>
#include <node_object_wrap.h>
#include <object_accessor.hpp>

using RealmAccessor = realm::NativeAccessor<v8::Value, std::nullptr_t>;

class RealmObject : public node::ObjectWrap {
public:
    RealmObject(realm::Object& target);

    static void Init(v8::Handle<v8::Object> exports);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void Get(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info);
    static void Set(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info);

private:
    ~RealmObject();

    static v8::Persistent<v8::Function> constructor;
    realm::Object& wrapped;
};

#endif
