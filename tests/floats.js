var testutil = require('../testutil');

testutil.simple('floats', [
  {
    'canonical': 685230.15,
    'exponential': 685230.15,
    'fixed': 685230.15,
    'sexagesimal': 685230.15,
    'infinity': Infinity,
    'negative infinity': -Infinity,
    'not a number': NaN
  }
]);
