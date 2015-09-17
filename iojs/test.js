
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
	  	for (var test in suite) {
	  		var caught;
	  		try {
	  			suite.test();
	  		}
	  		catch(exception) {
            	caught = exception;
        	}
        	if (caught != undefined) {
        		console.log("PASSED - " + test);
        	}
        	else {
        		console.log("FAILED - " + test + " - " + exception);
        	}
	  	};
	});
}

