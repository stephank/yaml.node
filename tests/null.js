var testutil = require('../testutil');

testutil.withYamlAndJs('null', [
  null,
  {
    'empty': null,
    'canonical': null,
    'english': null,
    'null': 'null key'
  },
  {
    'sparse': [null, '2nd entry', null, '4th entry', null]
  }
]);
