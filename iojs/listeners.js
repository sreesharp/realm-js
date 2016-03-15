fs = require('fs');
require('Realm.js');

var filename = 'objects.realm';

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


try { fs.unlinkSync(filename); } catch (ignore) {}
var realm = new Realm({path: filename, schema: [PersonObject]});

var notificationCount = 0;
var notificationName;

realm.addListener('change', function(realm, name) {
  notificationCount++;
  notificationName = name;
});

console.log('notifications : ' + notificationCount);

realm.write(function() {
  realm.create('Person', ['Douglas', 42]);
});

console.log('notifications : ' + notificationCount);
