var test = require('tap').test;
var testutil = require('../testutil');
var Yaml = require('../yaml');

test('bad parser input', function(t) {
  t.plan(1);

  t.throws(function() {
    Yaml.readFileSync(testutil.inputPath('badinput'));
  }, {
    name: "Error",
    message: "did not find expected key, while parsing a block mapping, on line 2"
  });
});

test('bad emitter input', function(t) {
  t.plan(1);

  var e = Yaml.stream.createEmitter();
  t.throws(function() {
    e.streamStart();
    e.documentStart();
    e.mappingEnd();
  }, {
    name: "Error",
    message: "expected SCALAR, SEQUENCE-START, MAPPING-START, or ALIAS"
  });
});
