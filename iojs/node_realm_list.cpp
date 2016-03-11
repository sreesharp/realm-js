/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential.
 */

#include <node.h>

#include "object_accessor.hpp"
#include "parser.hpp"
#include "query_builder.hpp"

#include "node_realm_list.hpp"
#include "node_realm_results.hpp"
#include "node_realm_object.hpp"
#include "util.hpp"

using namespace v8;

Persistent<Function> RealmListWrap::constructor;
RealmListWrap::RealmListWrap() {}
RealmListWrap::~RealmListWrap() {}

using NullType = std::nullptr_t;
using ValueType = Local<Value>;
using IsolateType = Isolate*;
using Accessor = realm::NativeAccessor<ValueType, IsolateType>;

void RealmListWrap::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, RealmListWrap::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "RealmList"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    NODE_SET_PROTOTYPE_METHOD(tpl, "push",     RealmListWrap::Push);
    NODE_SET_PROTOTYPE_METHOD(tpl, "pop",      RealmListWrap::Pop);
    NODE_SET_PROTOTYPE_METHOD(tpl, "shift",    RealmListWrap::Shift);
    NODE_SET_PROTOTYPE_METHOD(tpl, "unshift",  RealmListWrap::Unshift);
    NODE_SET_PROTOTYPE_METHOD(tpl, "splice",   RealmListWrap::Splice);
    NODE_SET_PROTOTYPE_METHOD(tpl, "filtered", RealmListWrap::Filtered);
    NODE_SET_PROTOTYPE_METHOD(tpl, "sorted",   RealmListWrap::Sorted);
    NODE_SET_PROTOTYPE_METHOD(tpl, "snapshot", RealmListWrap::StaticResults);

    tpl->InstanceTemplate()->SetIndexedPropertyHandler(Getter, Setter, 0, 0, 0);
    tpl->InstanceTemplate()->SetAccessor(ToString(isolate, "length"), RealmListWrap::GetLength, 0);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "RealmList"), tpl->GetFunction());
}

void RealmListWrap::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    if (args.IsConstructCall()) {
        // Invoked as constructor: `new RealmList(...)`
        try {
            ValidateArgumentCount(args.Length(), 0);
            RealmListWrap* rrw = new RealmListWrap();       
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
        // FIXME: Invoked as plain function `RealmList(...)`, turn into construct call.
    }
}

Local<Value> RealmListWrap::Create(Isolate* ctx, realm::List& list) {
    EscapableHandleScope scope(ctx);

    Local<Function> cons = Local<Function>::New(ctx, constructor);
    Handle<Object> obj = cons->NewInstance(0, nullptr);
    RealmListWrap* rlw = RealmListWrap::Unwrap<RealmListWrap>(obj);
    rlw->m_list = new realm::List(list);
    return scope.Escape(obj);
}

void RealmListWrap::GetLength(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);
    
    try {
        RealmListWrap* rlw = ObjectWrap::Unwrap<RealmListWrap>(info.This());
        realm::List* list = rlw->m_list;
        
        info.GetReturnValue().Set(Number::New(iso, list->size()));
    }
    catch (std::exception& ex) {
        makeError(iso, ex.what());
        info.GetReturnValue().SetUndefined();
    }
}

void RealmListWrap::Getter(uint32_t index, const PropertyCallbackInfo<Value>& info) {
    Isolate* iso = info.GetIsolate();
    HandleScope scope(iso);
    
    try {
        RealmListWrap* rlw = ObjectWrap::Unwrap<RealmListWrap>(info.This());
        realm::List* list = rlw->m_list;
        info.GetReturnValue().Set(RealmObjectWrap::Create(iso, new realm::Object(list->realm(), list->get_object_schema(), list->get(index))));
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

void RealmListWrap::Setter(uint32_t index, Local<Value> value, const PropertyCallbackInfo<Value>& info) {
    Isolate* iso = info.GetIsolate();
    HandleScope scope(iso);
    
    try {
        RealmListWrap* rlw = ObjectWrap::Unwrap<RealmListWrap>(info.This());
        realm::List* list = rlw->m_list;
        list->set(iso, value, index);
        info.GetReturnValue().Set(Boolean::New(iso, true));
    }
    catch (std::invalid_argument &exp) {
        // for stol failure this could be another property that is handled externally, so ignore
        info.GetReturnValue().Set(Boolean::New(iso, false));
    }
    catch (std::exception& ex) {
        makeError(iso, ex.what());
        info.GetReturnValue().SetUndefined();
    }
}
    
void RealmListWrap::Push(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        RealmListWrap* rlw = ObjectWrap::Unwrap<RealmListWrap>(args.This());
        realm::List* list = rlw->m_list;
        ValidateArgumentCountIsAtLeast(args.Length(), 1);
        for (int i = 0; i < args.Length(); i++) {
            list->add(iso, args[i]);
        }
        args.GetReturnValue().Set(Number::New(iso, list->size()));
    }
    catch  (std::exception& ex) {
        makeError(iso, ex.what());
        args.GetReturnValue().SetUndefined();
    }
}

void RealmListWrap::Pop(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        RealmListWrap* rlw = ObjectWrap::Unwrap<RealmListWrap>(args.This());
        realm::List* list = rlw->m_list;
        ValidateArgumentCount(args.Length(), 0);

        std::size_t size = list->size();
        if (size == 0) {
            list->verify_in_tranaction();
            args.GetReturnValue().SetUndefined();
            return;
        }
        std::size_t index = size - 1;
        Local<v8::Object> object = RealmObjectWrap::Create(iso, new realm::Object(list->realm(), list->get_object_schema(), list->get(index)));
        args.GetReturnValue().Set(object);
        list->remove(index);
    }
    catch  (std::exception& ex) {
        makeError(iso, ex.what());
        args.GetReturnValue().SetUndefined();
    }
}

void RealmListWrap::Unshift(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        RealmListWrap* rlw = ObjectWrap::Unwrap<RealmListWrap>(args.This());
        realm::List* list = rlw->m_list;
        ValidateArgumentCountIsAtLeast(args.Length(), 1);
        for (int i = 0; i < args.Length(); i++) {
            list->insert(iso, args[i], i);
        }
        args.GetReturnValue().Set(Number::New(iso, list->size()));
    }
    catch  (std::exception& ex) {
        makeError(iso, ex.what());
        args.GetReturnValue().SetUndefined();
    }
}

void RealmListWrap::Shift(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        RealmListWrap* rlw = ObjectWrap::Unwrap<RealmListWrap>(args.This());
        realm::List* list = rlw->m_list;
        ValidateArgumentCount(args.Length(), 0);
        if (list->size() == 0) {
            list->verify_in_tranaction();
            args.GetReturnValue().SetUndefined();
            return;
        }
        Local<v8::Object> object = RealmObjectWrap::Create(iso, new realm::Object(list->realm(), list->get_object_schema(), list->get(0)));
        args.GetReturnValue().Set(object);
        list->remove(0);
    }
    catch  (std::exception& ex) {
        makeError(iso, ex.what());
        args.GetReturnValue().SetUndefined();
    }
}
        
void RealmListWrap::Splice(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        RealmListWrap* rlw = ObjectWrap::Unwrap<RealmListWrap>(args.This());
        realm::List* list = rlw->m_list;
        std::size_t size = list->size();

        ValidateArgumentCountIsAtLeast(args.Length(), 2);
        long index = std::min<long>(ValidatedValueToNumber(iso, args[0]), size);
        if (index < 0) {
            index = std::max<long>(size + index, 0);
        }

        long remove = std::max<long>(ValidatedValueToNumber(iso, args[0]), 0);
        remove = std::min<long>(remove, size - index);

        std::vector<Local<v8::Object>> removedObjects(remove);
        for (int i = 0; i < remove; i++) {
            removedObjects[i] = RealmObjectWrap::Create(iso, new realm::Object(list->realm(), list->get_object_schema(), list->get(index)));
            list->remove(index);
        }
        for (int i = 2; i < args.Length(); i++) {
            list->insert(iso, args[i], index + i - 2);
        }
        Handle<v8::Array> array = v8::Array::New(iso, remove);
        for (int i = 0; i < remove; i++) {
            array->Set(i, removedObjects[i]);
        }
        args.GetReturnValue().Set(array);
    }
    catch  (std::exception& ex) {
        makeError(iso, ex.what());
        args.GetReturnValue().SetUndefined();
    }
}

void RealmListWrap::StaticResults(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        RealmListWrap* rlw = ObjectWrap::Unwrap<RealmListWrap>(args.This());
        realm::List* list = rlw->m_list;

        ValidateArgumentCount(args.Length(), 0);

        args.GetReturnValue().Set(RealmResultsWrap::Create(iso, list->realm(), list->get_object_schema(), std::move(list->get_query()), false));
    }
    catch  (std::exception& ex) {
        makeError(iso, ex.what());
        args.GetReturnValue().SetUndefined();
    }
}

void RealmListWrap::Filtered(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        RealmListWrap* rlw = ObjectWrap::Unwrap<RealmListWrap>(args.This());
        realm::List* list = rlw->m_list;
        ValidateArgumentCountIsAtLeast(args.Length(), 1);
        args.GetReturnValue().Set(RealmResultsWrap::CreateFiltered(iso, list->realm(), list->get_object_schema(), std::move(list->get_query()), args.Length(), args));
    }
    catch  (std::exception& ex) {
        makeError(iso, ex.what());
        args.GetReturnValue().SetUndefined();
    }
}

void RealmListWrap::Sorted(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        RealmListWrap* rlw = ObjectWrap::Unwrap<RealmListWrap>(args.This());
        realm::List* list = rlw->m_list;
        ValidateArgumentRange(args.Length(), 1, 2);
        args.GetReturnValue().Set(RealmResultsWrap::CreateFiltered(iso, list->realm(), list->get_object_schema(), std::move(list->get_query()), args.Length(), args));
    }
    catch  (std::exception& ex) {
        makeError(iso, ex.what());
        args.GetReturnValue().SetUndefined();
    }
}
        
