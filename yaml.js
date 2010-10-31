/*
 * Copyright (c) 2010 Stéphan Kochen
 * MIT-licensed. (See the included LICENSE file.)
 */

var sys = require('sys');
var binding = exports.capi = require('./binding');

exports.parse = binding.parse;

exports.load = function(input) {
  var documents = [], document = null;
  binding.parse(input, {
    onStreamStart: function(e) {
      console.log("stream start");
      console.log(e);
    },

    onStreamEnd: function(e) {
      console.log("stream end");
      console.log(e);
    },

    onDocumentStart: function(e) {
      console.log("document start");
      console.log(e);
    },

    onDocumentEnd: function(e) {
      console.log("document end");
      console.log(e);
    },

    onAlias: function(e) {
      console.log("alias");
      console.log(e);
    },

    onScalar: function(e) {
      console.log("scalar");
      console.log(e);
    },

    onSequenceStart: function(e) {
      console.log("sequence start");
      console.log(e);
    },

    onSequenceEnd: function(e) {
      console.log("sequence end");
      console.log(e);
    },

    onMappingStart: function(e) {
      console.log("mapping start");
      console.log(e);
    },

    onMappingEnd: function(e) {
      console.log("mapping end");
      console.log(e);
    }
  });
  return documents;
};
