/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential.
 */

#include <node.h>

#include "node_realm.hpp"

#include "shared_realm.hpp"
#include "binding_context.hpp"
#include "object_store.hpp"
#include "object_accessor.hpp"
#include "schema.hpp"

#include "node_realm_object.hpp"
#include "node_realm_schema.hpp"
#include "util.hpp"

#include <set>
#include <string>

using namespace v8;

// FIXME: incomplete!
class NodeRealmDelegate : public realm::BindingContext {
public:
    virtual void changes_available() {
        assert(0);
    }

    virtual void did_change(std::vector<ObserverState> const& observers, std::vector<void*> const& invalidated) {
        notify("change");
    }

    virtual std::vector<ObserverState> get_observed_rows() {
        return std::vector<ObserverState>();
    }

    virtual void will_change(std::vector<ObserverState> const& observers,
                             std::vector<void*> const& invalidated) {};

    NodeRealmDelegate(realm::WeakRealm realm, Isolate* ctx) : m_context(ctx), m_realm(realm) {};

    ~NodeRealmDelegate() {
        remove_all_notifications();
    };

    void add_notification(Handle<Object> notification) {};
    void remove_notification(Handle<Object> notification) {};
    void remove_all_notifications() {};

private:
    Isolate* m_context;
    realm::WeakRealm m_realm;

    void notify(const char* notification_name) {};
};


static std::string s_defaultPath = "/tmp/default.realm";

Persistent<Function> RealmWrap::constructor;
RealmWrap::RealmWrap() {}
RealmWrap::~RealmWrap() {}

void RealmWrap::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, RealmWrap::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "Realm"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    NODE_SET_PROTOTYPE_METHOD(tpl, "create", RealmWrap::CreateObject);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "Realm"), tpl->GetFunction());
}

void RealmWrap::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    if (args.IsConstructCall()) {
        // Invoked as constructor: `new Realm(...)`
        try {
            RealmWrap* rw = new RealmWrap();
            realm::Realm::Config config;
            std::map<std::string, realm::ObjectDefaults> defaults;
            std::map<std::string, Local<Value>> prototypes;
            switch (args.Length()) {
            case 0:
                config.path = s_defaultPath;
                break;
            case 1:
                if (args[0]->IsStringObject()) {
                    config.path = ToString(args[0]->ToString());
                }
                else if (args[0]->IsObject()) {
                    Local<Object> configValue = args[0]->ToObject();

                    if (configValue->Has(ToString(iso, "path"))) {
                        Local<Value> path = configValue->Get(ToString(iso, "path"));
                        std::string p = ToString(path);
                        if (p == "") {
                            config.path = s_defaultPath;
                        }
                        else {
                            config.path = p;
                        }
                    }
                    else {
                        config.path = s_defaultPath;
                    }

                    if (configValue->Has(ToString(iso, "schema"))) {
                        Local<Value> schemaValue = configValue->Get(ToString(iso, "schema"));
                        if (schemaValue->IsObject()) {
                            config.schema.reset(new realm::Schema(RealmSchemaWrap::ParseSchema(iso, schemaValue->ToObject(), defaults, prototypes)));
                        }
                        else {
                            throw new std::runtime_error("Property 'schema' expected to be an object.");
                        }
                    }

                    if (configValue->Has(ToString(iso, "schemaVersion"))) {
                        Local<Value> versionValue = configValue->Get(ToString(iso, "schemaVersion"));
                        if (versionValue->IsNumber()) {
                            config.schema_version = versionValue->NumberValue();
                        }
                        else {
                            throw new std::runtime_error("Property 'schemaVersion' expected to be a number.");
                        }
                    }
                    else {
                        config.schema_version = 0;
                    }
                }
                break;
            default:
                makeError(iso, "invalid arguments.");
                args.GetReturnValue().SetUndefined();
                return;
            }
            // FIXME: ensure_directory_exists_for_file(config.path);
            realm::SharedRealm realm = realm::Realm::get_shared_realm(config);
            if (!realm->m_binding_context) {
                realm->m_binding_context.reset(new NodeRealmDelegate(realm, iso));
            }
            // FIXME: save defaults
            // FIXME: save prototypes

            rw->m_realm = realm;
            rw->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
        }
        catch (std::exception &ex) {
            makeError(iso, ex.what());
            args.GetReturnValue().SetUndefined();
            return;
        }
    } else {
        // FIXME: Invoked as plain function `Realm(...)`, turn into construct call.
    }
}

void RealmWrap::CreateObject(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        ValidataArgumentRange(args.Length(), 2, 3);
        std::string class_name = ValidatedStringForValue(iso, args[0], "objectType");
        RealmWrap* rw = ObjectWrap::Unwrap<RealmWrap>(args.This());
        realm::SharedRealm shared_realm = rw->m_realm;
        auto object_schema = shared_realm->config().schema->find(class_name);
        if (object_schema == shared_realm->config().schema->end()) {
            makeError(iso, "Object type '" + class_name + "' not found in schema.");
            args.GetReturnValue().SetUndefined();
            return;
        }
        Local<Object> obj = ValidatedValueToObject(iso, args[1]);
        if (args[1]->IsArray()) {
            obj = DictForPropertyArray(iso, *object_schema, v8::Local<v8::Array>::Cast(args[1]));
        }

        bool update = false;
        if (args.Length() == 3) {
            update = ValidatedValueToBool(args[2]);
        }

        realm::Object object = realm::Object::create<Local<Value>>(iso, shared_realm, *object_schema,
             obj, update);
        Local<Object> wrapped = RealmObjectWrap::Create(iso, new realm::Object(object));
        args.GetReturnValue().Set(wrapped);
    } catch (std::exception& ex) {
        makeError(iso, ex);
        args.GetReturnValue().SetUndefined();
        return;
    }
}
