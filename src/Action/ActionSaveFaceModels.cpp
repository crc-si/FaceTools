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

#include <ActionSaveFaceModels.h>
#include <FaceControl.h>
#include <QMessageBox>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionSaveFaceModels;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionSaveFaceModels::ActionSaveFaceModels( const QString& dn, const QIcon& ico, const QKeySequence& ks,
                                            FaceModelManager* fmm, QWidget *parent)
    : FaceAction( dn, ico, ks), _fmm(fmm), _parent(parent)
{
    setAsync(true);
}   // end ctor


bool ActionSaveFaceModels::testReady( const FaceControl* fc)
{
    FaceModel *fm = fc->data();
    return !fm->isSaved() && (_fmm->hasPreferredFileFormat(fm) || !fm->hasMetaData());
}   // end testReady


bool ActionSaveFaceModels::doAction( FaceControlSet& fset, const QPoint&)
{
    const FaceModelSet& fms = fset.models();
    for ( FaceModel* fm : fms)
    {
        std::string filepath;   // Will be the last saved filepath
        if ( !_fmm->write( fm, &filepath))  // Save using current filepath for the model
            _fails[_fmm->error()] << filepath.c_str();
    }   // end for
    return true;
}   // end doAction


void ActionSaveFaceModels::doAfterAction( ChangeEventSet&, const FaceControlSet&, bool)
{
    for ( auto f : _fails)  // Display a critical error for each type of error message received
    {
        QString msg( (f.first + "\nUnable to save the following:\n").c_str());
        msg.append( f.second.join("\n"));
        QMessageBox::critical( _parent, tr("Unable to save file(s)!"), tr(msg.toStdString().c_str()));
    }   // end for
    _fails.clear(); // Ensure the fail set is cleared
}   // end doAfterAction
