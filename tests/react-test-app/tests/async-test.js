/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential
 */

'use strict';

const Realm = require('realm');
const Asserts = require('realm-tests/asserts');

const OBJECT_SCHEMA = {
    name: 'UniqueObject',
    primaryKey: 'id',
    properties: {
        id: 'int',
    }
};

function createRealm() {
    let realm = new Realm({schema: [OBJECT_SCHEMA]});

    realm.write(() => {
        for (let i = 0; i < 10; i++) {
            realm.create('UniqueObject', {id: i});
        }
    });

    return realm;
}

module.exports = {
    afterEach() {
        Realm.clearTestState();
    },

    testAsyncQuery() {
        let realm = createRealm();
        let objects = realm.objects('UniqueObject');

        var ret = objects.filteredSnapshot((results) => {
            Asserts.assertEqual(results.count, 5);
        }, 'id < 5');
        Asserts.assertTrue(ret == undefined);
    },
};
