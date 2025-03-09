example_sources = ['doctest.cpp', 'tests.cpp', 'states.cpp']

test_exe = executable(
    meson.project_name(), 
    example_sources, 
    dependencies: [vsm_dep, doctest_dep],
    cpp_args : '-std=c++17',
)

test('vsm_test', test_exe)