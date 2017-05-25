/*
 * This file Copyright (C) 2009-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: relocate.h 14724 2016-03-29 16:37:21Z jordan $
 */

#pragma once

#include <gtk/gtk.h>
#include "tr-core.h"

GtkWidget * gtr_relocate_dialog_new (GtkWindow * parent,
                                     TrCore    * core,
                                     GSList    * torrentIds);

