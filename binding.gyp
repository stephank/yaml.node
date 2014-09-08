{
  'targets': [
    {
      'target_name': 'binding',
      'sources': [ 'binding.cc' ],
      'defines': [
        'YAML_DECLARE_STATIC'
      ],
      'dependencies': [
        'deps/yaml/yaml.gyp:yaml'
      ]
    }
  ]
}
