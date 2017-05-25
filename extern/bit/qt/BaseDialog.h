/*
 * This file Copyright (C) 2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: BaseDialog.h 14724 2016-03-29 16:37:21Z jordan $
 */

#pragma once

#include <QDialog>

class BaseDialog: public QDialog
{
  public:
    BaseDialog (QWidget * parent = nullptr, Qt::WindowFlags flags = 0):
      QDialog (parent, flags)
    {
      setWindowFlags (windowFlags () & ~Qt::WindowContextHelpButtonHint);
    }
};

