gtk = dependency('gtk+-3.0')
gtkmm = dependency('gtkmm-3.0', version: '>=3.24')
netman = dependency('libnm')
nma = dependency('libnma')
secret = dependency('libsecret-1')
notify = dependency('libnotify')

lsources = files(
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

lsources += gnome.compile_resources(
  'applet-resources',
  'applet.gresource.xml',
  dependencies: resource_data
)

ldeps = [ gtk, netman, nma, secret, notify ]

lincdir = include_directories('/usr/include/lxpanel')

largs = [ '-DLXPLUG', '-DPACKAGE_DATA_DIR="' + lresource_dir + '"', '-DGETTEXT_PACKAGE="lxplug_' + meson.project_name() + '"', '-DENABLE_GTK3', '-DG_DISABLE_ASSERT', '-DG_DISABLE_CHECKS', '-DG_DISABLE_CAST_CHECKS' ]

shared_module(meson.project_name(), lsources,
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

wargs = [ '-DPLUGIN_NAME="' + meson.project_name() + '"', '-DPACKAGE_DATA_DIR="' + wresource_dir + '"', '-DGETTEXT_PACKAGE="wfplug_' + meson.project_name() +'"', '-DENABLE_GTK3' ]

shared_module('lib' + meson.project_name(), wsources,
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
