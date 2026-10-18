project(
    'cubic-splines',
    'cpp',
    version: '0.1',
    meson_version: '>= 1.3.0',
    default_options: ['warning_level=3', 'cpp_std=c++23'],
)

gsl_dep = dependency(
    'Microsoft.GSL',
    method: 'cmake',
    modules: ['Microsoft.GSL::GSL'],
    fallback: ['microsoft-gsl', 'microsoft_gsl_dep'],
)
thomas_dep = dependency('thomas')
grid_dep = dependency('grid')

program_options_dep = dependency(
    'boost',
    modules: ['program_options'],
    version: '>=1.92.0',
)

deps = [gsl_dep, program_options_dep, thomas_dep, grid_dep]
inc_dir = include_directories('include')

part_a = executable(
    'part-a',
    ['src/part-a.cpp'],
    include_directories: inc_dir,
    dependencies: deps,
)

part_c = executable(
    'part-c',
    ['src/part-c.cpp'],
    include_directories: inc_dir,
    dependencies: deps,
)

