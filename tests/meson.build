example_sources = ['test.cpp']

test_exe = executable(
    meson.project_name(), 
    example_sources, 
    dependencies: [vsm_dep, doctest_dep],
)

test('vsm_test', test_exe)