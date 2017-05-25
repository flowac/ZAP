/*
 * This file Copyright (C) 2009-2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: TorrentDelegateMin.h 14724 2016-03-29 16:37:21Z jordan $
 */

#pragma once

#include "TorrentDelegate.h"

class TorrentDelegateMin: public TorrentDelegate
{
    Q_OBJECT

  public:
    explicit TorrentDelegateMin (QObject * parent = nullptr): TorrentDelegate (parent) {}
    virtual ~TorrentDelegateMin () {}

  protected:
    // TorrentDelegate
    virtual QSize sizeHint (const QStyleOptionViewItem&, const Torrent&) const;
    virtual void drawTorrent (QPainter * painter, const QStyleOptionViewItem& option, const Torrent&) const;
};

