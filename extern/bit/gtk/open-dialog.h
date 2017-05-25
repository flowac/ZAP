/*
 * This file Copyright (C) 2010-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: open-dialog.h 14724 2016-03-29 16:37:21Z jordan $
 */

#pragma once

#include <gtk/gtk.h>
#include "tr-core.h"

GtkWidget* gtr_torrent_open_from_url_dialog_new (GtkWindow * parent, TrCore * core);
GtkWidget* gtr_torrent_open_from_file_dialog_new (GtkWindow * parent, TrCore * core);

/* This dialog assumes ownership of the ctor */
GtkWidget* gtr_torrent_options_dialog_new (GtkWindow * parent, TrCore * core, tr_ctor * ctor);

