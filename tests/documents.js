var testutil = require('../testutil');

testutil.withYamlAndJs('documents', [
  ['a', 'b', 'c'],
  { first: 1, second: 2 },
  'test',
  3,
  5.2,
  true,
  null
]);
