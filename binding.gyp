{
  'targets': [
    {
      'target_name': 'binding',
      'include_dirs': ["<!(node -p -e \"require('path').dirname(require.resolve('nan'))\")"],
      'sources': [ 'binding.cc' ],
      'dependencies': [
        'deps/yaml/yaml.gyp:yaml'
      ]
    }
  ]
}
