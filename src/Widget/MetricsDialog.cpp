/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
 *
 * Cliniface is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cliniface is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#include <MetricsDialog.h>
#include <CheckAllTableHeader.h>
#include <ui_MetricsDialog.h>
#include <IntTableWidgetItem.h>
#include <MetricCalculatorManager.h>
#include <SyndromeManager.h>
#include <Ethnicities.h>
#include <ModelSelector.h>
#include <PhenotypeManager.h>
#include <FaceModelManager.h>
#include <FaceModel.h>
#include <FileIO.h>
#include <algorithm>
#include <QHeaderView>
#include <QDoubleSpinBox>
#include <QToolTip>
#include <QComboBox>
using FaceTools::Widget::IntTableWidgetItem;
using FaceTools::Widget::MetricsDialog;
using FaceTools::Widget::ChartDialog;
using FaceTools::Widget::PhenotypesDialog;
using HPOMan = FaceTools::Metric::PhenotypeManager;
using SynMan = FaceTools::Metric::SyndromeManager;
using MCM = FaceTools::Metric::MetricCalculatorManager;
using MC = FaceTools::Metric::MetricCalculator;
using MS = FaceTools::Action::ModelSelector;
using FaceTools::Vis::FV;
using FaceTools::FM;

namespace {
enum ColIndex
{
    SHOW_COL = 0,
    IDNT_COL = 1,
    NAME_COL = 2,
    CATG_COL = 3,
    DESC_COL = 4
};  // end enum
}   // end namespace


MetricsDialog::MetricsDialog( QWidget *parent) :
    QDialog(parent), _ui(new Ui::MetricsDialog), _syndromeToPhenotype(false),
    _pdialog( new PhenotypesDialog( parent)),
    _cdialog( new ChartDialog( parent))
{
    _ui->setupUi(this);
    setWindowTitle( parent->windowTitle() + " | Metrics");

    //setWindowFlags( windowFlags() & ~Qt::WindowCloseButtonHint);
    //setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    //setParent( parent, windowFlags() & ~Qt::WindowStaysOnTopHint);

    _ui->table->setColumnCount(5);
    _ui->table->setHorizontalHeaderLabels( QStringList( {"", "ID", "Name", "Category", "Description"}));

    CheckAllTableHeader* header = new CheckAllTableHeader( _ui->table, true/* use eye icon */);
    _ui->table->setHorizontalHeader(header);

    connect( header, &CheckAllTableHeader::allChecked, this, &MetricsDialog::_doOnSetAllChecked);
    connect( header, &CheckAllTableHeader::sectionClicked, this, &MetricsDialog::_sortOnColumn);

    //_ui->table->setSelectionBehavior( QAbstractItemView::SelectRows);
    //_ui->table->setSelectionMode( QAbstractItemView::SingleSelection);
    //_ui->table->setSelectionMode( QAbstractItemView::NoSelection);
    _ui->table->setShowGrid(false);
    _ui->table->setFocusPolicy( Qt::StrongFocus);
    _ui->table->setEditTriggers( QAbstractItemView::NoEditTriggers);
    _ui->table->setSortingEnabled(true);
    header->setSortIndicatorShown(false);

    // Leveraging signal activated since this is only emitted as a result of user interaction.
    connect( _ui->syndromesComboBox, QOverload<int>::of(&QComboBox::activated), [this](){ _doOnUserSelectedSyndrome(); refresh();});
    connect( _ui->hpoComboBox, QOverload<int>::of(&QComboBox::activated), [this](){ _doOnUserSelectedHPOTerm(); refresh();});
    connect( _ui->flipButton, &QToolButton::clicked, [this](){ _doOnClickedFlipCombosButton(); refresh();});
    connect( _ui->matchedCheckBox, &QCheckBox::clicked, [this](){ _doOnClickedMatchButton(); refresh();});
    connect( _ui->matchSubjectDemographicsCheckBox, &QCheckBox::clicked, this, &MetricsDialog::_doOnClickedUseSubjectDemographicsButton);

    //_ui->table->setColumnHidden(IDNT_COL, true);
    header->setStretchLastSection(true);   // Resize width of final column
    header->setMinimumSectionSize(14);

    _ui->table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _populate();
    _ui->table->setColumnWidth( SHOW_COL, 18);

    connect( _ui->table, &QTableWidget::cellClicked, [this]( int rw){ _setSelectedRow(rw); refresh();});
    connect( _ui->table, &QTableWidget::currentItemChanged, [this]( QTableWidgetItem* item){ _setSelectedRow(item->row()); refresh();});
    //connect( _ui->table, &QTableWidget::itemChanged, this, &MetricsDialog::doOnItemChanged);

    connect( _ui->showChartButton, &QToolButton::clicked, this, &MetricsDialog::_doOnShowChart);
    connect( _ui->showPhenotypesButton, &QToolButton::clicked, this, &MetricsDialog::_doOnShowPhenotypes);

    connect( _cdialog, &ChartDialog::onGrowthDataChanged, [this](){ _refreshPhenotypeList(); emit onSetMetricGrowthData();});

    refresh();
}   // end ctor


void MetricsDialog::_doOnShowChart()
{
    _cdialog->show();
    _cdialog->raise();
    _cdialog->activateWindow();
}   // _end doOnShowChart


void MetricsDialog::_doOnShowPhenotypes()
{
    _pdialog->show();
    _pdialog->raise();
    _pdialog->activateWindow();
}   // end _doOnShowPhenotypes


MetricsDialog::~MetricsDialog() { delete _ui;}


void MetricsDialog::closeEvent( QCloseEvent* e)
{
    e->accept();
    accept();
}   // end closeEvent


void MetricsDialog::setShowScanInfoAction( QAction* a) { _ui->showScanInfoButton->setDefaultAction(a);}


void MetricsDialog::_doOnSetAllChecked( bool c)
{
    const Qt::CheckState cstate = c ? Qt::Checked : Qt::Unchecked;

    disconnect( _ui->table, &QTableWidget::itemChanged, this, &MetricsDialog::_doOnItemChanged);

    const int nrows = _ui->table->rowCount();
    for ( int i = 0; i < nrows; ++i)
    {
        if ( !_ui->table->isRowHidden(i))   // Only affects visible rows
        {
            _ui->table->item( i, SHOW_COL)->setCheckState( cstate);
            const int mid = _ui->table->item( i, IDNT_COL)->text().toInt();
            MCM::metric(mid)->setVisible(c);
            emit onChangedMetricVisibility( mid, c);
        }   // end if
    }   // end for

    connect( _ui->table, &QTableWidget::itemChanged, this, &MetricsDialog::_doOnItemChanged);
}   // end _doOnSetAllChecked


void MetricsDialog::_sortOnColumn( int cidx)
{
    _ui->table->sortItems(cidx);    // Sort into ascending order
    _resetIdRowMap();
}   // end sortOnColumn


void MetricsDialog::_resetIdRowMap()
{
    _idRows.clear();    // Re-map metric IDs to row indices
    const int nrows = _ui->table->rowCount();
    for ( int i = 0; i < nrows; ++i)
    {
        int mid = _ui->table->item( i, IDNT_COL)->text().toInt();
        _idRows[mid] = i;
    }   // end for
}   // end _resetIdRowMap


void MetricsDialog::highlightRow( int mid)
{
    assert( _idRows.count(mid) > 0);
    const int rowid = _idRows.at(mid);
    disconnect( _ui->table, &QTableWidget::itemChanged, this, &MetricsDialog::_doOnItemChanged);

    MC::Ptr pam = MCM::previousMetric();
    if ( pam)
    {
        const int prowid = _idRows.at( pam->id());
        _ui->table->item( prowid, SHOW_COL)->setBackgroundColor( Qt::white);
        _ui->table->item( prowid, IDNT_COL)->setBackgroundColor( Qt::white);
        _ui->table->item( prowid, NAME_COL)->setBackgroundColor( Qt::white);
        _ui->table->item( prowid, CATG_COL)->setBackgroundColor( Qt::white);
        _ui->table->item( prowid, DESC_COL)->setBackgroundColor( Qt::white);
    }   // end if

    static const QColor bg(200,235,255);
    _ui->table->item( rowid, SHOW_COL)->setBackgroundColor( bg);
    _ui->table->item( rowid, IDNT_COL)->setBackgroundColor( bg);
    _ui->table->item( rowid, NAME_COL)->setBackgroundColor( bg);
    _ui->table->item( rowid, CATG_COL)->setBackgroundColor( bg);
    _ui->table->item( rowid, DESC_COL)->setBackgroundColor( bg);

    connect( _ui->table, &QTableWidget::itemChanged, this, &MetricsDialog::_doOnItemChanged);
}   // end highlightRow


void MetricsDialog::_setSelectedRow( int rowid)
{
    const int mid = _ui->table->item( rowid, IDNT_COL)->text().toInt();
    // Cause the metric in the corresponding row to become current which
    // will in turn cause MetricCalculator::selected(mid) to be emitted.
    MCM::setCurrentMetric( mid);
    emit onSelectedMetric( mid);
}   // end _setSelectedRow


void MetricsDialog::_doOnItemChanged( QTableWidgetItem* m)
{
    bool newVis = false;
    const int mid = _ui->table->item( m->row(), IDNT_COL)->text().toInt();
    switch ( m->column())
    {
        case SHOW_COL:
            newVis = m->checkState() == Qt::Checked && !_ui->table->isRowHidden( m->row());
            MCM::metric(mid)->setVisible( newVis);
            emit onChangedMetricVisibility( mid, newVis);
            break;
    }   // end switch
}   // end _doOnItemChanged


void MetricsDialog::_populate()
{
    _populateHPOs( HPOMan::ids());
    _populateSyndromes( SynMan::ids());

    // Place all the metrics into the table.
    _ui->table->clearContents();
    for ( int mid : MCM::ids())
        _appendRow( mid);

    _sortOnColumn( NAME_COL);
    _setSelectedRow( 0);
    _ui->table->resizeColumnsToContents();
    _ui->table->resizeRowsToContents();
    _ui->table->scrollToTop();

    CheckAllTableHeader* header = qobject_cast<CheckAllTableHeader*>(_ui->table->horizontalHeader());
    header->setAllChecked(true);

    _doOnClickedFlipCombosButton();
}   // end _populate


void MetricsDialog::_populateHPOs( const IntSet& hids)
{
    _ui->hpoComboBox->clear();
    _ui->hpoComboBox->addItem( "-- Any --", -1);

    std::unordered_map<QString, int> nids;
    QStringList slst;
    for ( int hid : hids)
    {
        QString nm = HPOMan::phenotype(hid)->name();
        slst.append(nm);
        nids[nm] = hid;
    }   // end for

    slst.sort();
    for ( const QString& nm : slst)
        _ui->hpoComboBox->addItem( nm, nids.at(nm));

    _ui->hpoComboBox->setEnabled( _ui->hpoComboBox->count() > 1);
    if ( _ui->hpoComboBox->count() > 0)
        _ui->hpoComboBox->setCurrentIndex(0);
}   // end _populateHPOs


void MetricsDialog::_populateSyndromes( const IntSet& sids)
{
    _ui->syndromesComboBox->clear();
    _ui->syndromesComboBox->addItem( "-- Any --", -1);

    std::unordered_map<QString, int> nids;
    QStringList slst;
    for ( int sid : sids)
    {
        QString nm = SynMan::syndrome(sid)->name();
        slst.append(nm);
        nids[nm] = sid;
    }   // end for

    slst.sort();
    for ( const QString& nm : slst)
        _ui->syndromesComboBox->addItem( nm, nids.at(nm));

    _ui->syndromesComboBox->setEnabled( _ui->syndromesComboBox->count() > 1);
    if ( _ui->syndromesComboBox->count() > 0)
        _ui->syndromesComboBox->setCurrentIndex(0);
}   // end _populateSyndromes


void MetricsDialog::_appendRow( int mid)
{
    MC::Ptr mc = MCM::metric( mid);
    assert(mc);

    const int rowid = _idRows[mid] = _ui->table->rowCount();    // Id of new row to be entered
    _ui->table->insertRow(rowid);

    QTableWidgetItem* iitem = new IntTableWidgetItem( mid, 5);
    iitem->setFlags( Qt::ItemIsEnabled);
    _ui->table->setItem( rowid, IDNT_COL, iitem);

    QTableWidgetItem* nitem = new QTableWidgetItem( mc->name());
    nitem->setFlags( Qt::ItemIsEnabled);// | Qt::ItemIsEditable);
    _ui->table->setItem( rowid, NAME_COL, nitem);

    QTableWidgetItem* citem = new QTableWidgetItem( mc->category());
    citem->setFlags( Qt::ItemIsEnabled);
    _ui->table->setItem( rowid, CATG_COL, citem);

    QTableWidgetItem* ditem = new QTableWidgetItem( mc->description());
    ditem->setFlags( Qt::ItemIsEnabled);// | Qt::ItemIsEditable);
    _ui->table->setItem( rowid, DESC_COL, ditem);

    QTableWidgetItem* vitem = new QTableWidgetItem;
    vitem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    vitem->setCheckState( mc->isVisible() ? Qt::Checked : Qt::Unchecked);
    _ui->table->setItem( rowid, SHOW_COL, vitem);
}   // end _appendRow


/*
void MetricsDialog::updateRow( int rowid)
{
    MC::Ptr mc = MCM::metric( _ui->table->item(rowid,IDNT_COL)->text().toInt());
    _ui->table->item( rowid, SHOW_COL)->setCheckState( mc->isVisible() ? Qt::Checked : Qt::Unchecked);
    _ui->table->item( rowid, NAME_COL)->setText( mc->name());
    _ui->table->item( rowid, CATG_COL)->setText( mc->category());
    _ui->table->item( rowid, DESC_COL)->setText( mc->description());
    _ui->table->resizeColumnsToContents();
}   // end updateRow
*/


// private slot
void MetricsDialog::_doOnUserSelectedHPOTerm()
{
    if ( _ui->hpoComboBox->count() == 2)
        _ui->hpoComboBox->setCurrentIndex(1);
    const int hid = _ui->hpoComboBox->currentData().toInt();
    const int sid = _ui->syndromesComboBox->currentData().toInt();

    IntSet smset;   // Fill with selected metrics if necessary
    const IntSet *mset = &smset;

    if ( hid >= 0)
        mset = &HPOMan::phenotype(hid)->metrics(); // Metrics just from the selected HPO term
    else if ( sid < 0)   // Any syndrome too, so use all metrics in the matched set of phenotypes
    {
        if ( !_ui->matchedCheckBox->isChecked())
            mset = &MCM::ids(); // All metrics - don't care about matching
        else
        {
            for ( int h : _mpids)
            {
                const IntSet& mset0 = HPOMan::phenotype(h)->metrics();
                smset.insert( mset0.begin(), mset0.end());
            }   // end for
        }   // end else
    }   // end else if
    else
    {
        for ( int h : SynMan::syndrome(sid)->hpos()) // Get the HPO ids related to the currently selected syndrome
        {
            if ( _mpids.count(h) > 0)   // Restrict to only those that match
            {
                const IntSet& mset0 = HPOMan::phenotype(h)->metrics();
                smset.insert( mset0.begin(), mset0.end());
            }   // end if
        }   // end for
    }   // end else

    // Show/hide the metric table rows according to if the name of the metric is in the set for the selected HPO term.
    int rowCount = _ui->table->rowCount();
    int smid = -1;
    const int pam = MCM::currentMetric()->id();

    for ( int i = 0; i < rowCount; ++i)
    {
        const int mid = _ui->table->item(i, IDNT_COL)->text().toInt();
        bool vis = false;

        if ( mset->count(mid) > 0)
        {
            _ui->table->showRow( i);
            vis = _ui->table->item(i, SHOW_COL)->checkState() == Qt::Checked;
            if ( smid < 0 || mid == pam)
                smid = mid;
        }   // end if
        else
            _ui->table->hideRow( i);

        MCM::metric(mid)->setVisible( vis);
        emit onChangedMetricVisibility( mid, vis);
    }   // end for

    if ( !_syndromeToPhenotype)
    { // Re-populate the syndromes combo box with only those related to the selected phenotype
        if ( hid >= 0)
            _populateSyndromes( SynMan::hpoSyndromes(hid));
        else
            _populateSyndromes( SynMan::ids());  // Repopulate with all syndromes
    }   // end if

    if ( smid >= 0)
        _setSelectedRow( _idRows.at(smid));
}   // end _doOnUserSelectedHPOTerm


void MetricsDialog::_doOnUserSelectedSyndrome()
{
    if ( _syndromeToPhenotype)
    {
        if ( _ui->syndromesComboBox->count() == 2)
            _ui->syndromesComboBox->setCurrentIndex(1); // Not 'any' if only two syndromes

        if ( _ui->syndromesComboBox->currentData().toInt() < 0) // Any syndrome
            _ui->hpoComboBox->setCurrentIndex( 0);  // So any phenotype
    }   // end if
}   // end _doOnUserSelectedSyndrome


// public
void MetricsDialog::show()
{
    QWidget::show();
    raise();
    activateWindow();
}   // end show


// public
void MetricsDialog::hide()
{
    QWidget::hide();
    _cdialog->hide();
    _pdialog->hide();
}   // end hide


void MetricsDialog::_refreshPhenotypeList()
{
    _ui->matchedCheckBox->setEnabled( MS::selectedModel() != nullptr);
    int csid = _ui->syndromesComboBox->currentData().toInt();
    int cpid = _ui->hpoComboBox->currentData().toInt();

    const IntSet* hset = nullptr;
    if ( _syndromeToPhenotype && csid >= 0)
        hset = &SynMan::syndrome(csid)->hpos();
    else
        hset = &HPOMan::ids();

    // Restrict the current set of phenotypes to only those found on the current model
    _mpids.clear(); // Store model matching HPO ids
    if ( _ui->matchedCheckBox->isChecked() && MS::isViewSelected())
    {
        FM* fm = MS::selectedModel();
        for ( int hid : *hset)
        {
            if ( HPOMan::phenotype(hid)->isPresent(fm))
                _mpids.insert(hid);
        }   // end for
    }   // end if
    else
        _mpids = *hset; // Straight copy

    if ( _mpids.count(cpid) == 0)
        cpid = -1;

    _populateHPOs( _mpids);

    int rowid = cpid >= 0 ? _ui->hpoComboBox->findData(cpid) : 0;
    _ui->hpoComboBox->setCurrentIndex( rowid);
    _doOnUserSelectedHPOTerm();

    if ( !_syndromeToPhenotype)
    {
        if ( cpid >= 0)
            _populateSyndromes( SynMan::hpoSyndromes(cpid));
        else
            _populateSyndromes( IntSet());   // Empty set
    }   // end if

    _pdialog->doOnShowPhenotypes( _mpids);
}   // end _refreshPhenotypeList


void MetricsDialog::refresh()
{
    _doOnClickedUseSubjectDemographicsButton();
}   // end refresh


void MetricsDialog::_doOnClickedMatchButton()
{
    if ( !_ui->matchedCheckBox->isChecked())
        _ui->hpoComboBox->setCurrentIndex(0);   // Any phenotype
}   // end doOnClickedMatchButton


void MetricsDialog::_doOnClickedUseSubjectDemographicsButton()
{
    // Refreshes chart data and also (possibly) updates the currently set growth
    // data statistics for the currently selected metric.
    const FM* fm = MS::selectedModel();
    const bool matchSubject = fm && _ui->matchSubjectDemographicsCheckBox->isChecked();
    if ( !matchSubject)
        fm = nullptr;

    // Set the set of available growth curve data for each metric to be either all available stats
    // if !matchSubject, or just the set that is most compatible with the currently selected model.
    for ( MC::Ptr mc : MCM::metrics())
        mc->setCompatibleSources( fm);
    emit onSetMetricGrowthData();

    // When metric growth data changes, update hpo terms and syndromes
    // shown because the show matched only checkbox might be checked.
    _refreshPhenotypeList();
    _cdialog->refreshMetricOrModel();
}   // end doOnClickedUseSubjectDemographicsButton


void MetricsDialog::_doOnClickedFlipCombosButton()
{
    _syndromeToPhenotype = !_syndromeToPhenotype;
    QString iconName = ":/icons/ABOVE";
    QString toolTipText = tr("Restrict the list of syndromes to the currently selected phenotype.");
    if ( _syndromeToPhenotype)
    {
        iconName= ":/icons/BELOW";
        toolTipText = tr("Restrict the list of phenotypes to the currently selected syndrome.");
    }   // end if
    _ui->flipButton->setIcon( QIcon(iconName));
    _ui->flipButton->setToolTip( toolTipText);

    if ( _syndromeToPhenotype)
    {
        QVariant v = _ui->syndromesComboBox->currentData();
        _populateSyndromes( SynMan::ids());
        _ui->syndromesComboBox->setCurrentIndex( _ui->syndromesComboBox->findData( v));
        _doOnUserSelectedSyndrome();
    }   // eif
    else
    {
        QVariant v = _ui->hpoComboBox->currentData();
        _populateHPOs( HPOMan::ids());
        _ui->hpoComboBox->setCurrentIndex( _ui->hpoComboBox->findData(v));
        _doOnUserSelectedHPOTerm();
    }   // end else

    // Show the tooltip on click.
    QToolTip::showText( _ui->flipButton->mapToGlobal(QPoint()), toolTipText);
}   // end doOnClickedFlipCombosButton