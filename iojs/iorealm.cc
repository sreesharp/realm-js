#include <node.h>

#include "realm.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
    Realm::Init(exports);
//    RealmObject::Init(exports);
//    RealmResults::Init(exports);
//    RealmArray::Init(exports);
//    RealmType::Init(exports);
}

NODE_MODULE(Realm, InitAll)