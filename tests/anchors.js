var testutil = require('../testutil');

testutil.simple('anchors', [
  {
    base: {
      foo: 3,
      bar: 5
    },
    copy: {
      foo: 3,
      bar: 5
    },
    mergeBefore: {
      foo: 3,
      bar: 15
    },
    mergeAfter: {
      foo: 3,
      bar: 5
    }
  }
]);
