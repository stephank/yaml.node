var testutil = require('../testutil');

testutil.simple('datetime', [
  {
    'canonical':        new Date('2001-12-15T02:59:43.10Z'),
    'valid iso8601':    new Date('2001-12-14T21:59:43.10-05:00'),
    'space separated':  new Date('2001-12-14T21:59:43.10-05:00'),
    'no time zone (Z)': new Date('2001-12-15T02:59:43.10Z'),
    'date (00:00:00Z)': new Date('2002-12-14T00:00:00.00Z')
  }
]);
