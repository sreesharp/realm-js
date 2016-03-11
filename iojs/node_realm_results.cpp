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

    NODE_SET_PROTOTYPE_METHOD(tpl, "filtered", RealmResultsWrap::Filtered);
    NODE_SET_PROTOTYPE_METHOD(tpl, "sorted",   RealmResultsWrap::Sorted);

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
    rrw->m_shared_realm = realm;
    
    return scope.Escape(obj);
}

Local<Value> RealmResultsWrap::Create(Isolate* ctx, realm::SharedRealm realm, realm::Results* results) {
    EscapableHandleScope scope(ctx);
    
    Local<Function> cons = Local<Function>::New(ctx, constructor);
    Handle<Object> obj = cons->NewInstance(0, nullptr);
    RealmResultsWrap* rrw = RealmResultsWrap::Unwrap<RealmResultsWrap>(obj);
    rrw->m_results = results;
    rrw->m_shared_realm = realm;
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

Local<Value> RealmResultsWrap::Create(Isolate* ctx, realm::SharedRealm realm, const realm::ObjectSchema &objectSchema, const realm::Query &query, bool live = true) {
    EscapableHandleScope scope(ctx);

    realm::Results* results = new realm::Results(realm, objectSchema, query);
    results->set_live(live);

    Local<Function> cons = Local<Function>::New(ctx, constructor);
    Handle<Object> obj = cons->NewInstance(0, nullptr);
    RealmResultsWrap* rrw = RealmResultsWrap::Unwrap<RealmResultsWrap>(obj);
    rrw->m_results = results;
    
    return scope.Escape(obj);
}

Local<Value> RealmResultsWrap::CreateFiltered(Isolate* ctx, realm::SharedRealm realm, const realm::ObjectSchema& objectSchema, realm::Query query, size_t argumentCount, const FunctionCallbackInfo<Value>& arguments) {
    std::string queryString = ValidatedStringForValue(ctx, arguments[0]);
    std::vector<Local<Value>> args(argumentCount - 1);
    for (size_t i = 1; i < argumentCount; i++) {
        args[i-1] = arguments[i];
    }
    
    realm::parser::Predicate predicate = realm::parser::parse(queryString);
    realm::query_builder::ArgumentConverter<Local<Value>, Isolate*> queryArgs(ctx, args);
    realm::query_builder::apply_predicate(query, predicate, queryArgs, *realm->config().schema, objectSchema.name);
    
    return RealmResultsWrap::Create(ctx, realm, objectSchema, std::move(query));
}

Local<Value> RealmResultsWrap::CreateSorted(Isolate* ctx,  realm::SharedRealm realm, const realm::ObjectSchema& objectSchema, realm::Query query, size_t argumentCount, const FunctionCallbackInfo<Value>& arguments) {
    size_t prop_count;
    std::vector<std::string> prop_names;
    std::vector<bool> ascending;
    
    if (arguments[0]->IsArray()) {
        ValidateArgumentCount(argumentCount, 1, "Second argument is not allowed if passed an array of sort descriptors");
        
        Local<Object> js_prop_names = ValidatedValueToObject(ctx, arguments[0]);
        prop_count = js_prop_names->GetOwnPropertyNames()->Length();
        if (!prop_count) {
            throw std::invalid_argument("Sort descriptor array must not be empty");
        }

        prop_names.resize(prop_count);
        ascending.resize(prop_count);

        for (unsigned int i = 0; i < prop_count; i++) {
            Local<Value> val = js_prop_names->Get(i);
            
            if (val->IsArray()) {
                v8::Array *arr = v8::Array::Cast(*val);
                prop_names[i] = ValidatedStringForValue(ctx, arr->Get(0));
                ascending[i] = !((arr->Get(1)->ToBoolean())->BooleanValue())    ;
            }
            else {
                prop_names[i] = ValidatedStringForValue(ctx, val);
                ascending[i] = true;
            }
        }
    }
    else {
        ValidateArgumentRange(argumentCount, 1, 2);
        prop_count = 1;
        prop_names.push_back(ValidatedStringForValue(ctx, arguments[0]));
        ascending.push_back(argumentCount == 1 ? true : !(arguments[1]->BooleanValue()));
    }
    
    std::vector<size_t> columns(prop_count);
    size_t index = 0;

    for (std::string prop_name : prop_names) {
        const realm::Property *prop = objectSchema.property_for_name(prop_name);
        if (!prop) {
            throw std::runtime_error("Property '" + prop_name + "' does not exist on object type '" + objectSchema.name + "'");
        }
        columns[index++] = prop->table_column;
    }

    realm::Results *results = new realm::Results(realm, objectSchema, std::move(query), {std::move(columns), std::move(ascending)});
    return RealmResultsWrap::Create(ctx, realm, results);

}

void RealmResultsWrap::Filtered(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    
    try {
        RealmResultsWrap* rrw = RealmResultsWrap::Unwrap<RealmResultsWrap>(args.This());
        realm::Results* results = rrw->m_results;
        
        realm::SharedRealm sharedRealm = rrw->m_shared_realm;
        Local<Value> res = RealmResultsWrap::CreateFiltered(iso, sharedRealm, results->get_object_schema(), std::move(results->get_query()), args.Length(), args);
        args.GetReturnValue().Set(res);
    }
    catch (std::exception& ex) {
        makeError(iso, ex.what());
        args.GetReturnValue().SetUndefined();
    }
}

void RealmResultsWrap::Sorted(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    
    try {
        RealmResultsWrap* rrw = RealmResultsWrap::Unwrap<RealmResultsWrap>(args.This());
        realm::Results* results = rrw->m_results;

        ValidateArgumentRange(args.Length(), 1, 2);
        
        realm::SharedRealm sharedRealm = rrw->m_shared_realm;
        Local<Value> res = RealmResultsWrap::CreateSorted(iso, sharedRealm, results->get_object_schema(), std::move(results->get_query()), args.Length(), args);
        args.GetReturnValue().Set(res);
    }
    catch (std::exception& ex) {
        makeError(iso, ex.what());
        args.GetReturnValue().SetUndefined();
    }
}
