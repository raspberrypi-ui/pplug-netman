/*============================================================================
Copyright (c) 2022-2025 Raspberry Pi Holdings Ltd.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
============================================================================*/

#include <locale.h>
#include <glib/gi18n.h>

#ifdef LXPLUG
#include "plugin.h"
#else
#include "lxutils.h"
#endif

#include "netman.h"

/*----------------------------------------------------------------------------*/
/* Typedefs and macros                                                        */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Global data                                                                */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Prototypes                                                                 */
/*----------------------------------------------------------------------------*/

static int wifi_country_set (void);
static void netman_button_clicked (GtkWidget *, NMApplet *nm);

/*----------------------------------------------------------------------------*/
/* Function definitions                                                       */
/*----------------------------------------------------------------------------*/

static int wifi_country_set (void)
{
    FILE *fp;

    // is this 5G-compatible hardware?
    fp = popen ("iw phy0 info | grep -q '\\*[ \\t]*5[0-9][0-9][0-9][ \\t]*MHz'", "r");
    if (pclose (fp)) return 1;

    // is the country set?
    fp = popen ("raspi-config nonint get_wifi_country 1", "r");
    if (pclose (fp)) return 0;

    return 1;
}

/*----------------------------------------------------------------------------*/
/* wf-panel plugin functions                                                  */
/*----------------------------------------------------------------------------*/

/* Handler for button click */
static void netman_button_clicked (GtkWidget *, NMApplet *nm)
{
    CHECK_LONGPRESS
    status_icon_activate_cb (nm);
}

/* Handler for system config changed message from panel */
void netman_update_display (NMApplet *nm)
{
    if (nm->active)
        status_icon_size_changed_cb (nm);
    else
        gtk_widget_hide (nm->plugin);
}

/* Handler for control message */
gboolean netman_control_msg (NMApplet *nm, const char *cmd)
{
    if (!g_strcmp0 (cmd, "menu"))
    {
        if (nm->menu && gtk_widget_get_visible (nm->menu)) gtk_widget_hide (nm->menu);
        else if (nm_client_get_nm_running (nm->nm_client)) status_icon_activate_cb (nm);
    }

    if (!g_strcmp0 (cmd, "cset"))
    {
        nm->country_set = wifi_country_set ();
    }
    return TRUE;
}

void netman_init (NMApplet *nm)
{
    setlocale (LC_ALL, "");
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

    /* Allocate icon as a child of top level */
    nm->status_icon = gtk_image_new ();
    gtk_container_add (GTK_CONTAINER (nm->plugin), nm->status_icon);

    /* Set up button */
    gtk_button_set_relief (GTK_BUTTON (nm->plugin), GTK_RELIEF_NONE);
#ifndef LXPLUG
    g_signal_connect (nm->plugin, "clicked", G_CALLBACK (netman_button_clicked), nm);

    /* Set up long press */
    nm->gesture = add_long_press (nm->plugin, NULL, NULL);
#endif

    /* Set up variables */
    nm->country_set = wifi_country_set ();

    if (system ("ps ax | grep NetworkManager | grep -qv grep"))
    {
        nm->active = FALSE;
        g_message ("netman: network manager service not running; plugin hidden");
    }
    else
    {
        nm->active = TRUE;
        applet_startup (nm);
    }

    /* Show the widget and return */
    gtk_widget_show_all (nm->plugin);
}

void netman_destructor (gpointer user_data)
{
    NMApplet *nm = (NMApplet *) user_data;

    applet_finalize (nm);

#ifndef LXPLUG
    if (nm->gesture) g_object_unref (nm->gesture);
    g_object_unref (nm);
#else
    g_free (nm);
#endif
}

/*----------------------------------------------------------------------------*/
/* LXPanel plugin functions                                                   */
/*----------------------------------------------------------------------------*/
#ifdef LXPLUG

static GtkWidget *nm_constructor (LXPanel *panel, config_setting_t *settings)
{
    /* Allocate and initialize plugin context */
    NMApplet *nm = (NMApplet *) g_object_new (NM_TYPE_APPLET, NULL);

    /* Allocate top level widget and set into plugin widget pointer. */
    nm->panel = panel;
    nm->settings = settings;
    nm->plugin = gtk_button_new ();
    lxpanel_plugin_set_data (nm->plugin, nm, netman_destructor);

    nm->icon_size = panel_get_safe_icon_size (nm->panel);

    netman_init (nm);

    return nm->plugin;
}

/* Handler for button press */
static gboolean nm_button_press_event (GtkWidget *plugin, GdkEventButton *event, LXPanel *)
{
    NMApplet *nm = lxpanel_plugin_get_data (plugin);
    if (event->button == 1)
    {
        netman_button_clicked (plugin, nm);
        return TRUE;
    }
    else return FALSE;
}

/* Handler for system config changed message from panel */
static void nm_configuration_changed (LXPanel *, GtkWidget *plugin)
{
    NMApplet *nm = lxpanel_plugin_get_data (plugin);
    nm->icon_size = panel_get_safe_icon_size (nm->panel);
    netman_update_display (nm);
}

/* Handler for control message */
static gboolean nm_control (GtkWidget *plugin, const char *cmd)
{
    NMApplet *nm = lxpanel_plugin_get_data (plugin);
    return netman_control_msg (nm, cmd);
}

FM_DEFINE_MODULE (lxpanel_gtk, netman)

/* Plugin descriptor */
LXPanelPluginInit fm_module_init_lxpanel_gtk = {
    .name = N_("Network Manager"),
    .description = N_("Controller for Network Manager"),
    .new_instance = nm_constructor,
    .reconfigure = nm_configuration_changed,
    .button_press_event = nm_button_press_event,
    .control = nm_control,
    .gettext_package = GETTEXT_PACKAGE
};
#endif

/* End of file */
/*----------------------------------------------------------------------------*/
