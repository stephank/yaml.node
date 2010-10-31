// YAML.node, © 2010 Stéphan Kochen
// MIT-licensed. (See the included LICENSE file.)

var sys = require('sys');
var binding = exports.capi = require('./binding');

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
      newValue(e.value);
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
