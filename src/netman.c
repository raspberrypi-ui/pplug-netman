/*============================================================================
Copyright (c) 2022-2025 Raspberry Pi
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

#include "plugin.h"

#include "netman.h"

/*----------------------------------------------------------------------------*/
/* Typedefs and macros                                                        */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Global data                                                                */
/*----------------------------------------------------------------------------*/

conf_table_t conf_table[1] = {
    {CONF_TYPE_NONE, NULL, NULL, NULL, NULL}
};

/*----------------------------------------------------------------------------*/
/* Prototypes                                                                 */
/*----------------------------------------------------------------------------*/

static int wifi_country_set (void);
static gboolean start_applet (gpointer data);
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

static gboolean start_applet (gpointer data)
{
    NMApplet *nm = (NMApplet *) data;

    nm->startup_id = 0;
    applet_startup (nm);

    return FALSE;
}

/*----------------------------------------------------------------------------*/
/* wf-panel plugin functions                                                  */
/*----------------------------------------------------------------------------*/

/* Handler for button click */
static void netman_button_clicked (GtkWidget *, NMApplet *nm)
{
    CHECK_LONGPRESS
    status_icon_activate_cb (NULL, nm);
}

/* Handler for system config changed message from panel */
void netman_update_display (NMApplet *nm)
{
    status_icon_size_changed_cb (NULL, wrap_icon_size (nm), nm);
}

/* Handler for control message */
gboolean netman_control_msg (NMApplet *nm, const char *cmd)
{
    if (!g_strcmp0 (cmd, "menu"))
    {
        if (nm->menu && gtk_widget_get_visible (nm->menu)) gtk_menu_popdown (GTK_MENU (nm->menu));
        else if (nm_client_get_nm_running (nm->nm_client)) status_icon_activate_cb (NULL, nm);
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
    wrap_set_taskbar_icon (nm, nm->status_icon, "network-idle");

    /* Set up button */
    gtk_button_set_relief (GTK_BUTTON (nm->plugin), GTK_RELIEF_NONE);
    g_signal_connect (nm->plugin, "clicked", G_CALLBACK (netman_button_clicked), nm);
    wrap_add_longpress (nm->gesture, nm->plugin, NULL, NULL);

    /* Set up variables */
    nm->icon_cache = NULL;
    nm->country_set = wifi_country_set ();
    nm->reloading = reload;

    /* Start the applet on idle */
    nm->startup_id = g_idle_add (start_applet, nm);

    /* Show the widget and return */
    gtk_widget_show_all (nm->plugin);
}

void netman_destructor (gpointer user_data)
{
    NMApplet *nm = (NMApplet *) user_data;

    if (nm->startup_id)
    {
        g_source_remove (nm->startup_id);
        nm->startup_id = 0;
    }

    wrap_free_gesture (nm->gesture);

    applet_finalize (nm);

    g_object_unref (nm);
}

/* End of file */
/*----------------------------------------------------------------------------*/
