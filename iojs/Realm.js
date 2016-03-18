global.Realm = require('./build/Release/Realm').Realm;
global.Realm.Types = {
	'BOOL': 	'bool',
	'INT': 		'int',
	'FLOAT': 	'float',
	'DOUBLE': 	'double',
	'STRING': 	'string',
	'DATE': 	'date',
	'DATA': 	'data',
	'OBJECT': 	'object',
	'LIST': 	'list'
};
global.Realm.defaultPath = 'default.realm'; // FIXME: also defined in node_realm.cpp

global.Realm;
