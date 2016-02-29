{
  "targets": [
    {
      "target_name": "Realm",
      "sources": [
        "../src/object-store/index_set.cpp",
        "../src/object-store/list.cpp",
        "../src/object-store/object_schema.cpp",
        "../src/object-store/object_store.cpp",
        "../src/object-store/results.cpp",
        "../src/object-store/schema.cpp",
        "../src/object-store/shared_realm.cpp",
        "../src/object-store/impl/transact_log_handler.cpp",
        "../src/object-store/impl/apple/external_commit_helper.cpp",
        "../src/object-store/parser/parser.cpp",
        "../src/object-store/parser/query_builder.cpp",
        "node_init.cpp",
        "node_realm.cpp",
        "node_realm_object.cpp",
        "node_realm_schema.cpp",
        "node_realm_results.cpp"
      ],
      "include_dirs": [ 
          "../core/include",
          "../src/object-store",
          "../src/object-store/impl",
          "../src/object-store/impl/apple",
          "../src/object-store/parser",
          "../vendor/PEGTL"
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
