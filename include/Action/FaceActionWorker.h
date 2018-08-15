/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

#ifndef FACE_TOOLS_ACTION_FACE_ACTION_WORKER_H
#define FACE_TOOLS_ACTION_FACE_ACTION_WORKER_H

#include <FaceTools_Export.h>
#include <opencv2/opencv.hpp>
#include <QThread>
#include <QPoint>

namespace FaceTools {
class FaceControlSet;

namespace Action {

class FaceAction;

class FaceTools_EXPORT FaceActionWorker : public QThread
{ Q_OBJECT
public:
    FaceActionWorker( FaceAction*, FaceControlSet*, const QPoint&);
    void run() override;

signals:
    void workerFinished(bool);

private:
    FaceAction* _worker;
    FaceControlSet* _rset;
    QPoint _tp;
};  // end class

}   // end namespace
}   // end namespace

#endif
