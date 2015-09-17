#ifndef _REALMOBJECT_H
#define _REALMOBJECT_H

#include <node.h>
#include <node_object_wrap.h>

class RealmObject : public node::ObjectWrap {
public:
    RealmObject();

    static void Init(v8::Handle<v8::Object> exports);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

private:
    ~RealmObject();

    static v8::Persistent<v8::Function> constructor;
};

#endif