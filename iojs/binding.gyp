{
  "targets": [
    {
      "target_name": "Realm",
      "sources": [ 
        "../src/object-store/object_accessor.cpp",
		"../src/object-store/object_schema.cpp",
		"../src/object-store/object_store.cpp",
		"../src/object-store/results.cpp",
		"../src/object-store/shared_realm.cpp",
      ],
      "include_dirs": [ "../core/include" ],
      "xcode_settings" : {
        'OTHER_CFLAGS' : [ '-DREALM_ENABLE_REPLICATION', '-mmacosx-version-min=10.8', '-std=c++14', '-stdlib=libc++', '-fexceptions', '-frtti' ],
        'OTHER_LDFLAGS': [ '-L../core/', '-lrealm' ]
      },
    }
  ]
}