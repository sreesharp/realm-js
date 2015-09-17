#include <node.h>

#include <string>

#include "realm.h"
#include "realmschema.hpp"
#include "realmutils.hpp"

#import "shared_realm.hpp"
#import "object_accessor.hpp"
#import "realm_delegate.hpp"

#import <set>

using namespace v8;


// FIXME: should be removed
class RJSRealmDelegate : public realm::RealmDelegate {
public:
    typedef std::shared_ptr<std::function<void(const std::string)>> NotificationFunction;
    void add_notification(NotificationFunction &notification) { m_notifications.insert(notification); }
    void remove_notification(NotificationFunction notification) { m_notifications.erase(notification); }
    void remove_all_notifications() { m_notifications.clear(); }
    std::set<NotificationFunction> m_notifications;

    virtual void changes_available() {
        for (NotificationFunction notification : m_notifications) {
            (*notification)("RefreshRequiredNotification");
        }
    }

    virtual void did_change(std::vector<ObserverState> const& observers,
                            std::vector<void*> const& invalidated) {
        for (NotificationFunction notification : m_notifications) {
            (*notification)("DidChangeNotification");
        }
    }

    virtual std::vector<ObserverState> get_observed_rows() {
        return std::vector<ObserverState>();
    }

    virtual void will_change(std::vector<ObserverState> const& observers,
                             std::vector<void*> const& invalidated) {

    }
};

Persistent<Function> RealmIO::constructor;

RealmIO::RealmIO() {}

RealmIO::~RealmIO() {}

void RealmIO::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, RealmIO::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "Realm"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    NODE_SET_PROTOTYPE_METHOD(tpl, "objects",     RealmIO::Objects);
    NODE_SET_PROTOTYPE_METHOD(tpl, "create",      RealmIO::Create);
    NODE_SET_PROTOTYPE_METHOD(tpl, "delete",      RealmIO::Delete);
    NODE_SET_PROTOTYPE_METHOD(tpl, "deleteAll",   RealmIO::DeleteAll);
    NODE_SET_PROTOTYPE_METHOD(tpl, "write",       RealmIO::Write);
    // FIXME: addNotification

    // Realm.defaultPath
    v8::Local<v8::String> constant_name = 
        v8::String::NewFromUtf8(isolate, "defaultPath");
    v8::Local<v8::String> constant_value = 
        v8::String::NewFromUtf8(isolate, "./default.realm");   
    v8::PropertyAttribute constant_attributes =
        static_cast<v8::PropertyAttribute>(v8::DontDelete); 
    tpl->Set(constant_name, constant_value, constant_attributes);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "Realm"), tpl->GetFunction());
}

std::string writeablePathForFile(const std::string &fileName) {
    // FIXME: find a better place than current directory
    return "./" + fileName;
}


void RealmIO::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    if (args.IsConstructCall()) {
        // Invoked as constructor: `new Realm(...)`
        try {
            realm::Realm::Config config;
            switch (args.Length()) {
            case 0:
                config.path = writeablePathForFile("default.realm");
                break;
            case 1:
                if (args[0]->IsStringObject()) {
                    v8::String::Utf8Value s(args[0]->ToString());
                    config.path = std::string(*s);
                }
                else if (args[0]->IsObject()) {
                    Local<Object> configValue = args[0]->ToObject();
                    Local<Value> version = configValue->Get(String::NewFromUtf8(iso, "schemaVersion"));
                    if (!version->IsUndefined()) {
                        config.schema_version = configValue->Get(String::NewFromUtf8(iso, "prototype"))->IntegerValue();
                    }

                    Local<Value> schema = configValue->Get(String::NewFromUtf8(iso, "schema"));
                    if (!schema->IsUndefined()) {
                        config.schema = std::make_unique<realm::Schema>(RealmSchema::ParseSchema(*schema));
                    }

                    Local<Value> path = configValue->Get(String::NewFromUtf8(iso, "path"));
                    if (!path->IsUndefined()) {
                        config.path = *String::Utf8Value(path->ToString());
                    }
                    else {
                        config.path = writeablePathForFile("default.realm");
                    }

                    break;
                }
            default:
                makeError(iso, "invalid arguments.");
            }
            RealmIO* r = new RealmIO();
            realm::SharedRealm realm = realm::Realm::get_shared_realm(config);
            if (!realm->m_delegate) {
                realm->m_delegate = std::make_unique<RJSRealmDelegate>();
            }

            v8::Local<v8::String> prop_name = v8::String::NewFromUtf8(iso, "path");
            v8::Local<v8::String> prop_value = v8::String::NewFromUtf8(iso, config.path.c_str());   
            (*args.This())->Set(prop_name, prop_value);

            r->realm = realm;
            r->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
        }
        catch (std::exception &ex) {
            makeError(iso, ex);
            args.GetReturnValue().SetUndefined();
        }
    } else {
        // FIXME: Invoked as plain function `Realm(...)`, turn into construct call.
    }
}

void RealmIO::Objects(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
}

void RealmIO::Create(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    
}

void RealmIO::Delete(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

}

void RealmIO::DeleteAll(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

}

void RealmIO::Write(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

}
