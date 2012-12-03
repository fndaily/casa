//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
#include "HistogramMain.qo.h"
#include <guitools/Histogram/BinPlotWidget.qo.h>
#include <guitools/Histogram/ColorPreferences.qo.h>

#include <QMessageBox>

namespace casa {

HistogramMain::HistogramMain(QWidget *parent)
    : QMainWindow(parent), fileLoader(this),
      plotWidget( NULL ),
      logger(LogOrigin("CASA", "Histogram")){

	ui.setupUi(this);

	QLayout* mainLayout = ui.centralwidget->layout();
	if ( mainLayout == NULL ){
		mainLayout = new QHBoxLayout();
	}
	plotWidget = new BinPlotWidget( true,false,true,this );
	mainLayout->addWidget( plotWidget );
	ui.centralwidget->setLayout( mainLayout );

	//Set-up the initial colors
	preferencesColor = new ColorPreferences( this );
	colorsChanged();

	connect(ui.actionImageFile, SIGNAL(triggered()), this, SLOT(openFileLoader()));
	connect(ui.actionColor, SIGNAL(triggered()), this, SLOT(openColorPreferences()));
	connect( &fileLoader, SIGNAL(imageFileChanged()), this, SLOT(imageFileChanged()));
	connect( preferencesColor, SIGNAL(colorsChanged()), this, SLOT(colorsChanged()));
}

void HistogramMain::openFileLoader(){
	fileLoader.exec();
}

void HistogramMain::openColorPreferences(){
	preferencesColor->exec();
}

void HistogramMain::colorsChanged(){
	QColor histogramColor = preferencesColor->getHistogramColor();
	QColor fitCurveColor = preferencesColor->getFitCurveColor();
	QColor fitEstimateColor = preferencesColor->getFitEstimateColor();
	plotWidget->setHistogramColor( histogramColor );
	plotWidget->setFitCurveColor( fitCurveColor );
	plotWidget->setFitEstimateColor( fitEstimateColor );
}

void HistogramMain::imageFileChanged(){
	QString imageFile = fileLoader.getFilePath();
	ImageInterface<Float>* image = NULL;
	bool success = generateImage( imageFile, image );
	if ( success ){
		/*bool histogramSet =*/ plotWidget->setImage( image );
	}
	else {
		QString msg( "Please check that the file "+imageFile+" represents a valid image.");
		QMessageBox::warning( this, "Problem Loading Image", msg );
	}
}

bool HistogramMain::generateImage( const QString& imagePath,
		ImageInterface<Float>*& image ) {
	bool success = true;
	try {
		String filePath = imagePath.toStdString();
		logger << LogIO::NORMAL
				<< "\nLoading Image: "<<filePath<<"\n"
				<< LogIO::POST;
		image = new PagedImage<Float>(filePath);
	}
	catch (AipsError x) {
		String msg = x.getMesg();
		logger << LogIO::SEVERE << "Caught exception: " << msg<< LogIO::EXCEPTION;
		success = false;;
	}
	return success;
}


HistogramMain::~HistogramMain(){
}
}
