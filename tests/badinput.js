var test = require('tap').test;
var testutil = require('../testutil');
var yaml = require('../yaml');

test('badinput', function(t) {
  t.plan(1);

  t.throws(function() {
    yaml.loadFileSync(testutil.inputPath(t));
  }, {
    name: "Error",
    message: "did not find expected key, while parsing a block mapping, on line 2"
  });
});
