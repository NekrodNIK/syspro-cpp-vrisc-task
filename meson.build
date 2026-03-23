project(
  'syspro-cpp-vrisc-task',
  'cpp',
  version : '0.1',
  meson_version : '>= 1.3.0',
  default_options : ['warning_level=3', 'cpp_std=c++23'],
)

tests = [
  'test_emulator.cpp',
]

exe = executable(
  'emulator',
  'main.cpp',
  install : true,
)

gtest_dep = dependency('gtest', main : true)
e = executable('testexe', tests, dependencies : gtest_dep)
test('gtest test', e)
