project('gpt-chat', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

cpp_std = 'c++17'
ncurses = dependency('ncurses', required: false)

exe = executable(
  'gpt-chat',
  'chat.hh', 'chat.cc',
  'app.hh', 'app.cc',
  'ncwindow.hh', 'ncwindow.cc',
  'main.cc', 
  dependencies: [ncurses],
  install : true)

test('basic', exe)
