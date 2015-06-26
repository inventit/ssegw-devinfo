{
  'variables': {
    'sseutils_root': './ssegw-utils-moatc',
  },
  'includes': [
    'common.gypi',
    'config.gypi',
    './ssegw-utils-moatc/sseutils.gypi',
  ],
  'targets': [
    # your M2M/IoT application
    {
      'target_name': '<(package_name)',
      'sources': [
        '<@(sseutils_src)',
        'src/devinfo/devinfo_repository.c',
        'src/<(package_name).c',
       ],
      'product_prefix': '',
      'type': 'shared_library',
      'cflags': [ '-fPIC' ],
      'include_dirs' : [
        '<(sseutils_include)',
      ],
      'libraries': [
        '-lmoatapp',
      ],
      'dependencies': [
      ],
      'conditions': [
        ['test_config == "unit"', {
          'sources!': [
            'src/<(package_name).c',
          ],
          'sources': [
            'src/<(package_name)_unittest.c',
          ],
        }],
      ]
    },
  ],
}
