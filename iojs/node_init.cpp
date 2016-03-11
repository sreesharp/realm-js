/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential.
 */

#include <node.h>

#include "node_realm.hpp"
#include "node_realm_object.hpp"
#include "node_realm_results.hpp"
#include "node_realm_list.hpp"

using namespace v8;

void InitAll(Handle<Object> exports) {
    RealmWrap::Init(exports);
    RealmObjectWrap::Init(exports);
    RealmResultsWrap::Init(exports);
    RealmListWrap::Init(exports);
}

NODE_MODULE(Realm, InitAll)
