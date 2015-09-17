#include <node.h>

#include "realmresults.h"
#include "realmobject.h"
#include "realmutils.hpp"

#include "object_accessor.hpp"

using namespace v8;

Persistent<Function> RealmResults::constructor;

RealmResults::RealmResults(realm::Results results) : m_results(std::move(results)) {
}

RealmResults::~RealmResults() {}

void RealmResults::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, RealmResults::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "RealmResults"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Methods
    NODE_SET_PROTOTYPE_METHOD(tpl, "sortByProperty",     RealmResults::SortByProperty);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "RealmResults"), tpl->GetFunction());
}


void RealmResults::Get(uint32_t index, const v8::PropertyCallbackInfo<v8::Value>& info) {
    Isolate* isolate = Isolate::GetCurrent();
    RealmResults *results = ObjectWrap::Unwrap<RealmResults>(info.This());

    EscapableHandleScope handle_scope(isolate);
    realm::Object *obj =  new realm::Object(results->m_results.realm, results->m_results.object_schema, results->m_results.get(index));
    Persistent<RealmObject> object(obj);
    info.GetReturnValue().Set(object);
}


void RealmResults::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (args.IsConstructCall()) {
        // Invoked as constructor: `new RealmResults(...)`
        args.GetReturnValue().Set(args.This());
    } else {
        // TODO: Invoked as plain function `RealmResults(...)`, turn into construct call.
    }

    Local<ObjectTemplate> result = ObjectTemplate::New(isolate);
    result->SetIndexedPropertyHandler(RealmResults::Get);

}

bool ValidateArgumentRange(size_t count, size_t min, size_t max) {
    if (count < min || count > max)
    {
        makeError(Isolate::GetCurrent(), "argument count must be between " + std::to_string(min) + " and " + std::to_string(max));
        return false;
    }
    return true;
}

void RealmResults::SortByProperty(const v8::FunctionCallbackInfo<v8::Value>& args) {
    RealmResults *results = ObjectWrap::Unwrap<RealmResults>(args.This());

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (!ValidateArgumentRange(args.Length(), 1, 2)) {
        return;
    }

    std::string propName = ToString(args[0]);
    Property *prop = results->m_results.object_schema.property_for_name(propName);
    if (!prop) {
        makeError(isolate, "Property '" + propName + "' does not exist on object type '" + 
            results->m_results.object_schema.name + "'");
        return;
    }

    bool ascending = true;
    if (args.Length() == 2) {
        ascending = *args[1]->ToBoolean();
    }

    SortOrder sort = {{prop->table_column}, {ascending}};
    results->m_results.setSort(sort);
}
