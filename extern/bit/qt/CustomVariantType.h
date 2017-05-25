/*
 * This file Copyright (C) 2012-2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: CustomVariantType.h 14724 2016-03-29 16:37:21Z jordan $
 */

#pragma once

#include <QVariant>

class CustomVariantType
{
  public:
    enum
    {
      TrackerStatsList = QVariant::UserType,
      PeerList = QVariant::UserType,
      FileList,
      FilterModeType,
      SortModeType
    };
};

