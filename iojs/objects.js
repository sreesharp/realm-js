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

for (var i = 0; i < 10; i++) {
    realm.write(function() {
        realm.create('Person', ['Name ' + i, i]);
    });
}

console.log('All objects');
var all = realm.objects('Person');
for (var i = 0; i < all.length; i++) {
    console.log(all[i].id + ': ' + all[i].name);
}

console.log('\n\nSome objects: id > 5');
var some = realm.objects('Person').filtered('id > 5');
for (var i = 0; i < some.length; i++) {
    console.log(some[i].id + ': ' + some[i].name);
}

console.log('\n\nGet an object');
var obj = realm.objects('Person')[5];
console.log('Name: ' + obj.name);
realm.write(function() {
    realm.delete(obj)
});
console.log('Length: ' + realm.objects('Person').length);

console.log('\n\nSorted');
var sorted = realm.objects('Person').sorted('id', true);
console.log('First: ' + sorted[0].name);
console.log('Last:  ' + sorted[8].name);