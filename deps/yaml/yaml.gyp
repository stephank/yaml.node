{
  'target_defaults': {
    'default_configuration': 'Debug',
    'configurations': {
      'Debug': {},
      'Release': {}
    }
  },

  'targets': [
    {
      'target_name': 'yaml',
      'type': 'static_library',
      'include_dirs': [ 'src/include' ],
      'direct_dependent_settings': {
        'include_dirs': [ 'src/include' ]
      },
      'defines': [
        'YAML_VERSION_MAJOR=0',
        'YAML_VERSION_MINOR=1',
        'YAML_VERSION_PATCH=4',
        'YAML_VERSION_STRING="0.1.4"'
      ],
      'sources': [
        'src/src/api.c',
        'src/src/reader.c',
        'src/src/scanner.c',
        'src/src/parser.c',
        'src/src/loader.c',
        'src/src/writer.c',
        'src/src/emitter.c',
        'src/src/dumper.c'
      ]
    }
  ]
}
