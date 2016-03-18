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
#include "node_realm_results.hpp"
#include "util.hpp"

#include <set>
#include <string>
#include <map>

using namespace v8;

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

    void add_notification(Handle<Function> notification) {
        if (has_notification(notification) == -1) {
            // FIXME: protect?
            CopyablePersistentTraits<Value>::CopyablePersistent p_object;
            p_object.Reset(m_context, notification);
            m_notifications.push_back(p_object);
        }
    };

    void remove_notification(Handle<Function> notification) {
        auto pos = has_notification(notification);
        if (pos >= 0) {
            // FIXME: unprotect
            //m_notifications.erase(std::remove(m_notifications.begin(), m_notifications.end(), notification), m_notifications.end());
        }
    };
    
    void remove_all_notifications() {
        //for (auto notification : m_notifications) {
            // FIXME: unprotect
        //}
        m_notifications.clear();
    };
    
    std::map<std::string, realm::ObjectDefaults> m_defaults;
    std::map<std::string, Local<Value>> m_prototypes;

private:
    std::vector<CopyablePersistentTraits<Value>::CopyablePersistent> m_notifications;
    Isolate* m_context;
    realm::WeakRealm m_realm;

    void notify(const char* notification_name) {
        realm::SharedRealm realm = m_realm.lock();
        if (!realm) {
            throw std::runtime_error("Realm no longer exists");
        }
        
        Handle<Value> arguments[] = { RealmWrap::Create(m_context, new realm::SharedRealm(m_realm)), ToString(m_context, notification_name) };
        for (size_t i = 0; i < m_notifications.size(); i++) {
            TryCatch trycatch;
            Local<Value> tmp = Local<Value>::New(m_context, m_notifications[i]);
            Local<Function> fun = Local<Function>::Cast(tmp);
            fun->Call(v8::Context::New(m_context)->Global(), 2, arguments);
            if (trycatch.HasCaught()) {
                throw std::runtime_error(ToString(trycatch.Message()->Get()));
            }
        }
    };

    int has_notification(Handle<Function> new_notification) {
        int pos = 0;
        while (pos  < m_notifications.size()) {
            Local<Function> tmp = Local<Function>::Cast(Local<Value>::New(m_context, m_notifications[pos]));
            if (tmp->Equals(new_notification)) {
                return pos;
            }
            pos++;
        }
        return -1;
    }
};

std::map<std::string, realm::ObjectDefaults> &NodeDefaults(realm::Realm *realm) {
    return static_cast<NodeRealmDelegate *>(realm->m_binding_context.get())->m_defaults;
}

std::map<std::string, Local<Value>> &NodePrototypes(realm::Realm *realm) {
    return static_cast<NodeRealmDelegate *>(realm->m_binding_context.get())->m_prototypes;
}

static std::string s_defaultPath = "default.realm";

Persistent<Function> RealmWrap::constructor;
RealmWrap::RealmWrap() {}
RealmWrap::~RealmWrap() {}

void RealmWrap::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, RealmWrap::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "Realm"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    NODE_SET_PROTOTYPE_METHOD(tpl, "create",    RealmWrap::CreateObject);
	NODE_SET_PROTOTYPE_METHOD(tpl, "write",     RealmWrap::Write);
	NODE_SET_PROTOTYPE_METHOD(tpl, "deleteAll", RealmWrap::DeleteAll);
    NODE_SET_PROTOTYPE_METHOD(tpl, "delete",    RealmWrap::Delete);
    NODE_SET_PROTOTYPE_METHOD(tpl, "close",     RealmWrap::Close);
    NODE_SET_PROTOTYPE_METHOD(tpl, "objects",   RealmWrap::Objects);
    NODE_SET_PROTOTYPE_METHOD(tpl, "addListener", RealmWrap::AddListener);
    NODE_SET_PROTOTYPE_METHOD(tpl, "removeListener", RealmWrap::RemoveListener);
    NODE_SET_PROTOTYPE_METHOD(tpl, "removeAllListeners", RealmWrap::RemoveAllListeners);

    tpl->InstanceTemplate()->SetAccessor(ToString(isolate, "defaultPath"), RealmWrap::GetDefaultPath, RealmWrap::SetDefaultPath);
    tpl->InstanceTemplate()->SetAccessor(ToString(isolate, "path"), RealmWrap::GetPath, 0);
    
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
                            throw std::runtime_error("Property 'schema' expected to be an object.");
                        }
                    }

                    if (configValue->Has(ToString(iso, "schemaVersion"))) {
                        Local<Value> versionValue = configValue->Get(ToString(iso, "schemaVersion"));
                        if (versionValue->IsNumber()) {
                            config.schema_version = versionValue->NumberValue();
                        }
                        else {
                            throw std::runtime_error("Property 'schemaVersion' expected to be a number.");
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
            NodeDefaults(realm.get()) = defaults;
            NodePrototypes(realm.get()) = prototypes;

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

Handle<Object> RealmWrap::Create(Isolate* ctx, realm::SharedRealm* realm) {
    EscapableHandleScope scope(ctx);

    Local<Function> cons = Local<Function>::New(ctx, constructor);
    Handle<Object> obj = cons->NewInstance(0, nullptr);
    
    RealmWrap* rw = ObjectWrap::Unwrap<RealmWrap>(obj);
    rw->m_realm = *realm;
    return scope.Escape(obj);
}

void RealmWrap::GetDefaultPath(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    info.GetReturnValue().Set(ToString(iso, s_defaultPath.c_str()));
}

void RealmWrap::SetDefaultPath(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void>& info) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        s_defaultPath = ToString(value);
    }
    catch (std::exception &ex) {
        makeError(iso, ex.what());
        info.GetReturnValue().SetUndefined();
    }
}

void RealmWrap::GetPath(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);
    RealmWrap* rw = ObjectWrap::Unwrap<RealmWrap>(info.This());
    realm::SharedRealm shared_realm = rw->m_realm;

    info.GetReturnValue().Set(ToString(iso, shared_realm.get()->config().path.c_str()));
}

void RealmWrap::CreateObject(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        ValidateArgumentRange(args.Length(), 2, 3);
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
        makeError(iso, ex.what());
        args.GetReturnValue().SetUndefined();
        return;
    }
}

void RealmWrap::Write(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);
	
    try {
        ValidateArgumentCount(args.Length(), 1);
		Local<Function> fun = ValidatedValueToFunction(iso, args[0]);
		RealmWrap* rw = ObjectWrap::Unwrap<RealmWrap>(args.This());
		realm::SharedRealm shared_realm = rw->m_realm;
		shared_realm->begin_transaction();
        TryCatch trycatch;
		fun->Call(v8::Context::New(iso)->Global(), 0, NULL);
		if (trycatch.HasCaught()) {
			shared_realm->cancel_transaction();
		    trycatch.ReThrow();
		 }
		 else {
		 	shared_realm->commit_transaction();
		  }
    }
	catch (std::exception &ex) {
		makeError(iso, ex.what());
	}
	args.GetReturnValue().SetUndefined();
}

void RealmWrap::DeleteAll(const FunctionCallbackInfo<Value>& args) {
	Isolate* iso = Isolate::GetCurrent();
	HandleScope scope(iso);
	
	try {
		ValidateArgumentCount(args.Length(), 0);
		RealmWrap* rw = ObjectWrap::Unwrap<RealmWrap>(args.This());
		realm::SharedRealm realm = rw->m_realm;
		if (!realm->is_in_transaction()) {
			throw std::runtime_error("Can only delete objects within a transaction.");
		}
		
		for (auto objectSchema : *realm->config().schema) {
			realm::ObjectStore::table_for_object_type(realm->read_group(), objectSchema.name)->clear();
		}
	}
	catch (std::exception& ex) {
		makeError(iso, ex.what());
	}
	args.GetReturnValue().SetUndefined();
}

void delete_object(realm::SharedRealm realm, realm::Object* object) {
    if (!realm->is_in_transaction()) {
        throw std::runtime_error("Argument to 'delete' must be a Realm object or a collection of Realm objects.");
    }

    realm::TableRef table = realm::ObjectStore::table_for_object_type(realm->read_group(), object->get_object_schema().name);
    table->move_last_over(object->row().get_index());  
}

void RealmWrap::Delete(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    
    try {
        ValidateArgumentCount(args.Length(), 1);

        RealmWrap* rw = ObjectWrap::Unwrap<RealmWrap>(args.This());
        realm::SharedRealm realm = rw->m_realm;
                
        if (args[0]->IsArray()) { // or result or list
            v8::Array* array = v8::Array::Cast(*args[0]);
            size_t length = array->Length();
            for (long i = length-1; i >= 0; i--) {
                RealmObjectWrap* row = ObjectWrap::Unwrap<RealmObjectWrap>(args[0]->ToObject());
                realm::Object* object = row->m_object;
                delete_object(realm, object);              
            }
        }
        
        // FIXME: Check if RealmObject
        RealmObjectWrap* row = ObjectWrap::Unwrap<RealmObjectWrap>(args[0]->ToObject());
        realm::Object* object = row->m_object;
        
        delete_object(realm, object);
    }
    catch (std::exception& ex) {
        makeError(iso, ex.what());
    }
    args.GetReturnValue().SetUndefined();
}


void RealmWrap::Close(const FunctionCallbackInfo<Value>& args) {
	Isolate* iso = Isolate::GetCurrent();
	HandleScope scope(iso);
	
	try {
		ValidateArgumentCount(args.Length(), 0);
		RealmWrap* rw = ObjectWrap::Unwrap<RealmWrap>(args.This());
		realm::SharedRealm realm = rw->m_realm;
		realm->close();
		// FIXME: remove from cache
	}
    catch (std::exception& ex) {
        makeError(iso, ex.what());
    }
    args.GetReturnValue().SetUndefined();
}

void RealmWrap::Objects(const FunctionCallbackInfo<Value>& args) {
	Isolate* iso = Isolate::GetCurrent();
	HandleScope scope(iso);

    try {
        ValidateArgumentCount(args.Length(), 1);
        std::string className = ValidatedStringForValue(iso, args[0], "objectType");
		RealmWrap* rw = ObjectWrap::Unwrap<RealmWrap>(args.This());
		realm::SharedRealm realm = rw->m_realm;
        Local<Value> results = RealmResultsWrap::Create(iso, realm, className);
        args.GetReturnValue().Set(results);
    }
    catch (std::exception& ex) {
        makeError(iso, ex.what());
        args.GetReturnValue().SetUndefined();
    }
}

void RealmWrap::AddListener(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        ValidateArgumentCount(args.Length(), 2);
        __unused std::string name = ValidatedNotificationName(iso, args[0]);
        Local<Function> callback = ValidatedValueToFunction(iso, args[1]);

        RealmWrap* rw = ObjectWrap::Unwrap<RealmWrap>(args.This());
        realm::SharedRealm realm = rw->m_realm;
        static_cast<NodeRealmDelegate*>(realm->m_binding_context.get())->add_notification(callback);
    }
    catch (std::exception& ex) {
        makeError(iso, ex.what());
    }
    args.GetReturnValue().SetUndefined();
}

void RealmWrap::RemoveListener(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        ValidateArgumentCount(args.Length(), 2);
        __unused std::string name = ValidatedNotificationName(iso, args[0]);
        Local<Function> callback = ValidatedValueToFunction(iso, args[1]);

        RealmWrap* rw = ObjectWrap::Unwrap<RealmWrap>(args.This());
        realm::SharedRealm realm = rw->m_realm;
        static_cast<NodeRealmDelegate*>(realm->m_binding_context.get())->remove_notification(callback);        
    }
    catch (std::exception& ex) {
        makeError(iso, ex.what());
    }
    args.GetReturnValue().SetUndefined();
}

void RealmWrap::RemoveAllListeners(const FunctionCallbackInfo<Value>& args) {
    Isolate* iso = Isolate::GetCurrent();
    HandleScope scope(iso);

    try {
        ValidateArgumentRange(args.Length(), 0, 1);
        if (args.Length()) {
            ValidatedNotificationName(iso, args[0]);
        }

        RealmWrap* rw = ObjectWrap::Unwrap<RealmWrap>(args.This());
        realm::SharedRealm realm = rw->m_realm;
        static_cast<NodeRealmDelegate*>(realm->m_binding_context.get())->remove_all_notifications();        
    }
    catch (std::exception& ex) {
        makeError(iso, ex.what());
    }
    args.GetReturnValue().SetUndefined();
}
