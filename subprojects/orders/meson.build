project(
  'orders',
  'cpp',
  version : '0.1',
  default_options : ['warning_level=3']
)

project_headers = [
  'include/orders_loader.h'
]

project_source_files = [
  'src/orders_loader.cpp'
]

project_dependencies = [
  
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
  link_with : project_target
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