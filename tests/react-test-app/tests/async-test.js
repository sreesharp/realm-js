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

    async testAsyncQuery() {
        let realm = createRealm();
        let objects = realm.objects('UniqueObject');

        let results = await new Promise((resolve) => {
            let ret = objects.filteredSnapshot('id < 5', resolve);
            Asserts.assertTrue(ret == undefined);
        });

        Asserts.assertEqual(results.length, 5);
    },
};
