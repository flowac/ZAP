/*
 * This file Copyright (C) 2009-2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: StatsDialog.h 14724 2016-03-29 16:37:21Z jordan $
 */

#pragma once

#include "BaseDialog.h"

#include "ui_StatsDialog.h"

class QTimer;

class Session;

class StatsDialog: public BaseDialog
{
    Q_OBJECT

  public:
    StatsDialog (Session&, QWidget * parent = nullptr);
    ~StatsDialog ();

    // QWidget
    virtual void setVisible (bool visible);

  private slots:
    void updateStats ();

  private:
    Session& mySession;

    Ui::StatsDialog ui;

    QTimer * myTimer;
};

