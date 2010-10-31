// YAML.node, © 2010 Stéphan Kochen
// MIT-licensed. (See the included LICENSE file.)

var sys = require('sys');
var binding = exports.capi = require('./binding');

// Helper function that converts a string scalar to its actual type.
// Borrows heavily from tenderlove's `Psych::ScalarScanner`.
var parseScalar = function(v) {
  if (!v) return null;
  if (/^[A-Za-z~]/.test(v)) {
    if (v.length > 5 || /^[^ytonf~]/i.test(v)) return v;
    if (v === '~' || /^null$/i.test(v)) return null;
    if (/^(yes|true|on)$/i.test(v)) return true;
    if (/^(no|false|off)$/i.test(v)) return false;
    return v;
  }
  if ( /^\.inf$/i.test(v)) return  1/0;
  if (/^-\.inf$/i.test(v)) return -1/0;
  if ( /^\.nan$/i.test(v)) return 0/0;
  // FIXME: Time, integer, and float parsing
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
      popHandler = function()  {
        stack.shift();
      };

  // Call into the parser and build the documents.
  // FIXME: Handle tags and anchors. Parse scalars.
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
