#ifndef _REALMOBJECT_H
#define _REALMOBJECT_H

#include <node.h>
#include <node_object_wrap.h>

class RealmObject : public node::ObjectWrap {
public:
    RealmObject();

    static void Init(v8::Handle<v8::Object> exports);
    static v8::Local<RealmObject> Create(realm::Object *target);

private:
    ~RealmObject();

    static v8::Persistent<v8::Function> constructor;
    realm::Object *m_object;
};

#endif