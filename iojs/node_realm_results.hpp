/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential.
 */

#ifndef _NODE_REALM_RESULTS_H_
#define _NODE_REALM_RESULTS_H_

#include <node.h>
#include <node_object_wrap.h>

#include "shared_realm.hpp"
#include "results.hpp"

class RealmResultsWrap : public node::ObjectWrap {
public:
    RealmResultsWrap();
  
    static void Init(v8::Handle<v8::Object> exports);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

    static v8::Local<v8::Value> Create(v8::Isolate*, realm::SharedRealm, std::string);
    static v8::Local<v8::Value> Create(v8::Isolate*, realm::SharedRealm, std::string, std::string, std::vector<v8::Local<v8::Value>>);
    static v8::Local<v8::Value> Create(v8::Isolate*, realm::SharedRealm, const realm::ObjectSchema&, const realm::Query&, bool);
    static v8::Local<v8::Value> CreateFiltered(v8::Isolate*, realm::SharedRealm, const realm::ObjectSchema&, realm::Query, size_t, const v8::FunctionCallbackInfo<v8::Value>&);
    
    static void Filtered(const v8::FunctionCallbackInfo<v8::Value>&);

    // getter - results are read-only
    static void Getter(uint32_t index, const v8::PropertyCallbackInfo<v8::Value>& info);
    static void GetLength(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
      
private:
    ~RealmResultsWrap();

    static v8::Persistent<v8::Function> constructor;
    
    realm::Results* m_results;
    realm::SharedRealm m_shared_realm;
};

#endif // _NODE_REALM_RESULTS_H_