#include <node.h>

#include <string>

#include "realm.h"
#include "realmschema.hpp"

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

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "Realm"), tpl->GetFunction());
}

std::string writeablePathForFile(const std::string &fileName) {
    // FIXME: find a better place than current directory
    return "./" + fileName;
}


void RealmIO::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

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
                    break;
                }
                if (args[0]->IsObject()) {
                    config.schema = std::make_unique<realm::Schema>(RealmSchema::ParseSchema(args[0]));
                    break;
                }
                if (args[0]->IsNumber()) {
                    config.schema_version = args[0]->IntegerValue();
                    break;
                }
            default:
                return;
            }
            RealmIO* r = new RealmIO();
            realm::SharedRealm realm = realm::Realm::get_shared_realm(config);
            if (!realm->m_delegate) {
                realm->m_delegate = std::make_unique<RJSRealmDelegate>();
            }
            r->realm = realm;
            r->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
        }
        catch (std::exception &ex) {
            isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, ex.what())));
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
