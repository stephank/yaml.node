#!/usr/bin/env coffee

# YAML.node, © 2010 Stéphan Kochen
# MIT-licensed. (See the included LICENSE file.)

YAML = require './yaml'

test = '''
    ---
    item:
      seq: [1,2,3]
      float: 5.4
      string: this is some text
    other:
      # Comment
      boolean: true

    ---
    - New document
    - Has a sequence as root
  '''

for document, i in YAML.load(test)
  console.log "Document ##{i+1}"
  console.log document
  console.log ''
