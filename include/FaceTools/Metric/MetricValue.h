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

#ifndef FACE_TOOLS_METRIC_METRIC_VALUE_H
#define FACE_TOOLS_METRIC_METRIC_VALUE_H

/*
 * A container for N-dimensional metrics with associated z-scores.
 */

#ifdef _WIN32
#pragma warning( disable : 4251)
#endif

#include <FaceTools/FaceTypes.h>
#include <boost/property_tree/ptree.hpp>
using PTree = boost::property_tree::ptree;

namespace FaceTools { namespace Metric {

class GrowthData;

class FaceTools_EXPORT MetricValue
{
public:
    MetricValue();
    MetricValue( int id, const FM*);
    MetricValue( int id, const FM*, const std::vector<float>& dvals, bool planar);
    MetricValue( const MetricValue&) = default;
    MetricValue& operator=( const MetricValue&) = default;

    int id() const { return _id;}
    size_t ndims() const { return _values.size();}

    // Overwrite with new values stating whether the measurement was
    // taken as an in-plane projection (true) or was taken out of plane.
    void setValues( const std::vector<float>& dvals, bool planar);

    // Return the raw vector of values.
    const std::vector<float>& values() const { return _values;}

    // Returns true iff the measurement was taken projected in plane.
    bool planar() const { return _planar;}

    // Return the raw value at dimension i.
    float value( size_t i=0) const { return _values.at(i);}

    // Returns the mean at the given age for dimension i
    // against the currently set growth data for the metric.
    float mean( float age, size_t i=0) const;

    // Returns the z-score at the given age for dimension i
    // against the currently set growth data for the metric.
    float zscore( float age, size_t i=0) const;

    void write( PTree&, float age) const;

private:
    int _id;
    const FM *_fm;
    std::vector<float> _values;
    bool _planar;
};  // end class

bool operator==( const MetricValue&, const MetricValue&);
bool operator!=( const MetricValue&, const MetricValue&);

}}   // end namespace

#endif
