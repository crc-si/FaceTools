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

#ifndef FACE_TOOLS_FACE_MODEL_H
#define FACE_TOOLS_FACE_MODEL_H

#include "PathSet.h"
#include "LandmarkSet.h"
#include "FaceViewSet.h"
#include <ObjModelTools.h>   // RFeatures
#include <QReadWriteLock>

namespace FaceTools {

class FaceTools_EXPORT FaceModel
{
public:
    explicit FaceModel( RFeatures::ObjModelInfo::Ptr);
    FaceModel();

    // Use these read/write locks before accessing or writing to this FaceModel.
    void lockForWrite();
    void lockForRead() const;
    void unlock() const;    // Call after done with read or write locks.

    // Update with new data. Returns false iff a null object is passed in or the wrapped ObjModel is not a
    // valid 2D manifold. If parameter NULL, update with existing data (which is presumed to have changed).
    bool update( RFeatures::ObjModelInfo::Ptr=nullptr);

    // For making linear changes to the model that can be expressed using a matrix.
    // Transform the model, the orientation, and the landmarks using the given matrix.
    void transform( const cv::Matx44d&);

    // Use this function to access the model for making direct changes. After making
    // changes, call update to ensure that updates propagate through. If making changes
    // to the wrapped ObjModel, ensure that ObjModelInfo::reset is called before update.
    inline RFeatures::ObjModelInfo::Ptr info() const { return _minfo;}

    // Get the KD-tree - DO NOT MAKE CHANGES TO IT DIRECTLY!
    inline RFeatures::ObjModelKDTree::Ptr kdtree() const { return _kdtree;}

    // Returns boundary values for each model component as [xmin,xmax,ymin,ymax,zmin,zmax].
    inline const std::vector<cv::Vec6d>& bounds() const { return _cbounds;}

    // Returns the super boundary of this model i.e. the smallest boundary
    // in 3D that contains all of this model's components.
    inline const cv::Vec6d& superBounds() const { return _sbounds;}

    // Set/get orientation of the face.
    inline void setOrientation( const RFeatures::Orientation &o) { _orientation = o; setSaved(false);}
    inline const RFeatures::Orientation& orientation() const { return _orientation;}

    // Set/get "centre" of the face.
    inline void setCentre( const cv::Vec3f& c) { _centre = c; _centreSet = true; setSaved(false);}
    inline const cv::Vec3f& centre() const { return _centre;}
    inline bool centreSet() const { return _centreSet;}    // True iff setCentre has been called.

    inline LandmarkSet::Ptr landmarks() const { return _landmarks;}    // CALL setSaved(false) AFTER UPDATING!
    inline PathSet::Ptr paths() const { return _paths;}    // CALL setSaved(false) AFTER UPDATING!

    // Set/get description of data.
    inline void setDescription( const std::string& d) { _description = d; setSaved(false);}
    inline const std::string& description() const { return _description;}

    // Set/get source of data.
    inline void setSource( const std::string& s) { _source = s; setSaved(false);}
    inline const std::string& source() const { return _source;}

    // The views associated with this model.
    inline const FVS& fvs() const { return _fvs;}

    // Set/get if this model needs saving.
    inline bool isSaved() const { return _saved;}
    inline void setSaved( bool s=true) { _saved = s;}

    bool hasMetaData() const;

    // Convenience function to update renderers on all associated FaceViews.
    void updateRenderers() const;

    // Find and return the point on the surface closest to the given point (which may not be on the surface).
    cv::Vec3f findClosestSurfacePoint( const cv::Vec3f&) const;

    // Does this model's super bounds intersect with the other model's super bounds?
    bool supersIntersect( const FaceModel&) const;

    // Translate the given point to the surface of this model. First finds the
    // closest point on the surface using the internal kd-tree.
    double translateToSurface( cv::Vec3f&) const;

    void pokeTransform( vtkMatrix4x4*);

private:
    bool _saved;
    std::string _description;   // Long form description
    std::string _source;        // Data source info
    bool _centreSet;            // If face centre has been set.
    cv::Vec3f _centre;          // Face "centre"
    RFeatures::Orientation _orientation;
    FaceTools::LandmarkSet::Ptr _landmarks;
    FaceTools::PathSet::Ptr _paths;
    RFeatures::ObjModelInfo::Ptr _minfo;
    RFeatures::ObjModelKDTree::Ptr _kdtree;
    std::vector<cv::Vec6d> _cbounds;
    cv::Vec6d _sbounds;

    mutable QReadWriteLock _mutex;
    FVS _fvs;  // Associated FaceViews
    friend class Vis::FaceView;

    void fixTransform( vtkMatrix4x4*);
    void calculateBounds();
    FaceModel( const FaceModel&) = delete;
    void operator=( const FaceModel&) = delete;
};  // end class

}   // end namespace

#endif
