project('pacmanager', 'cpp', default_options : ['cpp_std=c++20'] )

gtkmm = dependency('gtkmm-4.0', version: '>= 4.6.0')
thread_dep = dependency('jthreads','threads')

add_project_arguments('-std=c++20', language: 'cpp')
add_project_arguments('-lstdc++', language: 'cpp')



executable(
    'main', 
    './src/main.cpp', 
    dependencies: [gtkmm, thread_dep], 
    include_directories: include_directories('./src'),
)

