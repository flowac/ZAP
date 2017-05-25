/*
 * This file Copyright (C) 2009-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: file-list.h 14724 2016-03-29 16:37:21Z jordan $
 */

#pragma once

#include <gtk/gtk.h>
#include "tr-core.h"

GtkWidget * gtr_file_list_new       (TrCore *, int torrent_id);
void        gtr_file_list_clear     (GtkWidget *);
void        gtr_file_list_set_torrent (GtkWidget *, int torrent_id);

