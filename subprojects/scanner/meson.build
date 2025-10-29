project(
  'scanner',
  'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++23']
)

project_headers = [
  'include/sscan/scanner.h',
]

project_source_files = [
  'src/storage.h',
  'src/storage.cpp',
  'src/scanner.cpp',
]

project_dependencies = [
  dependency('config', fallback : ['config', 'config_dep']),
  dependency('loader', fallback : ['loader', 'loader_dep']),
  dependency('notifier', fallback : ['notifier', 'notifier_dep']),
]


public_headers = include_directories('include')


project_target = static_library(
  meson.project_name(),
  project_source_files,
  dependencies: project_dependencies,
  include_directories : public_headers,
)


# =======
# Project
# =======

# Make this library usable as a Meson subproject.
project_dep = declare_dependency(
  include_directories: public_headers,
  link_with : project_target,
  dependencies: project_dependencies
)
set_variable(meson.project_name() + '_dep', project_dep)

# Make this library usable from the system's
# package manager.
install_headers(project_headers, subdir : meson.project_name())

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name : meson.project_name(),
  filebase : meson.project_name(),
  description : '',
  subdirs : meson.project_name(),
  libraries : project_target,
)