#ifndef _REALM_H
#define _REALM_H

#include <node.h>
#include <node_object_wrap.h>

class Realm : public node::ObjectWrap {
public:
    Realm();

    static void Init(v8::Handle<v8::Object> exports);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

private:
    ~Realm();

    static v8::Persistent<v8::Function> constructor;
};

#endif