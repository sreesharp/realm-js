
require('Realm.js');
fs = require('fs')
cp = require('child_process')

var testPath = '../tests/';

global.TestCase = eval(fs.readFileSync(testPath + 'TestCase.js', 'utf8'));; 
var testObjects = eval(fs.readFileSync(testPath + 'TestObjects.js', 'utf8'));
for (var name in testObjects) {
	global[name] = testObjects[name];
};

var suite = eval(fs.readFileSync(testPath + process.argv[2], 'utf8'));

var caught;
try {
	suite[process.argv[3]]();
}
catch(exception) {
    caught = exception;
}

if (!caught) {
	process.stdout.write("PASSED - " + process.argv[3]);
}
else {
	process.stdout.write("FAILED - " + process.argv[3] + " - " + caught);
}

