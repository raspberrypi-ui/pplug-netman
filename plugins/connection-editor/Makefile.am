bin_PROGRAMS = coned

coned_CFLAGS = \
	-I$(top_srcdir) \
	-DPACKAGE_DATA_DIR=\""$(datadir)/coned"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DNMALOCALEDIR=\""$(datadir)/locale"\" \
	-DBINDIR=\""$(bindir)"\" \
	-DICONDIR=\""$(datadir)/coned/data/icons"\" \
	-DPACKAGE_LIB_DIR=\""$(libdir)"\" \
	-DPACKAGE_UI_DIR=\""$(datadir)/coned"\" \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-I../../shared \
	-I../utils \
	-I../wireless-security \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

coned_SOURCES = \
	ce-page.c \
	ce-polkit-button.c \
	ce-polkit.c \
	ce-utils.c \
	connection-helpers.c \
	ip4-routes-dialog.c \
	ip6-routes-dialog.c \
	main.c \
	nm-connection-editor.c \
	nm-connection-list.c \
	page-8021x-security.c \
	page-bluetooth.c \
	page-bond.c \
	page-bridge.c \
	page-bridge-port.c \
	page-dcb.c \
	page-dsl.c \
	page-ethernet.c \
	page-general.c \
	page-infiniband.c \
	page-ip4.c \
	page-ip6.c \
	page-ip-tunnel.c \
	page-macsec.c \
	page-master.c \
	page-mobile.c \
	page-ppp.c \
	page-proxy.c \
	page-team.c \
	page-team-port.c \
	page-vlan.c \
	page-vpn.c \
	page-wifi.c \
	page-wifi-security.c \
	page-wireguard.c \
	ppp-auth-methods-dialog.c \
	vpn-helpers.c \
	../utils/utils.c \
	../wireless-security/eap-method.c \
	../../shared/nm-utils/nm-compat.c \
	../../shared/nm-utils/nm-shared-utils.c \
	ce-resources.c

coned_LDFLAGS = \
	$(PACKAGE_LIBS) \
	-lm
	
GLIB_COMPILE_RESOURCES=glib-compile-resources				

ce-resources.c : ce.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies ce.gresource.xml)
	$(GLIB_COMPILE_RESOURCES) --target=$@ --generate-source $<





