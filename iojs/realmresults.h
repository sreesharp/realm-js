#ifndef _REALMRESULTS_H
#define _REALMRESULTS_H

#include <node.h>
#include <node_object_wrap.h>

class RealmResults : public node::ObjectWrap {
public:
    RealmResults();

    static void Init(v8::Handle<v8::Object> exports);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

private:
    ~RealmResults();

    static v8::Persistent<v8::Function> constructor;
};

#endif
