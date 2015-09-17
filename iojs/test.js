
require('Realm.js');
fs = require('fs')

var testPath = '../tests/';

function evalFile(path, callback) {
	fs.readFile(path, 'utf8', function (err, data) {
	  	if (err) {
	  		callback(err);
	  	}
	  	callback(null, eval(data));
	});
}

evalFile(testPath + 'TestCase.js', function(err, data) { 
	global.TestCase = data; 
	evalFile(testPath + 'TestObjects.js', function(err, testObjects) {  
		for (var name in testObjects) {
			global[name] = testObjects[name];
		};

		var testSuites = [ 'RealmTests.js'];
		for (var i = 0; i < testSuites.length; i++) {
			evalFile(testPath + testSuites[i], function(err, suite) {
			  	if (err) {
			    	return console.log(err);
			  	}

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
			  	};
			});
		}
	});
});
