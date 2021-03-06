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

#ifndef FACE_TOOLS_METRIC_DISTANCE_METRIC_TYPE_H
#define FACE_TOOLS_METRIC_DISTANCE_METRIC_TYPE_H

#include "MetricType.h"
#include <FaceTools/Vis/DistanceVisualiser.h>

namespace FaceTools { namespace Metric {

struct FaceTools_EXPORT DistMeasure
{
    Vec3f point0;
    Vec3f point1;
};  // end struct


class FaceTools_EXPORT DistanceMetricType : public MetricType
{
public:
    DistanceMetricType();

    QString category() const override { return "Distance";}
    QString units() const override { return FM::LENGTH_UNITS;}
    QString typeRemarks() const override { return "Distances are measured between points in 3D space in general but may instead be measured as a projection into one of the standard anatomical planes.";}
    Vis::MetricVisualiser* visualiser() override { return &_vis;}

    bool fixedInPlane() const override { return false;}

    bool hasMeasurement( const FM *fm) const override { return _distInfo.count(fm) > 0;}
    void purge( const FM *fm) override { _distInfo.erase(fm);}

    const std::vector<DistMeasure> &distInfo( const FM *fm) const { return _distInfo.at(fm);}

protected:
    float update( size_t, const FM*, const std::vector<Vec3f>&, Vec3f, Vec3f, bool, bool) override;

private:
    Vis::DistanceVisualiser _vis;
    std::unordered_map<const FM*, std::vector<DistMeasure> > _distInfo;
};  // end class

}}   // end namespaces

#endif
