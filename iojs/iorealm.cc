#include <node.h>

#include "realm.h"
#include "realmobject.h"
#include "realmresults.h"
#include "realmarray.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
    RealmIO::Init(exports);
    RealmObject::Init(exports);
    RealmResults::Init(exports);
    RealmArray::Init(exports);
//    RealmType::Init(exports);
}

NODE_MODULE(Realm, InitAll)