/*
 * This file Copyright (C) 2010-2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: TrackerModelFilter.h 14724 2016-03-29 16:37:21Z jordan $
 */

#pragma once

#include <QSortFilterProxyModel>

class TrackerModelFilter: public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    TrackerModelFilter (QObject * parent = nullptr);

    void setShowBackupTrackers (bool);
    bool showBackupTrackers () const { return myShowBackups; }

  protected:
    // QSortFilterProxyModel
    virtual bool filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const;

  private:
    bool myShowBackups;
};

