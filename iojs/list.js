fs = require('fs');
require('Realm.js');

var filename = 'list.realm';
function OwnerObject() {}
OwnerObject.prototype.schema = {
  name: 'Owner',
  properties: [
    {name: 'name', type: Realm.Types.STRING},
    {name: 'id',   type: Realm.Types.INT},
    {name: 'dogs', type: Realm.Types.LIST, objectType: 'Dog'}
  ]
};
OwnerObject.prototype.description = function() {
    return this.name;
};

function DogObject() {}
DogObject.prototype.schema = {
  name: 'Dog',
  properties: [
    {name: 'name',  type: Realm.Types.STRING}
  ]
};
DogObject.prototype.description = function() {
  return this.name;
};

try { fs.unlinkSync(filename); } catch (ignore) {}
var realm = new Realm({path: filename, schema: [OwnerObject, DogObject]});

realm.write(function() {
  var fido  = realm.create('Dog', ['Fido']);
  var king  = realm.create('Dog', ['King']);
  var owner = realm.create('Owner', ['John', 1, [fido, king]]);
});

console.log('Owners: ' + realm.objects('Owner').length);
console.log('Dogs:   ' + realm.objects('Dog').length);
