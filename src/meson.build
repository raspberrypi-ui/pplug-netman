gtk = dependency('gtk+-3.0')
gtkmm = dependency('gtkmm-3.0', version: '>=3.24')
netman = dependency('libnm')
nma = dependency('libnma')
secret = dependency('libsecret-1')
notify = dependency('libnotify')

sources = files(
  'netman.c',
  'ap-menu-item.c',
  'applet.c',
  'applet-device-bt.c',
  'applet-device-wifi.c',
  'applet-vpn-request.c',
  'mb-menu-item.c',
  'applet-agent.c',
  'applet-device-ethernet.c',
  'applet-dialogs.c',
  'ethernet-dialog.c',
  'mobile-helpers.c',
  'utils.c',
  'eap-method.c',
  'nm-compat.c',
  'nm-shared-utils.c'
)

resource_data = files(
  '8021x.ui',
  'ce-page-dsl.ui',
  'gsm-unlock.ui',
  'info.ui'
)

gnome = import('gnome')

sources += gnome.compile_resources(
  'applet-resources',
  'applet.gresource.xml',
  dependencies: resource_data
)

lsources = sources

ldeps = [ gtk, netman, nma, secret, notify ]

lincdir = include_directories('/usr/include/lxpanel')

largs = [ '-DLXPLUG', '-DPACKAGE_DATA_DIR="' + lresource_dir + '"', '-D_GNU_SOURCE' ]

shared_module('netman', lsources,
        dependencies: ldeps,
        install: true,
        install_dir: get_option('libdir') / 'lxpanel/plugins',
        c_args : largs,
        include_directories : lincdir,
        name_prefix: ''
)

wsources = lsources + 'netman.cpp'

wdeps = [ gtkmm, netman, nma, secret, notify ]

wincdir = include_directories('/usr/include/wf-panel-pi')

wargs = [ '-DPLUGIN_NAME="netman"', '-DPACKAGE_DATA_DIR="' + wresource_dir + '"' ]

shared_module('libnetman', wsources,
        dependencies: wdeps,
        install: true,
        install_dir: get_option('libdir') / 'wf-panel-pi',
        c_args : wargs,
        cpp_args : wargs,
        include_directories : wincdir,
        name_prefix: ''
)

metadata = files()
install_data(metadata, install_dir: metadata_dir)
