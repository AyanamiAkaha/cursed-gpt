project('gpt-chat', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

cpp_std = 'c++17'
ncurses = dependency('ncurses', required: false)
incdir = include_directories('lib')

exe = executable(
  'gpt-chat',
  'lib/httplib.h',
  'lib/json.hpp',
  'src/chat.hh', 'src/chat.cc',
  'src/ncwindow.hh', 'src/ncwindow.cc',
  'src/concurrent_queue.hh', 'src/concurrent_queue.cc',
  'src/gpt_chat.hh', 'src/gpt_chat.cc',
  'src/app.hh', 'src/app.cc',
  'src/main.cc', 
  dependencies: [ncurses],
  include_directories: incdir,
  install : true)

test('basic', exe)
