#include <node.h>

using namespace v8;

void InitAll(Handle<Object> exports) {
}

NODE_MODULE(Realm, InitAll)