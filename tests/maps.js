var testutil = require('../testutil');

testutil.withYamlAndJs('maps', [
  { one: 1, two: 2, three: 3 },
  { one: 1, two: 2, three: 3 }
]);
