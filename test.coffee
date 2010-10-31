#!/usr/bin/env coffee

YAML = require './yaml'

test = '''
    item:
      seq: [1,2,3]
      float: 5.4
      string: this is some text
    other:
      # Comment
      boolean: true
  '''

[document] = YAML.load test
console.log document
