global.Realm = require('./build/Release/Realm').Realm;
global.RealmType = {
	'Bool': 	'RealmTypeBool',
	'Int': 		'RealmTypeInt',
	'Float': 	'RealmTypeFloat',
	'Double': 	'RealmTypeDouble',
	'String': 	'RealmTypeString',
	'Date': 	'RealmTypeDate',
	'Data': 	'RealmTypeData',
	'Object': 	'RealmTypeObject',
	'Array': 	'RealmTypeArray',
};
global.Realm.defaultPath = './default.realm';
global.Realm.schemaVersion = 0;