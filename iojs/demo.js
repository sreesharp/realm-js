require('Realm.js');

function PersonObject() {}
PersonObject.prototype.schema = {
  name: 'Person',
  properties: [
    {name: 'name', type: Realm.Types.STRING},
    {name: 'id',   type: Realm.Types.INT},
  ]
};
PersonObject.prototype.description = function() {
    return this.name + ' ' + this.age;
};

var realm = new Realm({schema: [PersonObject]});

var prompt = require('prompt');

prompt.start();

prompt.get(['name', 'id'], function (err, result) {
if (err) { return onErr(err); }
	console.log('Inserting into Realm:');
	console.log('  name: ' + result.name);
	console.log('  id: ' +   result.id);

	realm.write(function() {
		realm.create("Person", [result.name, result.id]);
	});
});

function onErr(err) {
	console.log(err);
	return 1;
}
