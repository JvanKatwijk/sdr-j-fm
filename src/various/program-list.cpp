#
/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the fm receiver
 *
 *    fm receiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    fm receiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with dab-pluto-fm; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	"program-list.h"
#include	"radio.h"
#include	<QFile>
#include	<QDataStream>
#include	<QMessageBox>
#include	<QHeaderView>
#include	<QDomDocument>
#include	<QTextStream>
#include	<QFile>

	programList::programList (RadioInterface *mr,
	                          const QString &saveName,
	                          QScrollArea	*myWidget) {
	this		-> myParent	= mr;
	this		-> saveName     = saveName;
        this		-> myWidget	= myWidget;
        myWidget	-> resize (240, 200);
        myWidget        -> setWidgetResizable (true);

        tableWidget     = new QTableWidget (0, 2);
        myWidget        -> setWidget(tableWidget);
        tableWidget     -> setHorizontalHeaderLabels (
                    QStringList () << tr ("Station") << tr ("Frequency"));

	tableWidget	-> verticalHeader ()	-> setVisible (false);
	connect (tableWidget, SIGNAL (cellClicked (int, int)),
                 this, SLOT (tableSelect (int, int)));
        connect (tableWidget, SIGNAL (cellDoubleClicked (int, int)),
                 this, SLOT (removeRow (int, int)));
        connect (this, SIGNAL (newFrequency (int)),
                 mr, SLOT (newFrequency (int)));
	connect (this, SIGNAL (insertRow (int)),
	         tableWidget, SLOT (insertRow (int)));
	connect (this, SIGNAL (removeRow (int)),
	         tableWidget, SLOT (removeRow (int)));
        loadTable ();
}

       programList::~programList () {
int16_t rows    = tableWidget -> rowCount ();
        for (int i = rows; i > 0; i --)
           removeRow (i);
}

void	programList::addRow (const QString &name, const QString &freq) {
int16_t row = tableWidget -> rowCount ();

	insertRow (row);

	QTableWidgetItem *item0	= new QTableWidgetItem;
	item0		-> setTextAlignment (Qt::AlignLeft | Qt::AlignVCenter);
	item0		-> setText (name);
	tableWidget	-> setItem (row, 0, item0);

	QTableWidgetItem *item1	= new QTableWidgetItem;
	item1		-> setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
	item1		-> setText (freq);
	tableWidget	-> setItem (row, 1, item1);
	tableWidget -> resizeColumnsToContents ();
}
//
void	programList::tableSelect (int row, int column) {
	(void)column;
	QTableWidgetItem *const item = tableWidget -> item (row, 1);
	int32_t freq = item -> text (). toInt ();
	newFrequency (Khz (freq));
}

void	programList::removeRow (int row, int column) {
	(void)column;

	QMessageBox::StandardButton resultButton =
	   QMessageBox::question (myParent, tr ("Delete entry?"),
	                                     tr ("Are you sure?\n"),
	                                     QMessageBox::No | QMessageBox::Yes,
	                                         QMessageBox::Yes);
	if (resultButton == QMessageBox::Yes) {
	   removeRow		(row);
	   tableWidget -> resizeColumnsToContents ();
	}
}

void	programList::saveTable () {

	QDomDocument theScanList;
	QDomElement root	= theScanList. createElement ("fmList");
	theScanList. appendChild (root);
	for (int i = 0; i <  tableWidget -> rowCount (); i ++) {
	   QString stationName	= tableWidget -> item (i, 0) -> text ();
	   QString frequency	= tableWidget -> item (i, 1) -> text ();
	   QDomElement element	= theScanList.
	                              createElement ("fm-transmitter");
	   element. setAttribute ("theName", stationName);
	   element. setAttribute ("theFrequency", frequency);
	   root. appendChild (element);
	}

	QFile file (saveName);
	if (!file. open (QIODevice::WriteOnly | QIODevice::Text))
	   return;
	QTextStream stream (&file);
	stream << theScanList. toString ();
	file.close();
}

void	programList::loadTable () {
QFile file (saveName);
int	rowCount = 0;
	if (!file. open (QIODevice::ReadOnly))
	   return;
	QDomDocument xmlBOM;
	xmlBOM. setContent (&file);
	file. close ();
	QDomElement root	= xmlBOM. documentElement ();
	for (QDomElement component = root. firstChildElement ("fm-transmitter");
	     !component. isNull ();
	     component = component. nextSiblingElement ("fm-transmitter")) {
	   QString stationName	= component. attribute ("theName");
	   QString frequency	= component. attribute ("theFrequency");
	   addRow (stationName, frequency);
	}
	tableWidget -> show ();
}
