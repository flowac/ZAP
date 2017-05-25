/*
 * This file Copyright (C) 2007-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: tr-icon.h 14724 2016-03-29 16:37:21Z jordan $
 */

#pragma once

#include <gtk/gtk.h>
#include "tr-core.h"

gpointer  gtr_icon_new    (TrCore * core);
void      gtr_icon_refresh (gpointer);

