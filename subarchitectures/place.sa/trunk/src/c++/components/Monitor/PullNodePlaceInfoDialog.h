// ==================================================================
// Place.SA - Place Classification Subarchitecture
// Copyright (C) 2008, 2009  Andrzej Pronobis
//
// This file is part of Place.SA.
//
// Place.SA is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Place.SA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Place.SA. If not, see <http://www.gnu.org/licenses/>.
// ==================================================================

/**
 * PullNodePlaceInfoDialog class.
 * \file PullNodePlaceInfoDialog.h
 * \author Andrzej Pronobis
 * \date 2008-09-04
 */

#ifndef __PLACE_PULL_NODE_PLACE_INFO_DIALOG__
#define __PLACE_PULL_NODE_PLACE_INFO_DIALOG__

#include "ui_PullNodePlaceInfoDialog.h"
#include <PlaceData.hpp>
#include <QDialog>

namespace place
{


class PullNodePlaceInfoDialog : public QDialog, public Ui_PullNodePlaceInfoDialog
{
  Q_OBJECT

public:

  /** Constructor. */
  PullNodePlaceInfoDialog(QWidget *parent);


private:

  void showEvent(QShowEvent * event);


private:


};


}

#endif

