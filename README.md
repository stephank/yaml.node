## YAML.node [![Build Status](https://secure.travis-ci.org/stephank/yaml.node.png)](http://travis-ci.org/stephank/yaml.node)

A simple binding for LibYAML.

### Installing

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

### Upgrading from pre-0.1.0

As of 0.1.0, the API immitates the JavaScript built-in `JSON` parser for string handling, and the
Node.js built-in `fs`-module for file I/O. Function names have changed as follows:

    YAML.load ➞ YAML.parse
    YAML.dump ➞ YAML.stringify

    YAML.loadFile ➞ YAML.readFile
    YAML.dumpFile ➞ YAML.writeFile

    YAML.loadFileSync ➞ YAML.readFileSync
    YAML.dumpFileSync ➞ YAML.writeFileSync

    YAML.parse ➞ YAML.stream.parse
