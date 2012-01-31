var fs = require('fs');
var _ = require('underscore');
var test = require('tap').test;
var Yaml = require('../yaml');

test('basic stream parse tests', function(t) {
  var expectedTypes = ['streamStart', 'documentStart', 'scalar', 'documentEnd', 'streamEnd'];
  t.plan(expectedTypes.length * 2);

  var pos;
  var handler = function(ev) {
    t.equal(ev.type, expectedTypes[pos]);
    pos++;
  };

  pos = 0;
  Yaml.stream.parse('foo', handler);

  pos = 0;
  Yaml.stream.parse('foo', {
    onStreamStart: handler,
    onDocumentStart: handler,
    onScalar: handler,
    onDocumentEnd: handler,
    onStreamEnd: handler
  });
});

test('basic stream emit tests', function(t) {
  t.plan(2);

  var expected = '--- foo\n...\n';

  var e;
  var data;
  var handler = function(chunk) {
    data += chunk;
  };

  data = '';
  e = Yaml.stream.createEmitter(handler);
  e.event({ type: 'streamStart' });
  e.event({ type: 'documentStart' });
  e.event({ type: 'scalar', value: 'foo' });
  e.event({ type: 'documentEnd' });
  e.event({ type: 'streamEnd' });
  t.equal(data, expected);

  data = '';
  e = Yaml.stream.createEmitter(handler);
  e.streamStart();
  e.documentStart();
  e.scalar('foo');
  e.documentEnd();
  e.streamEnd();
  t.equal(data, expected);
});

test('basic parse test', function(t) {
  t.plan(1);

  var input = 'foo';
  var expected = ['foo'];

  var result = Yaml.parse(input);
  t.ok(_.isEqual(result, expected), 'should be equal', {
    found: result,
    wanted: expected
  });
});

test('basic stringify test', function(t) {
  t.plan(1);

  var input = 'foo';
  var expected = '--- foo\n...\n';

  var result = Yaml.stringify(input);
  t.ok(_.isEqual(result, expected), 'should be equal', {
    found: result,
    wanted: expected
  });
});

test('basic sync file I/O tests', function(t) {
  t.plan(1);

  var file = '/tmp/yaml.node-sync-test.yml';
  var input = 'foo';

  Yaml.writeFileSync(file, input);
  var result = Yaml.readFileSync(file);
  fs.unlinkSync(file);

  t.ok(_.isEqual(result, [input]), 'should be equal', {
    found: result,
    wanted: [input]
  });
});

test('basic async file I/O tests', function(t) {
  t.plan(1);

  var file = '/tmp/yaml.node-async-test.yml';
  var input = 'foo';

  Yaml.writeFile(file, input, function(error) {
    Yaml.readFile(file, function(error, result) {
      fs.unlinkSync(file);

      t.ok(_.isEqual(result, [input]), 'should be equal', {
        found: result,
        wanted: [input]
      });
    });
  });
});
