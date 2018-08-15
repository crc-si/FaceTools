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

#ifndef FACE_TOOLS_ACTION_LOAD_DIR_FACE_MODELS_H
#define FACE_TOOLS_ACTION_LOAD_DIR_FACE_MODELS_H

#include "FaceAction.h"
#include <LoadFaceModelsHelper.h>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionLoadDirFaceModels : public FaceAction
{ Q_OBJECT
public:
    ActionLoadDirFaceModels( const QString&, const QIcon&, FileIO::LoadFaceModelsHelper*);

protected slots:
    bool testEnabled( const QPoint* mc=nullptr) const override { return !_loadHelper->reachedLoadLimit();}
    bool doBeforeAction( FaceControlSet&, const QPoint&) override;
    bool doAction( FaceControlSet&, const QPoint&) override;
    void doAfterAction( ChangeEventSet&, const FaceControlSet&, bool) override;

private:
    FileIO::LoadFaceModelsHelper *_loadHelper;
};  // end class

}   // end namespace
}   // end namespace

#endif
