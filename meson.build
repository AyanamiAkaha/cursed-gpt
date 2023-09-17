project('gpt-chat', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++14'])

exe = executable(
  'gpt-chat',
  'app.hh', 'app.cc',
  'main.cc', 
  install : true)

test('basic', exe)
