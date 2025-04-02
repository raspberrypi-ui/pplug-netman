
#include "nm-default.h"

#include <time.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "applet.h"
#include "applet-device-bt.h"
#include "applet-device-ethernet.h"
#include "applet-device-wifi.h"
#include "applet-dialogs.h"
#include "nma-wifi-dialog.h"
#include "applet-vpn-request.h"
#include "utils.h"

extern gboolean has_usable_wifi (NMApplet *applet);
extern void applet_connection_info_cb (NMApplet *applet);
extern void nma_edit_connections_cb (void);
extern void nma_set_wifi_enabled_cb (GtkWidget *widget, NMApplet *applet);
extern inline NMADeviceClass *get_device_class (NMDevice *device, NMApplet *applet);
extern char *get_tip_for_device_state (NMDevice *device, NMDeviceState state, NMConnection *connection);
extern void nma_menu_add_separator_item (GtkWidget *menu);


void activate_hotspot_cb (GObject *client, GAsyncResult *result, gpointer user_data)
{
	GError *error = NULL;
	NMActiveConnection *active;

	active = nm_client_activate_connection_finish (NM_CLIENT (client), result, &error);
	g_clear_object (&active);
	if (error)
	{
		const char *text = _("Failed to activate connection");
		char *err_text = g_strdup_printf ("(%d) %s", error->code, error->message ? error->message : _("Unknown error"));

		g_warning ("%s: %s", text, err_text);
		utils_show_error_dialog (_("Connection failure"), text, err_text, FALSE, NULL);
		g_free (err_text);
		g_error_free (error);
	}
	applet_schedule_update_icon (NM_APPLET (user_data));
}

void activate_hotspot (GtkMenuItem *item, gpointer user_data)
{
	NMApplet *applet = (NMApplet *) user_data;
	NMConnection *con;
	NMDevice *dev;
	const GPtrArray *all_devices;
	int i;

	// the connection path is stored as the name of the menu item widget - get the relevant connection
	con = NM_CONNECTION (nm_client_get_connection_by_path (applet->nm_client, gtk_widget_get_name (GTK_WIDGET (item))));
	if (!con) return;

	// now find a device which can use that connection
	all_devices = nm_client_get_devices (applet->nm_client);
	for (i = 0; all_devices && (i < all_devices->len); i++)
	{
		dev = all_devices->pdata[i];
		if (nm_device_connection_valid (dev, con)) break;
		dev = NULL;
	}
	if (!dev) return;

	// activate the connection with the found device - ap is NULL, as already in the connection
	nm_client_activate_connection_async (applet->nm_client, con, dev, NULL, NULL, activate_hotspot_cb, applet);
}

int add_hotspots (const GPtrArray *all_devices, const GPtrArray *all_connections, GtkWidget *menu, NMApplet *applet)
{
	const GPtrArray *act_conns;
	int i, n_devices = 0;
	GtkWidget *item, *hbox, *lbl, *icon, *sec;
	char *ssid_utf8;
	NMConnection *con;
	NMSettingWireless *s_wire;
	GBytes *ssid;
	gboolean dev_ok = FALSE;

	// is there a usable wifi device?
	for (i = 0; all_devices && (i < all_devices->len); i++)
	{
		NMDevice *device = all_devices->pdata[i];

		if (nm_device_get_device_type (device) == NM_DEVICE_TYPE_WIFI && !nma_menu_device_check_unusable (device))
			dev_ok = TRUE;
	}
	if (!dev_ok) return 0;

	// don't show hotspots if one is active
	act_conns = nm_client_get_active_connections (applet->nm_client);
	for (i = 0; act_conns && (i < act_conns->len); i++)
	{
		NMActiveConnection *ac = act_conns->pdata[i];
		con = NM_CONNECTION (nm_active_connection_get_connection (ac));
		s_wire = nm_connection_get_setting_wireless (con);
		if (!s_wire || !NM_IS_SETTING_WIRELESS (s_wire)) continue;
		if (!g_strcmp0 (nm_setting_wireless_get_mode (s_wire), "ap")) return 0;
	}

	// find any ap connections and add to menu
	for (i = 0; all_connections && (i < all_connections->len); i++)
	{
		con = NM_CONNECTION (all_connections->pdata[i]);
		s_wire = nm_connection_get_setting_wireless (con);
		if (!s_wire || !NM_IS_SETTING_WIRELESS (s_wire)) continue;
		if (g_strcmp0 (nm_setting_wireless_get_mode (s_wire), "ap")) continue;

		if (!n_devices) nma_menu_add_separator_item (menu);

		item = gtk_check_menu_item_new ();
		hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
		gtk_container_add (GTK_CONTAINER (item), hbox);

		ssid = nm_setting_wireless_get_ssid (s_wire);
		ssid_utf8 = nm_utils_ssid_to_utf8 (g_bytes_get_data (ssid, NULL), g_bytes_get_size (ssid));
		lbl = gtk_label_new (ssid_utf8);
		g_free (ssid_utf8);
		gtk_label_set_xalign (GTK_LABEL (lbl), 0.0);
		gtk_label_set_yalign (GTK_LABEL (lbl), 0.5);
		gtk_box_pack_start (GTK_BOX (hbox), lbl, TRUE, TRUE, 0);

		icon = gtk_image_new ();
		wrap_set_menu_icon (applet, icon, "network-wireless-hotspot");
		gtk_box_pack_end (GTK_BOX (hbox), icon, FALSE, TRUE, 0);

		sec = gtk_image_new ();
		NMSettingWirelessSecurity *s_sec = nm_connection_get_setting_wireless_security (con);
		if (s_sec) wrap_set_menu_icon (applet, sec, "network-wireless-encrypted");
		gtk_box_pack_end (GTK_BOX (hbox), sec, FALSE, TRUE, 0);

		g_signal_connect (item, "activate", G_CALLBACK (activate_hotspot), applet);
		gtk_widget_set_name (item, nm_connection_get_path (con));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), GTK_WIDGET (item));
		gtk_widget_show_all (GTK_WIDGET (item));

		n_devices++;
	}
	return n_devices;
}

void nma_menu_add_wifi_switch_item (GtkWidget *menu, NMApplet *applet)
{
	GtkWidget *menu_item = gtk_menu_item_new_with_mnemonic (nm_client_wireless_get_enabled (applet->nm_client) ? _("_Turn Off Wireless LAN") : _("_Turn On Wireless LAN"));
	gtk_widget_show_all (menu_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect (menu_item, "activate", G_CALLBACK (nma_set_wifi_enabled_cb), applet);
}

void set_country (GObject *o, gpointer data)
{
    if (fork () == 0)
    {
        char *args[] = { "rc_gui", "-w", NULL };
        execvp ("rc_gui", args);
    }
}

char *get_ip (NMDevice* device)
{
	// get the IP4 address
	NMIPConfig *ip4_config = nm_device_get_ip4_config (device);
	if (ip4_config)
	{
		const GPtrArray *addresses = nm_ip_config_get_addresses (ip4_config);
		if (addresses && addresses->len)
		{
			NMIPAddress *addr = (NMIPAddress *) g_ptr_array_index (addresses, 0);
			if (addr) return g_strdup_printf (_("IP : %s"), nm_ip_address_get_address (addr));
		}
	}
	else
	{
		// get the IP6 address if there is no IP4 address
		NMIPConfig *ip6_config = nm_device_get_ip6_config (device);
		if (ip6_config)
		{
			const GPtrArray *addresses = nm_ip_config_get_addresses (ip6_config);
			if (addresses && addresses->len)
			{
				NMIPAddress *addr = (NMIPAddress *) g_ptr_array_index (addresses, 0);
				if (addr) return g_strdup_printf (_("IP : %s"), nm_ip_address_get_address (addr));
			}
		}
	}
	return NULL;
}

char *get_tooltip (NMApplet *applet)
{
	char *out, *tmp, *ip, *ret = NULL;
	const char *icon_name;
	int i;

	g_return_val_if_fail (NM_IS_APPLET (applet), NULL);

	// loop through all current connnections
	const GPtrArray *connections = nm_client_get_active_connections (applet->nm_client);
	if (!connections || !connections->len) return NULL;

	for (i = 0; i < connections->len; i++)
	{
		NMActiveConnection *aconn = g_ptr_array_index (connections, i);

		// find the connection
		NMConnection *connection = (NMConnection *) nm_active_connection_get_connection (aconn);
		if (!connection) continue;

		// find the device
		const GPtrArray *devices = nm_active_connection_get_devices (aconn);
		if (!devices || !devices->len) continue;
		NMDevice *device = g_ptr_array_index (devices, 0);

		// find device state and class
		NMDeviceState state = nm_device_get_state (device);
		NMADeviceClass *dclass = get_device_class (device, applet);

		// filter out virtual devices
		if (!nm_device_get_hw_address (device)) continue;

		// filter out VPNs
		if (NM_IS_VPN_CONNECTION (aconn)) continue;

		// filter out loopback
		if (nm_device_get_device_type (device) == NM_DEVICE_TYPE_LOOPBACK) continue;

		// get the standard tooltip for the device, state and connection
		icon_name = NULL;
		out = NULL;
		if (dclass) dclass->get_icon (device, state, connection, NULL, &icon_name, &out, applet);

		// get the fallback tooltip if get_icon didn't supply one
		if (!out) out = get_tip_for_device_state (device, state, connection);

		if (out)
		{
			// append the new tooltip to any we already have
			if (!ret) ret = g_strdup_printf ("%s", out);
			else
			{
				tmp = g_strdup_printf ("%s\n%s", ret, out);
				g_free (ret);
				ret = tmp;
			}
			g_free (out);

			ip = get_ip (device);
			if (ip)
			{
				tmp = g_strdup_printf ("%s\n%s", ret, ip);
				g_free (ip);
				g_free (ret);
				ret = tmp;
			}
		}
	}
	return ret;
}

void applet_common_get_device_icon_lxp (gboolean wifi, NMDeviceState state,
	GdkPixbuf **out_pixbuf, char **out_icon_name, NMApplet *applet)
{
	char *name;
	int stage = -1;
	int lim;

	switch (state) {
	case NM_DEVICE_STATE_PREPARE:
	case NM_DEVICE_STATE_CONFIG:
	case NM_DEVICE_STATE_NEED_AUTH:
		stage = 0;
		break;
	case NM_DEVICE_STATE_IP_CONFIG:
		stage = 1;
		break;
	default:
		break;
	}

	if (stage >= 0)
	{
		if (stage == 0)
		{
			if (wifi)
			{
				lim = 4;
				if (applet->animation_step > lim) applet->animation_step = 0;
				switch (applet->animation_step)
				{
					case 0 : 	name = g_strdup_printf ("network-wireless-connected-00");
								break;
					case 1 : 	name = g_strdup_printf ("network-wireless-connected-25");
								break;
					case 2 : 	name = g_strdup_printf ("network-wireless-connected-50");
								break;
					case 3 : 	name = g_strdup_printf ("network-wireless-connected-75");
								break;
					case 4 : 	name = g_strdup_printf ("network-wireless-connected-100");
								break;
					default : 	name = g_strdup_printf ("network-wireless-connected-00");
								break;
				}
			}
			else
			{
				lim = 2;
				if (applet->animation_step > lim) applet->animation_step = 0;
				switch (applet->animation_step)
				{
					case 0 : 	name = g_strdup_printf ("network-transmit");
								break;
					case 1 : 	name = g_strdup_printf ("network-receive");
								break;
					case 2 : 	name = g_strdup_printf ("network-idle");
								break;
					default : 	name = g_strdup_printf ("network-transmit");
								break;
				}
			}
		}
		else
		{
			lim = 1;
			if (applet->animation_step > lim) applet->animation_step = 0;
			if (wifi)
			{
				switch (applet->animation_step)
				{
					case 0 : 	name = g_strdup_printf ("network-wireless-connected-00");
								break;
					case 1 : 	name = g_strdup_printf ("network-wireless-connected-100");
								break;
					default : 	name = g_strdup_printf ("network-wireless-connected-00");
								break;
				}
			}
			else
			{
				switch (applet->animation_step)
				{
					case 0 : 	name = g_strdup_printf ("network-transmit-receive");
								break;
					case 1 : 	name = g_strdup_printf ("network-idle");
								break;
					default : 	name = g_strdup_printf ("network-transmit-receive");
								break;
				}
			}
		}

		if (out_pixbuf)
			*out_pixbuf = nm_g_object_ref (nma_icon_check_and_load (name, applet));
		if (out_icon_name)
			*out_icon_name = name;
		else
			g_free (name);

		applet->animation_step++;
		if (applet->animation_step > lim)
			applet->animation_step = 0;
	}
}

