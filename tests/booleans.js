var testutil = require('../testutil');

testutil.withYamlAndJs('booleans', [
  [true, false, true, false, true, false]
]);
