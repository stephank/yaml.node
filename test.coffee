#!/usr/bin/env coffee
# YAML.node, © 2010 Stéphan Kochen
# MIT-licensed. (See the included LICENSE file.)

YAML = require './'

test = '''
    ---
    item:
      seq: [1, 2, 3, -9]
      float: 5.4e+3
      moreNumbers: [-3.5, +7, +.inf, .NaN]
      string: this is some text
    other:
      # Comment
      booleans: [true, fALse, YES, no, On, oFF]
      nothing: ~
      times:
        simple: 01:50:43
        fractional: 01:50:43.83
        negative: -01:50:43
    Scalar examples from the official spec:
      bool:
        canonical: y
        answer: NO
        logical: True
        option: on
      float:
        canonical: 6.8523015e+5
        exponentioal: 685.230_15e+03
        fixed: 685_230.15
        sexagesimal: 190:20:30.15
        negative infinity: -.inf
        not a number: .NaN
      int:
        canonical: 685230
        decimal: +685_230
        octal: 02472256
        hexadecimal: 0x_0A_74_AE
        binary: 0b1010_0111_0100_1010_1110
        sexagesimal: 190:20:30
      timestamp:
        canonical:        2001-12-15T02:59:43.1Z
        valid iso8601:    2001-12-14t21:59:43.10-05:00
        space separated:  2001-12-14 21:59:43.10 -5
        no time zone (Z): 2001-12-15 2:59:43.10
        date (00:00:00Z): 2002-12-14
      string: abcd

    ---
    - New document
    - Has a sequence as root

    # The following is from the `null` scalar example of the official specification.

    # A document may be null.
    ---
    ---
    # This mapping has four keys,
    # one has a value.
    empty:
    canonical: ~
    english: null
    ~: null key
    ---
    # This sequence has five
    # entries, two have values.
    sparse:
      - ~
      - 2nd entry
      -
      - 4th entry
      - Null
  '''

console.log "===== Parser"
for document, i in YAML.load(test)
  console.log "----- Document ##{i+1}"
  console.log document
  console.log ''

console.log "===== Emitter"
console.log YAML.dump(
  'Hello world!',
  null,
  ['Hello array document!', 'element!', 'moar'],
  { hello: 'world' }
)
