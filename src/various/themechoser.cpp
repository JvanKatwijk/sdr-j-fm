
#include	<QFile>
#include "themechoser.h"
#include <cassert>

#include <cstring>

	ThemeChoser::ThemeChoser () {
/* ------------------------------------------------- */
//	do not change this first entry (keep it also on first place)
	vecStyleSheetName. push_back ( "Default");
	vecStyleFileName.  push_back ( "");
/* ------------------------------------------------- */
	vecStyleSheetName. push_back ("Adaptic");
	vecStyleFileName.  push_back (":res/Adaptic.qss");
	vecStyleSheetName. push_back ("Combinear");
	vecStyleFileName.  push_back (":res/Combinear.qss");
	vecStyleSheetName. push_back ("Darkeum");
	vecStyleFileName.  push_back (":/res/Darkeum.qss");
	vecStyleSheetName. push_back ("Diffnes");
	vecStyleFileName.  push_back (":res/Diffnes.qss");
	vecStyleSheetName. push_back ("EasyCode");
	vecStyleFileName.  push_back (":res/EasyCode.qss");
}

int	ThemeChoser::get_idx_of_sheet_name (const QString &name) const {
	for (int idx = 0; idx < (int)vecStyleSheetName.size(); idx++ ) {
	   if (vecStyleSheetName [idx] == name)
	      return idx;
	}
	return 0; // choose "Default" when not found
}

const	ThemeChoser::TStringVec & ThemeChoser::get_style_sheet_names() const {
	return vecStyleSheetName;
}

const 	QString ThemeChoser::get_curr_style_sheet_string () const {
	QString fileName	= vecStyleFileName. at (currIdx);
	QString result;
	QFile file (fileName);
	if (file. open (QFile::ReadOnly |QFile::Text)) {
	   result = file. readAll ();
	   file. close ();
	}
	return result;
}

//int	ThemeChoser::get_style_sheet_size () const {
//	return vecStyleSheetCode. size ();
//}

int ThemeChoser::get_curr_style_sheet_idx () const {
	return currIdx;
}

void ThemeChoser::set_curr_style_sheet_idx (std::size_t ci) {
	assert(ci < vecStyleSheetName.size());
	currIdx = ci;
}

//ThemeChoser sThemeChoser;
