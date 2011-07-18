// YAML.node, © 2010 Stéphan Kochen
// MIT-licensed. (See the included LICENSE file.)

var sys = require('sys');
var binding = exports.capi = require('./binding');

// Most of these were derived from: http://yaml.org/type/
// Also borrows from tenderlove's `Psych::ScalarScanner`. (MIT-licensed)
var definitelyNonNumericRe = /^[a-z~]/i,
    definitelyNonBooleanRe = /^[^ytonf~]/i,
    canParseIntRe   = /^[-+]?(0x[0-9a-fA-F_]+|[\d_]+)$/,
    canParseFloatRe = /^[-+]?(\d[\d_]*)?\.[\d_]*(e[-+]\d+)?$/i,
    nullRe   = /^(~|null)$/i,
    trueRe   = /^(y|yes|true|on)$/i,
    falseRe  = /^(n|no|false|off)$/i,
    posInfRe = /^\+?\.inf$/i,
    negInfRe = /^-\.inf$/i,
    nanRe    = /^\.nan$/i,
    binRe    = /^([-+])?0b([01_]+)$/,
    timeRe   = /^([-+]?)([0-9][\d_]*(?::[0-5]?\d)+(?:\.[\d_]*)?)$/,
    dateRe   = /^\d{4}-\d\d?-\d\d?$/,
    timestampRe = /^(\d{4}-\d\d?-\d\d?(?:[Tt]|\s+)\d\d?:\d\d:\d\d(?:\.\d*)?)(?:\s*(Z|[-+]\d\d?(?::\d\d)?))?$/,
    underscoresRe = /_/g;

// Helper function that converts a string scalar to its actual type.
var parseScalar = function(v) {
  if (!v) return null;

  // Simple keywords.
  if (definitelyNonNumericRe.test(v)) {
    if (v.length > 5 || definitelyNonBooleanRe.test(v)) return v;
    if (nullRe.test(v)) return null;
    if (trueRe.test(v)) return true;
    if (falseRe.test(v)) return false;
    return v;
  }
  if (posInfRe.test(v)) return  1/0;
  if (negInfRe.test(v)) return -1/0;
  if (nanRe.test(v))    return  0/0;

  // JavaScript's `parseInt` does not support binary numbers.
  var m = binRe.exec(v);
  if (m) {
    var s = m[2], length = s.length; result = 0, i;
    for (i = 0; i < length; i++) {
      if (s[i] == '_')
        continue;
      result *= 2;
      if (s[i] == '1')
        result++;
    }
    if (m[1] == '-')
      result *= -1;
    return result;
  }

  // JavaScript's datetime parsing is subtly different from YAML's.
  var m = timestampRe.exec(v);
  if (m) {
    var offset = 0, dateTimePart = m[1].replace('t', 'T');
    if (m[2] && m[2] !== 'Z') {
      var parts = m[2].split(':');
      offset = parseInt(parts[0]) * 100;
      if (parts.length == 2)
        offset += parseInt(parts[1]);
    }
    if (offset >= 0)
      offset = "+" + offset;
    return new Date(dateTimePart + offset);
  }
  if (dateRe.test(v))
    return new Date(v + "T00:00:00Z");

  // Regular numbers.
  if (canParseIntRe.test(v))   return parseInt(  v.replace(underscoresRe, ''));
  if (canParseFloatRe.test(v)) return parseFloat(v.replace(underscoresRe, ''));

  // Times.
  var m = timeRe.exec(v);
  if (m) {
    var parts = m[2].split(':'), length = parts.length, result = 0, i;
    for (i = 0; i < parts.length; i++) {
      result *= 60;
      if (i + 1 == parts.length)
        result += parseFloat(parts[i]);
      else
        result += parseInt(parts[i]);
    }
    if (m[1] == '-')
      result *= -1;
    return result;
  }

  return v;
};

// `parse` is the binding to LibYAML's parser. It's signature is:
//
//     function(input, handler) { [native] }
//
// The handler object exposes methods for each LibYAML parser event. These are named `onScalar`,
// `onSequenceStart`, etc. All of these methods take an event object that is similar in structure
// to a flattened `yaml_event_t`.
exports.parse = binding.parse;

// The `load` function loads all of a YAML-file's contents into a JavaScript structure. The return
// value is an array of documents found in the input.
exports.load = function(input) {
  var documents = [],
      document;

  // A stack of value handlers, as they occur.
  // Document, sequence and mapping events add functions to this stack.
  var stack = [],
      newValue = function(v) {
        stack[0](v);
      },
      pushHandler = function(h) {
        stack.unshift(h);
      },
      popHandler = function() {
        stack.shift();
      };

  // Call into the parser and build the documents.
  // FIXME: Handle tags and anchors.
  binding.parse(input, {
    onDocumentStart: function(e) {
      pushHandler(function(v) {
        document = v;
      });
    },
    onDocumentEnd: function(e) {
      popHandler();
      documents.push(document);
    },

    onScalar: function(e) {
      newValue(parseScalar(e.value));
    },

    onSequenceStart: function(e) {
      var sequence = [];
      newValue(sequence);
      pushHandler(function(v) {
        sequence.push(v);
      });
    },
    onSequenceEnd: function(e) {
      popHandler();
    },

    onMappingStart: function(e) {
      var mapping = {},
          key,
          keyHandler = function(v) {
            key = v;
            pushHandler(valueHandler);
          },
          valueHandler = function(v) {
            mapping[key] = v;
            popHandler();
          };
      newValue(mapping);
      pushHandler(keyHandler);
    },
    onMappingEnd: function(e) {
      popHandler();
    }
  });

  return documents;
};

var serialize = function(emitter, item) {
  // FIXME: throw on circulars
  switch (typeof item) {
    case "string":
      emitter.scalar(item);
      break;
    case "object":
      if (!item) {
        emitter.scalar('~');
      }
      else if (item.length) {
        emitter.sequence(function() {
          var length = item.length;
          for (var i = 0; i < length; i++)
            serialize(emitter, item[i]);
        });
      }
      else {
        emitter.mapping(function() {
          for (var p in item) {
            if (Object.hasOwnProperty.call(item, p)) {
              emitter.scalar(p);
              serialize(emitter, item[p]);
            }
          }
        });
      }
      break;
    default:
      // FIXME: Properly serialize some special scalars (inf, datetimes, etc.)
      emitter.scalar(String(item));
      break;
  }
};

exports.dump = function() {
  var emitter = new binding.Emitter(),
      documents = arguments;
  emitter.stream(function() {
    var length = documents.length;
    for (var i = 0; i < length; i++) {
      var document = documents[i];
      emitter.document(function() {
        serialize(emitter, document);
      });
    }
  });
  return emitter.chunks.join('');
};
