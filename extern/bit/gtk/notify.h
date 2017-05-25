/*
 * This file Copyright (C) 2008-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: notify.h 14724 2016-03-29 16:37:21Z jordan $
 */

#pragma once

#include "tr-core.h"

void gtr_notify_init (void);

void gtr_notify_torrent_added   (const char * name);

void gtr_notify_torrent_completed (TrCore * core, int torrent_id);

