#include <node.h>

#include <string>

#include "realm.h"
#include "realmschema.hpp"
#include "realmutils.hpp"
#include "realmobject.h"

#include "shared_realm.hpp"
#include "object_accessor.hpp"
#include "binding_context.hpp"

#include <set>

using namespace v8;


// FIXME: should be removed
class RJSRealmDelegate : public realm::BindingContext {
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

    RJSRealmDelegate(realm::WeakRealm realm, Isolate* ctx) : m_realm(realm), m_context(ctx) {};

    // FIXME: implement!
    ~RJSRealmDelegate() {};

private:
    realm::WeakRealm m_realm;
    Isolate* m_context;
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

    tpl->InstanceTemplate()->SetAccessor(String::NewFromUtf8(isolate, "defaultPath"), RealmIO::DefaultPathGetter, RealmIO::DefaultPathSetter);
    tpl->InstanceTemplate()->SetAccessor(String::NewFromUtf8(isolate, "schemaVersion"), RealmIO::SchemaVersionGetter, RealmIO::SchemaVersionSetter);
    tpl->InstanceTemplate()->SetAccessor(String::NewFromUtf8(isolate, "path"), RealmIO::PathGetter, RealmIO::PathSetter);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "Realm"), tpl->GetFunction());
}

void RealmIO::DefaultPathGetter(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    RealmIO* r = ObjectWrap::Unwrap<RealmIO>(info.This());
    info.GetReturnValue().Set(ToString(isolate, r->defaultPath.c_str()));
}

void RealmIO::DefaultPathSetter(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void>& info) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    RealmIO* r = ObjectWrap::Unwrap<RealmIO>(info.This());

    if (value->IsString()) {
        r->defaultPath = ToString(value);
    }
    else {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "String expected")));
    }
}

void RealmIO::SchemaVersionGetter(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    RealmIO* r = ObjectWrap::Unwrap<RealmIO>(info.This());
    info.GetReturnValue().Set(Integer::New(isolate, r->schemaVersion));
}

void RealmIO::SchemaVersionSetter(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void>& info) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    RealmIO* r = ObjectWrap::Unwrap<RealmIO>(info.This());

    if (value->IsNumber()) {
        r->schemaVersion = value->IntegerValue();
    }
    else {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Integer expected")));
    }
}

void RealmIO::PathGetter(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    RealmIO* r = ObjectWrap::Unwrap<RealmIO>(info.This());
    info.GetReturnValue().Set(ToString(isolate, r->path.c_str()));
}

void RealmIO::PathSetter(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void>& info) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    RealmIO* r = ObjectWrap::Unwrap<RealmIO>(info.This());

    if (value->IsString()) {
        r->path = ToString(value);
    }
    else {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "String expected")));
    }
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
            RealmIO* r = new RealmIO();
            realm::Realm::Config config;
            config.cache = false;
            config.read_only = false;
            switch (args.Length()) {
            case 0:
                config.path = writeablePathForFile("default.realm");
                config.schema_version = 0;
                config.schema = nullptr;
                break;
            case 1:
                if (args[0]->IsStringObject()) {
                    v8::String::Utf8Value s(args[0]->ToString());
                    config.path = std::string(*s);
                }
                else if (args[0]->IsObject()) {
                    Local<Object> configValue = args[0]->ToObject();

                    Local<Value> version = configValue->Get(String::NewFromUtf8(iso, "schemaVersion"));
                    if (version->IsUndefined()) {
                        config.schema_version = 0; // default value
                    }
                    else {
                        config.schema_version = static_cast<uint64_t>(version->IntegerValue());
                    }

                    Local<Value> schema = configValue->Get(String::NewFromUtf8(iso, "schema"));
                    if (!schema->IsUndefined()) {
                        config.schema = std::make_unique<realm::Schema>(RealmSchema::ParseSchema(*schema));
                    }

                    Local<Value> path = configValue->Get(String::NewFromUtf8(iso, "path"));
                    if (path->IsUndefined()) {
                        config.path = writeablePathForFile("default.realm");
                    }
                    else {
                        std::string p = ToString(path);
                        if (p == "") {
                            config.path = writeablePathForFile("default.realm");
                        }
                        else {
                            config.path = ToString(path);
                        }
                    }
                }
                else {
                    makeError(iso, "Provide either path or configuration");
                }
                break;
            default:
                makeError(iso, "invalid arguments.");
            }
            realm::SharedRealm realm = realm::Realm::get_shared_realm(config);
            if (!realm->m_binding_context) {
                realm->m_binding_context = std::make_unique<RJSRealmDelegate>(realm, iso);
            }

            r->path = config.path;
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

    try {
        ValidateArgumentRange(args.Length(), 2, 3);
        std::string className = ValidatedStringForValue(args[0], "objectType");
        RealmIO *realm = ObjectWrap::Unwrap<RealmIO>(args.This());
        realm::SharedRealm sharedRealm = realm->realm;
        auto object_schema = sharedRealm->config().schema->find(className);
        if (object_schema == sharedRealm->config().schema->end()) {
            makeError(isolate, "Object type '" + className + "' not found in schema.");
            args.GetReturnValue().SetUndefined();
            return;
        }
        Local<Object> object = ValidatedValueToObject(args[1]);
        if (args[1]->IsArray()) {
            object = DictForPropertyArray(isolate, *object_schema, v8::Local<v8::Array>::Cast(args[1]));
        }

        bool update = false;
        if (args.Length() == 3) {
            update = ValidatedValueToBool(args[2]);
        }

        realm::Object obj = realm::Object::create<Local<Value>>(nullptr, sharedRealm, *object_schema,
             object, update);
        args.GetReturnValue().Set(RealmObject::Create(new realm::Object(obj)));

    } catch (std::exception &ex) {
        makeError(isolate, ex);
        args.GetReturnValue().SetUndefined();
    }
}


void RealmIO::Delete(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

/*   try {
        if (oneArgument(isolate, args)) {

            RealmIO *realm = ObjectWrap::Unwrap<RealmIO>(args.This());
            realm::SharedRealm r = realm->realm;

            if (r->is_in_transaction()) {
                throw std::runtime_error("Can only delete objects within a transaction.");
            }

            if (args[0]->IsArray()) { // FIXME: add RealmArray and RealmResults
                v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(args[0]);
                size_t length = array->Length();
                for (long i = length-1; i >= 0; i--) {
                    v8::Local<v8::Object> object = array->Get(i);
                    realm::Object* r_object = RealmObject::GetObject(object);
                    realm::TableRef table = realm::ObjectStore::table_for_object_type(r->read_group(), r_object->object_schema.name);
                    table->move_last_over(r_object->row.get_index());
                }
            }
            else {
                realm::Object* object = RealmObject::GetObject(args[0]);
                realm::TableRef table = realm::ObjectStore::table_for_object_type(r->read_group(), object->object_schema.name);
                table->move_last_over(object->row.get_index());
            }
        }
    } catch (std::exception &ex) {
        makeError(isolate, ex);
    }
    args.GetReturnValue().SetUndefined();*/
}

void RealmIO::DeleteAll(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    try {
        if (noArgument(isolate, args)) {
            RealmIO *realm = ObjectWrap::Unwrap<RealmIO>(args.This());
            realm::SharedRealm r = realm->realm;
            if (r->is_in_transaction()) {
                throw std::runtime_error("Can only delete objects within a transaction.");
            }
            for (auto objectSchema : *r->config().schema) {
                realm::ObjectStore::table_for_object_type(r->read_group(), objectSchema.name)->clear();
            }
        }
    } catch (std::exception &ex) {
        makeError(isolate, ex);
    }
    args.GetReturnValue().SetUndefined();
}

void RealmIO::Write(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (oneFunctionArgument(isolate, args)) {
        Local<Function> fun = Local<Function>::Cast(args[0]);
        RealmIO *realm = ObjectWrap::Unwrap<RealmIO>(args.This());
        TryCatch trycatch;
        realm->realm->begin_transaction();
        fun->Call(v8::Context::New(isolate)->Global(), 0, NULL);
        if (trycatch.HasCaught()) {
            realm->realm->cancel_transaction();
            trycatch.ReThrow();
        }
        else {
            realm->realm->commit_transaction();
        }
    }
    args.GetReturnValue().SetUndefined();
}
