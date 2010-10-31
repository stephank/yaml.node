/*
 * Copyright (c) 2010 Stéphan Kochen
 * MIT-licensed. (See the included LICENSE file.)
 */

var sys = require('sys');
var binding = exports.capi = require('./binding');

exports.load = function(input) {
  var documents = binding.load(input),
      document, i, length = documents.length;
  for (i = 0; i < length; i++) {
    document = documents[i];
    /* FIXME: convert scalars. */
  }
  return documents;
};
