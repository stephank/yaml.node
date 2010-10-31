#!/usr/bin/env coffee
# YAML.node, © 2010 Stéphan Kochen
# MIT-licensed. (See the included LICENSE file.)

YAML = require './yaml'

test = '''
    ---
    item:
      seq: [1,2,3,-9]
      float: 5.4e3
      moreNumbers: [-3.5,+7,+inf,NaN]
      string: this is some text
    other:
      # Comment
      booleans: [true, fALse, YES, no, On, oFF]
      nothing: ~

    ---
    - New document
    - Has a sequence as root
  '''

for document, i in YAML.load(test)
  console.log "Document ##{i+1}"
  console.log document
  console.log ''
