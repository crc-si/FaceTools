/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_ACTION_CHANGE_COLOUR_MAPPING_RANGE_H
#define FACE_TOOLS_ACTION_CHANGE_COLOUR_MAPPING_RANGE_H

#include "FaceAction.h"
#include <QTools/RangeMinMax.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionChangeColourMappingRange : public FaceAction
{ Q_OBJECT
public:
    explicit ActionChangeColourMappingRange( const QString&);

    QWidget* widget() const override { return _widget;}

    QString toolTip() const override;

protected:
    void postInit() override;
    bool isAllowed( Event) override;
    bool update( Event) override;

private slots:
    void _doOnValueChanged( float, float);

private:
    QTools::RangeMinMax *_widget;
};  // end class

}}   // end namespaces

#endif
