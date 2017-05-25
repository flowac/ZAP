/*
 * This file Copyright (C) 2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: DBusInteropHelper.h 14724 2016-03-29 16:37:21Z jordan $
 */

#pragma once

class QObject;
class QString;
class QVariant;

class DBusInteropHelper
{
  public:
    bool isConnected () const;

    QVariant addMetainfo (const QString& metainfo);

    static void registerObject (QObject * parent);
};

