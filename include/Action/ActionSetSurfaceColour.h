/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef FACE_TOOLS_ACTION_SET_SURFACE_COLOUR_H
#define FACE_TOOLS_ACTION_SET_SURFACE_COLOUR_H

#include "FaceAction.h"

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionSetSurfaceColour : public FaceAction
{ Q_OBJECT
public:
    // Display name will be used for spin box widget's tool tip.
    ActionSetSurfaceColour( const QString& dname, QWidget* parent=nullptr);

private slots:
    void tellReady( Vis::FV*, bool) override;
    bool doBeforeAction( FVS&, const QPoint&) override;
    bool doAction( FVS&, const QPoint&) override;
    void doAfterAction( EventSet& evs, const FVS&, bool) override { evs.insert(VIEW_CHANGE);}

private:
    QWidget *_parent;
    QColor _curColour;
    void setIconColour( const QColor&);
};  // end class

}   // end namespace
}   // end namespace

#endif
