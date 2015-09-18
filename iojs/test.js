
require('Realm.js');
fs = require('fs')

var testPath = '../tests/';

global.TestCase = eval(fs.readFileSync(testPath + 'TestCase.js', 'utf8'));; 
var testObjects = eval(fs.readFileSync(testPath + 'TestObjects.js', 'utf8'));
for (var name in testObjects) {
	global[name] = testObjects[name];
};

var testSuites = ['ObjectTests.js', 'RealmTests.js'];
for (var i = 0; i < testSuites.length; i++) {
	var suite = eval(fs.readFileSync(testPath + testSuites[i], 'utf8'));

  	for (var test in suite) {
  		var caught;
  		try {
  			suite[test]();
  		}
  		catch(exception) {
        	caught = exception;
    	}
    	if (!caught) {
    		console.log("PASSED - " + test);
    	}
    	else {
    		console.log("FAILED - " + test + " - " + caught);
    	}

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
