var path = require('path');
var util = require('util');
var _ = require('underscore');
var test = require('tap').test;
var yaml = require('./yaml');


// Test that only provides JS repr.
exports.withJs = function(name, js) {
  exports.dumpAndLoad(name, js);
};

// Test that provides both YAML and JS repr.
exports.withYamlAndJs = function(name, js) {
  exports.load(name, js);
  exports.dumpAndLoad(name, js);
};


// Path helper for YAML input.
exports.inputPath = function(name) {
  return path.resolve(module.filename, '..', 'tests', name + '.yaml');
};


// Load YAML and compare with expected JS repr.
exports.load = function(name, tags, expected) {
  if (expected === undefined) {
    expected = tags;
    tags = undefined;
  }

  test(name, function(t) {
    var file = exports.inputPath(name);
    var input = yaml.loadFileSync(file, tags);

    t.ok(_.isEqual(input, expected), 'load YAML', {
      input: util.inspect(input),
      expected: util.inspect(expected)
    });
    t.end();
  });
};


// Dump JS to YAML, load again, and compare the two.
exports.dumpAndLoad = function(name, input) {
  test(name, function(t) {
    var data = yaml.dump.apply(null, input);
    var output = yaml.load(data);
    t.ok(_.isEqual(input, output), 'dump & load YAML', {
      inputJs: util.inspect(input),
      yaml: data,
      outputJs: util.inspect(output)
    });
    t.end();
  });
};
