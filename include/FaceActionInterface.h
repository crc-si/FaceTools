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

#ifndef FACE_TOOLS_FACE_ACTION_INTERFACE_H
#define FACE_TOOLS_FACE_ACTION_INTERFACE_H

#include "FaceTools_Export.h"
#include <PluginInterface.h>    // QTools
#include <QProgressUpdater.h>
#include <opencv2/opencv.hpp>
#include <QToolBar>
#include <QToolButton>
#include <QPushButton>
#include <QMenu>
#include <QAction>

namespace FaceTools
{

class FaceActionWorker;
class FaceControl;

// Must remain pure virtual so Qt can make it a plugin interface.
class FaceTools_EXPORT FaceActionInterface : public QTools::PluginInterface
{ Q_OBJECT
public:
    virtual ~FaceActionInterface(){}

    // NOTES FOR CONSTRUCTORS OF DERIVED CLASSES
    // Derived classes may want FaceAction as their parent instead of FaceActionInterface.
    // Class FaceAction creates an internal member QAction which is configured to use the
    // FaceAction's display name, icon, and shortcut key. Child classes should call
    // setEnabled(bool) as appropriate on the member QAction* in
    // the slots that are connected to the FaceControl signals.
    virtual QAction* qaction() = 0;

    // Add/remove interactors so action knows what to apply itself
    // to and/or what conditions to respond to (e.g. to enable/disable
    // self, or automatically trigger when certain conditions are met).
    virtual void addController( FaceControl*) = 0;
    virtual void removeController( FaceControl*) = 0;

    // Allow an already added interactor to inform this action of when it starts or stops
    // being interacted on by the user. One or more interactors of the full set of added
    // interactors may be set interactive. Actions may want to reimplement this function
    // to allow them to enable/disable themselves in response to these events.
    virtual void setControlled( FaceControl*, bool) = 0;
};  // end class


class FaceTools_EXPORT FaceAction : public FaceActionInterface
{ Q_OBJECT
public:
    FaceAction();

    // Triggering the action calls this action's process function.
    virtual QAction* qaction() { return &_action;}

    // Connect interactors to this action. Several interactors may be attached to an
    // action, or each newly connected interactor may replace the existing one (decided
    // by the implementing action). Whether or not an interactor is currently being acted
    // upon is communicated via setControlled. In this way, an action can be aware of a
    // larger set of interactors than those telling it that they are ready to be acted
    // upon. Some actions may not care about which interactors are currently available
    // to it, and only want to work on the interactive set (e.g. visualisation actions).
    // In this case, it is only necessary to reimplement setControlled.
    virtual void addController( FaceControl*){}
    virtual void removeController( FaceControl*){}
    virtual void setControlled( FaceControl*, bool){}

    // Set whether this action will be undertaken asynchronously or not
    // on the next call to process. Default is synchronous (blocking calls).
    // Optionally set a progress updater which must be updated by the derived
    // type's implementation of doAction().
    virtual void setAsync( bool, QTools::QProgressUpdater* pupdater=NULL);

    // Connect the provided buttons to this action
    void connectToolButton( QToolButton*);
    void connectButton( QPushButton*);

signals:
    // Notify of action completion passing boolean returned from doAction.
    void finished( bool);

public slots:
    // If synchronous, calls doAction and blocks, returning the
    // return value of doAction. If asynchronous, function returns
    // immediately and caller should listen for signal finished.
    // Whether or not asynchronous processing is enabled, signal
    // finished is always emitted.
    bool process();

protected:
    // Derived types must call init() from within their constructor to initialise
    // the FaceAction's member action. This can't be done from within the FaceAction
    // constructor itself due to the use of polymorphic calls which, if used from the base
    // class's constructor, will resolve to an incompletely instantiated object of the
    // derived type resulting in all sorts of weird non-instantiated data issues
    // (NULL pointers being returned for the icon and key sequence).
    void init();

    // Preparation immediately before doAction - which may occur in a separate thread.
    // doPrepAction always occurs in the GUI thread so this is where present file dialogs
    // etc in order to get user input.
    virtual void doPrepAction(){}

    // Implement the action; process() decides whether it runs asynchronously or not.
    // DON'T call GUI functions inside doAction if running asynchronously.
    virtual bool doAction(){ return true;}

    // If interested in providing progress updates for long running
    // actions that may be asynchronous, derived type should regularly
    // call this function to provide external progress updates.
    // This function first checks to see if the external progress
    // updater has been set (in setAsync) so is always safe to call.
    void progress( float propComplete);

    friend class FaceActionWorker;

protected slots:
    void setVisible( bool b) { _action.setVisible(b);}
    void setEnabled( bool b) { _action.setEnabled(b);}
    void setCheckable( bool b, bool ival) { _action.setCheckable(b); _action.setChecked(ival);}
    void setChecked( bool b) { _action.setChecked(b);}
    bool isChecked() const { return _action.isChecked();}
    bool isEnabled() const { return _action.isEnabled();}

    // Subclass may want to adjust action based on context. The action's
    // initial name comes from QTools::PluginInterface::getDisplayName.
    void setText( const QString& nm) { _action.setText(nm);}

    // The initial tool tip contains the action text.
    void setToolTip( const QString& ttip) { _action.setToolTip(ttip);}

private:
    QAction _action;
    bool _doasync;
    QTools::QProgressUpdater* _pupdater;
    FaceActionWorker* _fmaw;

    FaceAction( const FaceAction&);     // No copy
    void operator=( const FaceAction&); // No copy
};  // end class


class FaceTools_EXPORT FaceActionGroup : public FaceActionInterface
{ Q_OBJECT
public:
    FaceActionGroup();
    ~FaceActionGroup(); // Deletes all actions added by addAction

    // Create menus / toolbars for the action group.
    const QMenu* createMenu();
    const QToolBar* createToolBar();

    virtual void addTo( QMenu*) const;
    virtual void addTo( QToolBar*) const;
    virtual void addToContext( QMenu*) const{}  // Add selected actions to provided context menu

    // QTools::PluginInterface
    virtual QStringList getInterfaceIds() const;
    virtual FaceAction* getInterface( const QString&) const;

protected slots:
    // Groups are exclusive by default
    void setExclusive( bool b) { _group.setExclusive(b);}
    bool isExclusive() const { return _group.isExclusive();}

    // Add new FaceAction instances; the lifetimes of which will be managed
    // by this parent class. Returns true iff the action was successfully
    // added to the group. No actions with duplicate names allowed!
    bool addAction( FaceAction*);

private:
    QActionGroup _group;
    QMenu *_menu;
    QToolBar *_toolbar;
    boost::unordered_map<std::string, FaceAction*> _actions;
    QList<FaceAction*> _actionList;

    virtual QAction* qaction() { return NULL;}
    virtual void addController( FaceControl*){}
    virtual void removeController( FaceControl*){}
    virtual void setControlled( FaceControl*, bool){}
    FaceActionGroup( const FaceActionGroup&);   // No copy
    void operator=( const FaceActionGroup&);    // No copy
};  // end class


}   // end namespace


#define FaceToolsPluginFaceActionInterface_iid "com.github.richeytastic.FaceTools.v021.FaceActionInterface"
Q_DECLARE_INTERFACE( FaceTools::FaceActionInterface, FaceToolsPluginFaceActionInterface_iid)

#endif


