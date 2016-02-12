
require('Realm.js');
fs = require('fs')
cp = require('child_process')

var testPath = '../tests/';

var testSuites = ['ObjectTests.js', 'RealmTests.js'];
for (var i = 0; i < testSuites.length; i++) {
	var suite = eval(fs.readFileSync(testPath + testSuites[i], 'utf8'));

  	for (var test in suite) {

        var ret = cp.spawnSync('node', ['testcase.js', testSuites[i], test]);
        console.log(ret.stdout.toString());

    	try {
    	fs.unlinkSync("default.realm");
    	fs.unlinkSync("default.realm.lock");
    	fs.unlinkSync("default.realm.log");
    	fs.unlinkSync("default.realm.log_a");
    	fs.unlinkSync("default.realm.log_b");
    	} catch(e) {}
    	try {
    	fs.unlinkSync("test1.realm");
    	fs.unlinkSync("test1.realm.lock");
    	fs.unlinkSync("test1.realm.log");
    	fs.unlinkSync("test1.realm.log_a");
    	fs.unlinkSync("test1.realm.log_b");
    	} catch(e) {}
    	try {
    	fs.unlinkSync("test2.realm");
    	fs.unlinkSync("test2.realm.lock");
    	fs.unlinkSync("test2.realm.log");
    	fs.unlinkSync("test2.realm.log_a");
    	fs.unlinkSync("test2.realm.log_b");
    	} catch(e) {}
  	}
}
