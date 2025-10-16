project(
  'securities_scanner', 
  'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++23']
)

subproject('jsoncpp_static')

project_source_files = [
  'src/main.cpp'
]

project_dependencies = [
  dependency('config', fallback : ['config', 'config_dep']),
  dependency('loader', fallback : ['loader', 'loader_dep']),
  dependency('scanner', fallback : ['scanner', 'scanner_dep']),
  dependency('notifier', fallback : ['notifier', 'notifier_dep']),

  dependency('boost', modules: ['log', 'log_setup', 'thread', 'filesystem', 'program_options'], static: true)
]

executable(
  meson.project_name(),
  sources: project_source_files,
  dependencies: project_dependencies,
  install: true
)


