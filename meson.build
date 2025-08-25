project(
  'securities_scanner', 
  'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++14']
)

project_source_files = [
  'src/main.cpp'
]

project_dependencies = [
  dependency('orders', fallback : ['orders', 'orders_dep'])
]

executable(
  meson.project_name(),
  sources: project_source_files,
  dependencies: project_dependencies,
  install: true
)


