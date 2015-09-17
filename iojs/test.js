
require('Realm.js');
fs = require('fs')

var testPath = '../tests/';

require(testPath + 'TestObjects.js');
require(testPath + 'TestCase.js');

var testSuites = [ 'RealmTests.js'];
for (var i = 0; i < testSuites.length; i++) {
	fs.readFile(testPath + testSuites[i], 'utf8', function (err, data) {
	  	if (err) {
	    	return console.log(err);
	  	}

	  	var suite = eval(data);	
		console.log(suite);
	});
}

