project('pacmanager', 'cpp')


gtkmm = dependency('gtkmm-4.0', version: '>= 4.6.0')
thread_dep = dependency('jthreads','threads')
boost_dep = dependency('boost')
catch2_dep = dependency('catch2')

add_project_arguments('-std=c++23', language: 'cpp')
add_project_arguments('-lstdc++', language: 'cpp')
add_project_arguments('-g', language: 'cpp')

#add_project_arguments(' -fsanitize=thread', language: 'cpp')
executable(
    'main', 
    './src/main.cpp', 
    dependencies: [gtkmm, thread_dep, boost_dep, catch2_dep], 
    include_directories: include_directories('./src'),
)

test_exe = executable(
    'test_exe',
    './src/test.cpp',
    dependencies: [gtkmm, thread_dep, boost_dep, catch2_dep],
    include_directories: include_directories('./src'),
)

test('test-logic', test_exe)

