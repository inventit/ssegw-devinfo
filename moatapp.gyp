{
  'variables': {
    'sseutils_root': './moat-c-utils',
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
        'ports/ri/src/devinfo_collector.c',
        'src/devinfo/devinfo_repository.c',
        'src/devinfo/devinfo_manager.c',
        'src/devinfo/devinfo_model_command.c',
        'src/<(package_name).c',
       ],
      'product_prefix': '',
      'type': 'shared_library',
      'cflags': [ '-fPIC', '-DDEVINFO_ENABLE_BASE64_ENCODE' ],
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
        ['target_product == "Armadillo-IoT"', {
          'sources': [
            'ports/armadillo_iot/src/devinfo_collector.c',
          ],
        }],
      ],
    },
  ],
}
