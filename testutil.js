var path = require('path');
var util = require('util');
var _ = require('underscore');
var test = require('tap').test;
var YAML = require('./yaml');


// Helper that, given a test name, returns the path to the YAML input file.
var inputPath = exports.inputPath = function(name) {
  return path.resolve(module.filename, '..', 'tests', name + '.yaml');
};


// The basic tests provided have YAML and JSON representations. The YAML is loaded and compared
// to the JSON. In addition, the JSON is dumped to YAML, loaded back in and compared again to
// test our emitter.
exports.simple = function(name, js) {
  test(name, function(t) {
    t.plan(2);
    load(t, js);
    dumpAndLoad(t, js);
  });
};


// Load the YAML for a Test, and compare it with the expected JSON.
var load = function(t, tags, expected) {
  if (expected === undefined) {
    expected = tags;
    tags = undefined;
  }

  var input = YAML.readFileSync(inputPath(t.conf.name), tags);
  t.ok(
    _.isEqual(input, expected),
    'load YAML and compare with JSON',
    {
      input: util.inspect(input),
      expected: util.inspect(expected)
    }
  );
};


// Dump the JSON to YAML, load it back in, and compare with the original.
var dumpAndLoad = function(t, input) {
  var data = YAML.stringify.apply(null, input),
      output = YAML.parse(data);
  t.ok(
    _.isEqual(input, output),
    'dump, load, and compare with the original',
    {
      inputJs: util.inspect(input),
      yaml: data,
      outputJs: util.inspect(output)
    }
  );
};
