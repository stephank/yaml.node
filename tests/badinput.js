var test = require('tap').test;
var testutil = require('../testutil');
var yaml = require('../yaml');

test('badinput', function(t) {
  t.throws(function() {
    yaml.loadFileSync(testutil.inputPath('badinput'));
  }, {
    name: "Error",
    message: "did not find expected key, while parsing a block mapping, on line 2"
  });
  t.end();
});
