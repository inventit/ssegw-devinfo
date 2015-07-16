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
        'src/devinfo/ports/ri/devinfo_collector.c',
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
            'src/devinfo/ports/armadillo_iot/devinfo_collector.c',
          ],
        }],
      ],
    },
  ],
}
