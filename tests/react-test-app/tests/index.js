/* Copyright 2016 Realm Inc - All Rights Reserved
 * Proprietary and Confidential
 */

'use strict';

const React = require('react-native');
const Realm = require('realm');
const RealmTests = require('realm-tests');

RealmTests.registerTests({
    ListViewTest: require('./listview-test'),
    AsyncTest: require('./async-test'),
});

const {
    NativeAppEventEmitter,
    NativeModules,
} = React;

module.exports = {
    runTests,
};

// Listen for event to run a particular test.
NativeAppEventEmitter.addListener('realm-run-test', async ({suite, name}) => {
    let error;
    try {
        await RealmTests.runTest(suite, name);
    } catch (e) {
        error = '' + e;
    }

    NativeModules.Realm.emit('realm-test-finished', error);
});

// Inform the native test harness about the test suite once it's ready.
setTimeout(() => {
    NativeModules.Realm.emit('realm-test-names', RealmTests.getTestNames());
}, 0);

async function runTests() {
    let testNames = RealmTests.getTestNames();

    for (let suiteName in testNames) {
        let testSuite = RealmTests[suiteName];

        console.log('Starting ' + suiteName);

        for (let testName of testNames[suiteName]) {
            await RealmTests.runTest(suiteName, 'beforeEach');

            try {
                await RealmTests.runTest(suiteName, testName);
                console.log('+ ' + testName);
            }
            catch (e) {
                console.warn('- ' + testName);
                console.warn(e.message);
            }
            finally {
                await RealmTests.runTest(suiteName, 'afterEach');
            }
        }
    }
}
