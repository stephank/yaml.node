{
  'targets': [
    {
      'target_name': 'binding',
      'sources': [ 'binding.cc' ],
      'dependencies': [
        'deps/yaml/yaml.gyp:yaml'
      ]
    }
  ]
}
