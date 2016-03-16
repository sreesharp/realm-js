fs = require('fs');
require('Realm.js');

var filename = 'objects.realm';
var i;

function PersonObject() {}
PersonObject.prototype.schema = {
  name: 'Person',
  properties: [
    {name: 'name', type: Realm.Types.STRING},
    {name: 'id',   type: Realm.Types.INT, optional: true}
  ]
};
PersonObject.prototype.description = function() {
    return this.name + ' ' + this.age;
};


try { fs.unlinkSync(filename); } catch (ignore) {}
var realm = new Realm({path: filename, schema: [PersonObject]});

realm.write(function() {
  realm.create('Person', {'name': 'Jim', 'id': null});
  realm.create('Person', {'name': 'John', 'id': 2});
  realm.create('Person', {'id': 4, 'name': 'Sarah'});
});


var all = realm.objects('Person');
for (i = 0; i < all.length; i++) {
  console.log(i + ': ' + all[i].name + ' ' + all[i].id);
}
