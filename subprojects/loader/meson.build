project(
  'loader',
  'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++23']
)

project_headers = [
  'include/sscan/bond_info.h',
  'include/sscan/bonds_loader.h',
  'include/sscan/price_loader.h',
]

project_source_files = [
  'src/dto.h',
  'src/dto.cpp',
  'src/price_calc.h',
  'src/bonds_loader.cpp',
  'src/price_loader.cpp',
]

project_dependencies = [
  dependency('config', fallback : ['config', 'config_dep']),
  dependency('jsoncpp_static', static: true),
  dependency('http', fallback : ['http', 'http_dep']),
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