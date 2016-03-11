/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential.
 */

#ifndef NODE_REALM_LIST_H_
#define NODE_REALM_LIST_H_

#include <node.h>
#include <node_object_wrap.h>

class RealmListWrap : public node::ObjectWrap {
public:
    RealmListWrap();
      
    static void Init(v8::Handle<v8::Object> exports);
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
    static v8::Local<v8::Value> Create(v8::Isolate*, realm::List&);
    
    static void Push(const v8::FunctionCallbackInfo<v8::Value>&);
    static void Pop(const v8::FunctionCallbackInfo<v8::Value>&);
    static void Shift(const v8::FunctionCallbackInfo<v8::Value>&);
    static void Unshift(const v8::FunctionCallbackInfo<v8::Value>&);
    static void Splice(const v8::FunctionCallbackInfo<v8::Value>&);
    static void Filtered(const v8::FunctionCallbackInfo<v8::Value>&);
    static void Sorted(const v8::FunctionCallbackInfo<v8::Value>&);
    static void StaticResults(const v8::FunctionCallbackInfo<v8::Value>&);

    static void Getter(uint32_t index, const v8::PropertyCallbackInfo<v8::Value>& info);
    static void Setter(uint32_t, v8::Local<v8::Value>, const v8::PropertyCallbackInfo<v8::Value>&);
    static void GetLength(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
    
private:
    ~RealmListWrap();

    static v8::Persistent<v8::Function> constructor;

    realm::List* m_list;
    realm::SharedRealm m_shared_realm;
};

#endif // NODE_REALM_LIST_H_
