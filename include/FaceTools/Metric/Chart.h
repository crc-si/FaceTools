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

#ifndef FACE_TOOLS_METRIC_CHART_H
#define FACE_TOOLS_METRIC_CHART_H

#include "GrowthData.h"
#include <QtCharts/QChart>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT Chart : public QtCharts::QChart
{ Q_OBJECT
public:
    /**
     * Create and return a new chart showing the given growth curve data for dimension d.
     */
    using Ptr = std::shared_ptr<Chart>;
    static Ptr create( int mid, size_t d, const FM* fm=nullptr);

    Chart( int mid, size_t, const FM* fm=nullptr);

    /**
     * Set this chart to have the default rich text formatted title
     * with components taken from the static functions below.
     */
    void addTitle();

    QString makeRichTextTitleString() const;
    QString makeLatexTitleString( int footnotemark=0) const;

private:
    const int _mid;
    const size_t _dim;
    const GrowthData *_gdata;
    float _ymin, _ymax;
    int _xmin, _xmax;
    bool _ageOutOfBounds;
    void _addSeriesToChart();
    void _addDataPoints( const FM*);
};  // end class

}}   // end namespace

#endif
