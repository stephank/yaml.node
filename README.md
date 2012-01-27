**YAML.node** is a simple binding for LibYAML. [![Build Status](https://secure.travis-ci.org/stephank/yaml.node.png)](http://travis-ci.org/stephank/yaml.node)

To use, make sure libYAML headers are installed, and then:

    $ npm install libyaml
    $ node
    > yaml = require('libyaml');
    > yaml.load('Hello world!');

To hack the code:

    git clone https://github.com/stephank/yaml.node.git
    cd yaml.node
    npm install
    npm test
