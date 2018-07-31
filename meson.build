
project('img2string', ['c'], default_options : ['buildtype=debugoptimized'])

src = [ 'img2string.c' ]

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)
caca_dep = dependency('caca')

executable(
  'img2string',
  src,
  dependencies: [caca_dep, m_dep],
  install: true,
)

