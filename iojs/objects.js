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
var all = realm.objects('Person');
console.log(all);