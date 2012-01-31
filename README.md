**YAML.node** is a simple binding for LibYAML. [![Build Status](https://secure.travis-ci.org/stephank/yaml.node.png)](http://travis-ci.org/stephank/yaml.node)

Make sure you're on Node.js 0.4 or 0.6, and have LibYAML headers installed. Then simply:

    $ npm install libyaml

### Usage

    var YAML = require('libyaml');

There's a basic JSON-like API:

    var documents = YAML.parse('Hello world!');
    var data = YAML.stringify({ fancy: ['object', 'structure'] });

And also a `fs`-like API:

    YAML.readFile('myfile.yml', function(error, documents) {
      /* ... */
    });

    var doc1 = { first: 'document' };
    var doc2 = { another: 'doc' };
    YAML.writeFile('myfile.yml', doc1, doc2, function(error) {
      /* ... */
    });

Including synchronous variants:

    var documents = YAML.readFileSync('myfile.yml');
    YAML.writeFileSync('myfile.yml', doc1, doc2);

### Hacking the code

    git clone https://github.com/stephank/yaml.node.git
    cd yaml.node
    npm install
    npm test
