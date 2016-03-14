{
    "targets": [
        {
            "target_name": "Realm",
            "sources": [
                "../src/object-store/src/index_set.cpp",
                "../src/object-store/src/list.cpp",
                "../src/object-store/src/object_schema.cpp",
                "../src/object-store/src/object_store.cpp",
                "../src/object-store/src/results.cpp",
                "../src/object-store/src/schema.cpp",
                "../src/object-store/src/shared_realm.cpp",
                "../src/object-store/src/impl/transact_log_handler.cpp",
                "../src/object-store/src/impl/realm_coordinator.cpp",
                "../src/object-store/src/parser/parser.cpp",
                "../src/object-store/src/parser/query_builder.cpp",
                "node_init.cpp",
                "node_realm.cpp",
                "node_realm_object.cpp",
                "node_realm_schema.cpp",
                "node_realm_results.cpp",
                "node_realm_list.cpp"
            ],
            "include_dirs": [ 
                "../core/include",
                "../src/object-store/src",
                "../src/object-store/src/impl",
                "../src/object-store/src/impl/apple",
                "../src/object-store/src/parser",
                "../src/object-store/external/pegtl"
            ],
            "cflags_cc": [ '-DREALM_HAVE_CONFIG', '-fexceptions', '-frtti', '-std=c++14', '-g', '-O0'],
            "ldflags": [ '-L../core/', '-lrealm' ],
            "xcode_settings" : {
                'OTHER_CFLAGS' : [ '-mmacosx-version-min=10.8', '-DREALM_HAVE_CONFIG', '-fexceptions', '-frtti', '-std=c++14', '-stdlib=libc++', '-g', '-O0', '-Wno-mismatched-tags' ],
                'OTHER_LDFLAGS': [ '-mmacosx-version-min=10.8', '-L../core/', '-lrealm', '-std=c++14' ]
            },
        }
    ]
}
