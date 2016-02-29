/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential.
 */

#include <node.h>

#include "object_accessor.hpp"
#include "shared_realm.hpp"
#include "results.hpp"
#include "parser.hpp"
#include "query_builder.hpp"

#include "node_realm_object.hpp"
#include "node_realm_results.hpp"
#include "util.hpp"

using namespace v8;

Persistent<Function> RealmResultsWrap::constructor;
RealmResultsWrap::RealmResultsWrap() {}
RealmResultsWrap::~RealmResultsWrap() {}

void RealmResultsWrap::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, RealmResultsWrap::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "RealmResults"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    tpl->InstanceTemplate()->SetIndexedPropertyHandler(Getter, 0, 0, 0, 0);
    tpl->InstanceTemplate()->SetAccessor(ToString(isolate, "length"), RealmResultsWrap::GetLength, 0);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "RealmResults"), tpl->GetFunction());
}

void RealmResultsWrap::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    if (args.IsConstructCall()) {
        // Invoked as constructor: `new RealmResults(...)`
        try {
            ValidateArgumentCount(args.Length(), 0);
            RealmResultsWrap* rrw = new RealmResultsWrap();       
            rrw->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
            return;
        }
        catch (std::exception& ex) {
            makeError(iso, ex.what());
            args.GetReturnValue().SetUndefined();
            return;
        }
    }
    else {
        // FIXME: Invoked as plain function `RealmResults(...)`, turn into construct call.
    }
}

void RealmResultsWrap::GetLength(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);
    
    try {
        RealmResultsWrap* rrw = ObjectWrap::Unwrap<RealmResultsWrap>(info.This());
        realm::Results* results = rrw->m_results;
        
        info.GetReturnValue().Set(Number::New(iso, results->size()));
    }
    catch (std::exception& ex) {
        makeError(iso, ex.what());
        info.GetReturnValue().SetUndefined();
    }
}

void RealmResultsWrap::Getter(uint32_t index, const PropertyCallbackInfo<Value>& info) {
    Isolate* iso = info.GetIsolate();
    HandleScope scope(iso);
    
    try {
        RealmResultsWrap* rrw = ObjectWrap::Unwrap<RealmResultsWrap>(info.This());
        realm::Results* results = rrw->m_results;
        
        // invariant: index >= 0
        auto row = results->get(index);
        if (!row.is_attached()) {
            info.GetReturnValue().SetNull();
            return;
        }
        Local<v8::Object> object = RealmObjectWrap::Create(iso, new realm::Object(results->get_realm(), results->get_object_schema(), row));
        info.GetReturnValue().Set(object);
    }
    catch (std::out_of_range &exp) {
        // getters for nonexistent properties in JS should always return undefined
        info.GetReturnValue().SetUndefined();
    }
    catch (std::invalid_argument &exp) {
        // for stol failure this could be another property that is handled externally, so ignore
    }
    catch (std::exception& ex) {
        makeError(iso, ex.what());
        info.GetReturnValue().SetUndefined();
    }
}

Local<Value> RealmResultsWrap::Create(Isolate* ctx, realm::SharedRealm realm, std::string class_name) {
    EscapableHandleScope scope(ctx);
    
    realm::TableRef table = realm::ObjectStore::table_for_object_type(realm->read_group(), class_name);
    auto object_schema = realm->config().schema->find(class_name);
    if (object_schema == realm->config().schema->end()) {
        throw std::runtime_error("Object type '" + class_name + "' not represent in Realm.");
    }
    
    Local<Function> cons = Local<Function>::New(ctx, constructor);
    Handle<Object> obj = cons->NewInstance(0, nullptr);
    RealmResultsWrap* rrw = RealmResultsWrap::Unwrap<RealmResultsWrap>(obj);
    rrw->m_results = new realm::Results(realm, *object_schema, *table);
    
    return scope.Escape(obj);
}

Local<Value> RealmResultsWrap::Create(Isolate* ctx, realm::SharedRealm realm, std::string className, std::string queryString, std::vector<Local<Value>> args) {
    EscapableHandleScope scope(ctx);
    
    realm::TableRef table = realm::ObjectStore::table_for_object_type(realm->read_group(), className);
    realm::Query query = table->where();
    const realm::Schema &schema = *realm->config().schema;
    auto object_schema = schema.find(className);
    if (object_schema == schema.end()) {
        throw std::runtime_error("Object type '" + className + "' not present in Realm.");
    }
    
    realm::parser::Predicate predicate = realm::parser::parse(queryString);
    realm::query_builder::ArgumentConverter<Local<Value>, Isolate*> arguments(ctx, args);
    realm::query_builder::apply_predicate(query, predicate, arguments, schema, object_schema->name);
    
    Local<Function> cons = Local<Function>::New(ctx, constructor);
    Handle<Object> obj = cons->NewInstance(0, nullptr);
    RealmResultsWrap* rrw = RealmResultsWrap::Unwrap<RealmResultsWrap>(obj);
    
    rrw->m_results = new realm::Results(realm, *object_schema, std::move(query));
    
    return scope.Escape(obj);
}

Local<Value> RealmResultsWrap::Create(Isolate* ctx, realm::SharedRealm realm, const realm::ObjectSchema &objectSchema, const realm::Query &query, bool live) {
    EscapableHandleScope scope(ctx);

    realm::Results* results = new realm::Results(realm, objectSchema, query);
    results->set_live(live);

    Local<Function> cons = Local<Function>::New(ctx, constructor);
    Handle<Object> obj = cons->NewInstance(0, nullptr);
    RealmResultsWrap* rrw = RealmResultsWrap::Unwrap<RealmResultsWrap>(obj);
    rrw->m_results = results;
    
    return scope.Escape(obj);
}