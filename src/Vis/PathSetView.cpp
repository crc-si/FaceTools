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

#include <PathSetView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::PathSetView;
using FaceTools::Vis::PathView;
using FaceTools::ModelViewer;
using FaceTools::PathSet;
using FaceTools::Path;
using ViewPair = std::pair<int, PathView*>;


// public
PathSetView::PathSetView( const PathSet::Ptr paths) : _paths(paths), _viewer(nullptr)
{
    for ( int id : paths->ids())
        addPath(id);
}   // end ctor


// public
PathSetView::~PathSetView()
{
    std::for_each( std::begin(_views), std::end(_views), [](const ViewPair& p){ delete p.second;});
}   // end dtor


// public
bool PathSetView::isVisible() const { return !_visible.empty();}


// public
void PathSetView::setVisible( bool enable, ModelViewer *viewer)
{
    while ( isVisible())
        showPath( false, *_visible.begin());
    _viewer = viewer;
    if ( enable && _viewer)
        std::for_each( std::begin(_views), std::end(_views), [this](const ViewPair& p){ this->showPath(true, p.first);});
}   // end setVisible


// public
void PathSetView::showPath( bool enable, int id)
{
    assert( _views.count(id) > 0);
    _views.at(id)->setVisible( enable, _viewer);
    _visible.erase(id);
    if ( enable)
        _visible.insert(id);
}   // end showPath


// public
bool PathSetView::isPathVisible( int id) const { return _visible.count(id) > 0;}
const std::unordered_set<int>& PathSetView::visible() const { return _visible;}


// public
PathView::Handle* PathSetView::addPath( int id)
{
    if ( _views.count(id) > 0)
    {
        std::cerr << "[WARNING] FaceTools::Vis::PathSetView::addPath: Trying to add path but PathView already present!" << std::endl;
        return _views.at(id)->handle0();
    }   // end if

    const Path* path = _paths->path(id);
    assert(path);
    if ( !path)
    {
        std::cerr << "[ERROR] FaceTools::Vis::PathSetView::addPath: Path data doesn't exist for path ID " << id << std::endl;
        return nullptr;
    }   // end if

    PathView* pv = new PathView( id, path->vtxs);
    _views[id] = pv;
    // Map handle props to the handles themselves for fast lookup.
    _handles[pv->handle0()->prop()] = pv->handle0();
    _handles[pv->handle1()->prop()] = pv->handle1();
    return pv->handle0();
}   // end addPath


// public
PathView::Handle* PathSetView::handle( const vtkProp* prop) const
{
    PathView::Handle *h = nullptr;
    if ( _handles.count(prop) > 0)
        h = _handles.at(prop);
    return h;
}   // end handle


// public
PathView* PathSetView::pathView( int id) const
{
    PathView* pv = nullptr;
    if ( _views.count(id) > 0)
        pv = _views.at(id);
    return pv;
}   // end pathView


// public
void PathSetView::updatePath( int id)
{
    assert(id >= 0);
    if ( _paths->has(id))
    {
        if ( _views.count(id) == 0)
            addPath(id);
        else
            _views.at(id)->update( _paths->path(id)->vtxs);
        showPath( true, id);
    }   // end if
    else if (_views.count(id) > 0)
    {   // Delete the path if no longer in the dataset
        showPath(false, id);
        PathView* pv = _views.at(id);
        _handles.erase(pv->handle0()->prop());
        _handles.erase(pv->handle1()->prop());
        _views.erase(id);
        delete pv;
    }   // end else if
}   // end updatePath


// public
void PathSetView::pokeTransform( const vtkMatrix4x4* m)
{
    std::for_each( std::begin(_views), std::end(_views), [=](const ViewPair& p){ p.second->pokeTransform(m);});
}   // end pokeTransform


// public
void PathSetView::fixTransform()
{
    std::for_each( std::begin(_views), std::end(_views), [=](const ViewPair& p){ p.second->fixTransform();});
}   // end fixTransform


// public
void PathSetView::refresh()
{
    std::unordered_set<int> pids;   // Will be union of path IDs both currently visualised and not.
    std::for_each( std::begin(_views), std::end(_views), [&](const ViewPair& p){ pids.insert(p.first);});
    // Add the current path IDs in from the data
    std::for_each( std::begin(_paths->ids()), std::end(_paths->ids()), [&](int p){ pids.insert(p);});
    // Run updatePath for all pids
    std::for_each( std::begin(pids), std::end(pids), [this](int p){ this->updatePath(p);});
}   // end refresh
