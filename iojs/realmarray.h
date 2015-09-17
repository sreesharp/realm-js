#ifndef _REALMARRAY_H
#define _REALMARRAY_H

#include <node.h>
#include <node_object_wrap.h>

class RealmArray : public node::ObjectWrap {
public:
    RealmArray();

    static void Init(v8::Handle<v8::Object> exports);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

    // Methods
    static void Push(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Pop(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Shift(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Unshift(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Splice(const v8::FunctionCallbackInfo<v8::Value>& args);


private:
    ~RealmArray();

    static v8::Persistent<v8::Function> constructor;
};

#endif
