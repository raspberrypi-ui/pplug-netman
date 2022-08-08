## Process this file with automake to produce Makefile.in

SUBDIRS = connection-editor

## loadable plugins
if BUILD_PLUGIN_LOADER
DYNAMIC_PLUGINS = \
	$(PLUGINS_LIST)
endif

pkglib_LTLIBRARIES = $(DYNAMIC_PLUGINS)

EXTRA_LTLIBRARIES = \
	netman.la

pkglibdir = $(libdir)/lxpanel/plugins

# netman
netman_la_SOURCES = \
	netman/netman.c \
	ap-menu-item.c \
	applet.c \
	applet-device-bt.c \
	applet-device-wifi.c \
	applet-vpn-request.c \
	mb-menu-item.c \
	applet-agent.c \
	applet-device-ethernet.c \
	applet-dialogs.c \
	ethernet-dialog.c \
	mobile-helpers.c \
	utils/utils.c \
	wireless-security/eap-method.c \
	../shared/nm-utils/nm-compat.c \
	../shared/nm-utils/nm-shared-utils.c \
	netman-resources.c

netman_la_CFLAGS = \
	-I$(top_srcdir) \
	-DPACKAGE_DATA_DIR=\""$(datadir)/lxpanel"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DNMALOCALEDIR=\""$(datadir)/locale"\" \
	-DBINDIR=\""$(bindir)"\" \
	-DICONDIR=\""$(datadir)/lxpanel/data/icons"\" \
	-DHAVE_LIBNOTIFY_07 \
	-D_GNU_SOURCE=1 \
	-I/usr/include/lxpanel \
	-I../shared \
	-Iutils \
	-Iwireless-security \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS) \
	-Wall

netman_la_LDFLAGS = \
	$(PACKAGE_LIBS) \
	-module @LXPANEL_MODULE@

GLIB_COMPILE_RESOURCES=glib-compile-resources

netman-resources.c : applet.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies applet.gresource.xml)
	$(GLIB_COMPILE_RESOURCES) --target=$@ --generate-source $<

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/lxpanel/plugins/*.la
	rm -f $(DESTDIR)$(libdir)/lxpanel/plugins/*.a
	rm -f $(DESTDIR)$(libdir)/lxpanel/plugins/*.

if BUILD_PLUGIN_LOADER
DYNAMIC_PLUGINS_INSTALLED = $(DYNAMIC_PLUGINS:.la=.so)
uninstall-hook:
	cd $(DESTDIR)$(libdir)/lxpanel/plugins && rm -f $(DYNAMIC_PLUGINS_INSTALLED) || true
endif
