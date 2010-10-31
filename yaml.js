/*
 * Copyright (c) 2010 Stéphan Kochen
 * MIT-licensed. (See the included LICENSE file.)
 */

var sys = require('sys');
var binding = exports.capi = require('./binding');

exports.parse = binding.parse;

// FIXME: Handle tags and anchors. Parse scalars.
exports.load = function(input) {
  var documents = [],
      document;
  
  var stack       = [],
      newValue    = function(v) { stack[0](v); },
      pushHandler = function(h) { stack.unshift(h); },
      popHandler  = function()  { stack.shift(); };

  binding.parse(input, {
    onDocumentStart: function(e) {
      pushHandler(function(v) { document = v; });
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
      pushHandler(function(v) { sequence.push(v); });
    },
    onSequenceEnd: function(e) {
      popHandler();
    },
    onMappingStart: function(e) {
      var mapping = {},
          key,
          keyHandler   = function(v) { key = v; pushHandler(valueHandler); },
          valueHandler = function(v) { mapping[key] = v; popHandler(); };
      newValue(mapping);
      pushHandler(keyHandler);
    },
    onMappingEnd: function(e) {
      popHandler();
    }
  });

  return documents;
};
