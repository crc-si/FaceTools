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

#include <ActionRemesh.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <ObjModelVertexAdder.h>   // RFeatures
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionRemesh;
using FaceTools::Action::EventSet;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;


ActionRemesh::ActionRemesh( const QString& dn, const QIcon& ico, QProgressBar* pb)
    : FaceAction(dn, ico), _maxtarea(2.0)
{
    if ( pb)
        setAsync(true, QTools::QProgressUpdater::create(pb));
}   // end ctor


bool ActionRemesh::doAction( FVS& fvs, const QPoint&)
{
    assert(fvs.size() == 1);
    FV* fv = fvs.first();
    FM* fm = fv->data();

    fm->lockForWrite();

    bool success = true;
    RFeatures::ObjModelInfo::Ptr info = fm->info();
    RFeatures::ObjModel::Ptr model = info->model();

    RFeatures::ObjModelVertexAdder vadder( model);
    vadder.subdivideAndMerge( maxTriangleArea());
    //vadder.addVerticesToMaxTriangleArea( maxTriangleArea());
    if ( info->reset( model))
        fm->update(info);
    else
    {
        std::cerr << "[ERROR] FaceTools::Action::ActionRemesh::doAction: Unable to clean model post remesh!" << std::endl;
        success = false;
    }   // end else

    fm->unlock();
    return success;
}   // end doAction
