**YAML.node** is a quick and dirty binding for LibYAML.

To use, make sure libYAML headers are installed, and then:

    $ npm install libyaml
    $ node
    > yaml = require('libyaml');
    > yaml.load('Hello world!');

To manually build and run the simple test:

    node-waf configure build
    LD_LIBRARY_PATH=. coffee ./test.coffee
