{
  'variables': {
    'ss_use_dtrace%': 'false',
    'ss_jemalloc': 'false',
    # ss_parent_path is the relative path to libuv in the parent project
    # this is only relevant when dtrace is enabled and libuv is a child project
    # as it's necessary to correctly locate the object files for post
    # processing.
    # XXX gyp is quite sensitive about paths with double / they don't normalize
    'ss_parent_path': '/',
  },

  'target_defaults': {
    'conditions': [
      ['OS != "win"', {
        'defines': [
          '_LARGEFILE_SOURCE',
          '_FILE_OFFSET_BITS=64',
        ],
        'conditions': [
          ['OS=="solaris"', {
            'cflags': [ '-pthreads' ],
          }],
          ['OS not in "solaris android"', {
            'cflags': [ '-pthread' ],
          }],
        ],
      }],
    ],
  },

  'targets': [
    {
      'target_name': 'stresstest',
      'type': 'executable',
	  'include_dirs': [
		'include',
		'src/',
		'third/libuv/include',
		'third/threadpool',
		'third/jemalloc/include/jemalloc',
		'third/jemalloc/include/msvc_compat',
		'third/EASTL/include',
		'third/tcc/libtcc',
	  ],
      'sources': [
        'src/Main.cpp',
        'src/commondef.cpp',
        'src/commondef.h',
        'src/define.h',
        'src/StressTest.h',
        'src/task.h',
        'src/task.cpp',
        'src/http_task.cpp',
        'src/http_task.h',
        'src/threadpool.h',
        'src/threadpool.cpp',
        'third/hash/hash.h',
        'third/hash/hash.c',
        'third/hash/jenkins_hash.h',
        'third/hash/jenkins_hash.c',
        'third/hash/murmur3_hash.h',
        'third/hash/murmur3_hash.c',
      ],
	  
		'defines': [
		'_UNICODE',
		'UNICODE',
		],
			  
      'conditions': [
        [ 'OS=="win"', 
			{
			  'defines': [
				'_WIN32_WINNT=0x0600',
				'_GNU_SOURCE',
			  ],
			  
			  'msvs_settings': {
				'VCLinkerTool': {
				  'AdditionalLibraryDirectories': [
					'third\\libuv\\$(Configuration)\\lib',
					'third\\eastl\\lib\\$(Configuration)',
					'third\\tcc\\',
				  ],
				  'GenerateDebugInformation': 'true',
				  'SubSystem': 1,
				},
			  },
			  
			  'link_settings': {
				'libraries': [
				  '-ladvapi32',
				  '-liphlpapi',
				  '-lpsapi',
				  '-lshell32',
				  '-lws2_32',
				  '-llibuv',
				  '-lEASTL',
				  '-llibtcc'
				],
			  },
        }, { # Not Windows i.e. POSIX
          'cflags': [
            '-g',
            '-pedantic',
            '-Wall',
            '-Wextra',
            '-Wno-unused-parameter',
          ],
		  
		  
			'configurations': {
				'Debug': {
					  'library_dirs': [
						'../third/libuv/out/Debug',
					  ],
				},
				'Release': {
					  'library_dirs': [
						'../third/libuv/out/Release',
					  ],
				}
			},
			
			'configurations': 
				['ss_jemalloc=="true"', {
					'Debug': {
						  'library_dirs': [
							'../third/jemalloc/out/Debug',
						  ],
					},
					'Release': {
						  'library_dirs': [
							'../third/jemalloc/out/Release',
						  ],
					}
				},
			],
			
          'link_settings': {
            'libraries':[  '-lm',
							'-luv' 
						],
            'conditions': [
              ['OS=="solaris"', {
                'ldflags': [ '-pthreads' ],
              }],
              ['OS != "solaris" and OS != "android"', {
                'ldflags': [ '-pthread' ],
              }],
            ],
          },
		  
          'link_settings': [
			'ss_jemalloc=="true"', {
				'libraries':[  '-ljemalloc' ],
			},
          ],
        }],
        [ 'OS=="mac"', {
          'defines': [
            '_DARWIN_USE_64_BIT_INODE=1',
          ]
        }],
        [ 'OS!="mac"', {
          # Enable on all platforms except OS X. The antique gcc/clang that
          # ships with Xcode emits waaaay too many false positives.
          'cflags': [ '-Wstrict-aliasing' ],
        }],
        [ 'OS=="linux"', {
          'link_settings': {
            'libraries': [ '-ldl', '-lrt' ],
          },
        }],
        [ 'OS=="android"', {
          'link_settings': {
            'libraries': [ '-ldl' ],
          },
        }],
        [ 'OS=="solaris"', {
          'defines': [
            '__EXTENSIONS__',
            '_XOPEN_SOURCE=500',
          ],
          'link_settings': {
            'libraries': [
              '-lkstat',
              '-lnsl',
              '-lsendfile',
              '-lsocket',
            ],
          },
        }],
        [ 'OS=="aix"', {
          'defines': [
            '_ALL_SOURCE',
            '_XOPEN_SOURCE=500',
          ],
          'link_settings': {
            'libraries': [
              '-lperfstat',
            ],
          },
        }],
        [ 'OS in "freebsd dragonflybsd openbsd netbsd".split()', {
          'link_settings': {
            'libraries': [ '-lkvm' ],
          },
        }],
        # FIXME(bnoordhuis or tjfontaine) Unify this, it's extremely ugly.
        ['ss_use_dtrace=="true"', {
          'defines': [ 'HAVE_DTRACE=1' ],
          'dependencies': [ 'uv_dtrace_header' ],
          'include_dirs': [ '<(SHARED_INTERMEDIATE_DIR)' ],
        }],
      ]
    },
  ]
}
