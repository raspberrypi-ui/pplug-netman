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
/* LXPanel plugin functions                                                   */
/*----------------------------------------------------------------------------*/

static GtkWidget *nm_constructor (LXPanel *panel, config_setting_t *settings)
{
    /* Allocate and initialize plugin context */
    NMApplet *nm = (NMApplet *) g_object_new (NM_TYPE_APPLET, NULL);

    /* Allocate top level widget and set into plugin widget pointer. */
    nm->panel = panel;
    nm->settings = settings;
    nm->plugin = gtk_button_new ();
    lxpanel_plugin_set_data (nm->plugin, nm, netman_destructor);

    netman_init (nm);

    return nm->plugin;
}

/* Handler for system config changed message from panel */
static void nm_configuration_changed (LXPanel *, GtkWidget *plugin)
{
    NMApplet *nm = lxpanel_plugin_get_data (plugin);
    netman_update_display (nm);
}

/* Handler for control message */
static gboolean nm_control (GtkWidget *plugin, const char *cmd)
{
    NMApplet *nm = lxpanel_plugin_get_data (plugin);
    return netman_control_msg (nm, cmd);
}

int module_lxpanel_gtk_version = 1;
char module_name[] = PLUGIN_NAME;

/* Plugin descriptor */
LXPanelPluginInit fm_module_init_lxpanel_gtk = {
    .name = PLUGIN_TITLE,
    .description = N_("Controller for Network Manager"),
    .new_instance = nm_constructor,
    .reconfigure = nm_configuration_changed,
    .control = nm_control,
    .gettext_package = GETTEXT_PACKAGE
};

/* End of file */
/*----------------------------------------------------------------------------*/
