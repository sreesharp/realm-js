{
  "targets": [
    {
      "target_name": "Realm",
      "sources": [ 
        	"../src/object-store/object_accessor.cpp",
		    "../src/object-store/object_schema.cpp",
		    "../src/object-store/object_store.cpp",
		    "../src/object-store/results.cpp",
		    "../src/object-store/index_set.cpp",
		    "../src/object-store/schema.cpp",
		    "../src/object-store/transact_log_handler.cpp",
        "iorealm.cc",
        "realm.cc",
        "realmobject.cc",
        "realmresults.cc",
        "realmarray.cc"
      ],
      "include_dirs": [ "../core/include" ],
      "xcode_settings" : {
        'OTHER_CFLAGS' : [ '-DREALM_ENABLE_REPLICATION', '-mmacosx-version-min=10.8', '-std=c++14', '-stdlib=libc++', '-fexceptions', '-frtti' ],
        'OTHER_LDFLAGS': [ '-L../core/', '-lrealm' ]
      },
    }
  ]
}