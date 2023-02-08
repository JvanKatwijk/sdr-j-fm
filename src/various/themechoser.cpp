#include "themechoser.h"
#include <cassert>
#include <cstring>

	ThemeChoser::ThemeChoser () {
//	Add here further stylesheets, currently
//	there are sorted alphabetically (beside "Default")
/* ------------------------------------------------- */
//	do not change this first entry (keep it also on first place)
	vecStyleSheetName. push_back ( "Default");
	vecStyleSheetCode. push_back (""); // this will reset the style sheet to the default
/* ------------------------------------------------- */
	vecStyleSheetName. push_back ("Adaptic");
	vecStyleSheetCode. push_back (
         #include "./stylesheets/Adaptic.qss"
	);
/* ------------------------------------------------- */
//	defective?
//	vecStyleSheetName. push_back ("Chatbee");
//	vecStyleSheetCode. push_back (
//         #include "./stylesheets/Chatbee.qss"
//	);
/* ------------------------------------------------- */
	vecStyleSheetName. push_back ("Combinear");
	vecStyleSheetCode. push_back (
         #include "./stylesheets/Combinear.qss"
	);
/* ------------------------------------------------- */
//	defective?
//	vecStyleSheetName. push_back ("Darkeum");
//	vecStyleSheetCode. push_back (
//         #include "./stylesheets/Darkeum.qss"
//	);
/* ------------------------------------------------- */
//	defective?
//	vecStyleSheetName. push_back ("DeepBox");
//	vecStyleSheetCode. push_back (
//         #include "./stylesheets/DeepBox.qss"
//	);
/* ------------------------------------------------- */
// like Combinear
//	vecStyleSheetName. push_back ("Eclippy");
//	vecStyleSheetCode. push_back (
//         #include "./stylesheets/Eclippy.qss"
//	);
/* ------------------------------------------------- */
	vecStyleSheetName. push_back ("Fibers");
	vecStyleSheetCode. push_back (
         #include "./stylesheets/Fibers.qss"
	);
/* ------------------------------------------------- */
//	some elements needs adaptions
//	vecStyleSheetName. push_back ("Filmovio");
//	vecStyleSheetCode. push_back (
//         #include "./stylesheets/Filmovio.qss"
//	);
/* ------------------------------------------------- */
//	defective?
//	vecStyleSheetName. push_back ("Geoo");
//	vecStyleSheetCode. push_back (
//         #include "./stylesheets/Geoo.qss"
//	);
/* ------------------------------------------------- */
// defective?
//	vecStyleSheetName. push_back ("Gravia");
//	vecStyleSheetCode. push_back (
//         #include "./stylesheets/Gravira.qss"
//	);
/* ------------------------------------------------- */
//	defective?
//	vecStyleSheetName. push_back ("HackBook");
//	vecStyleSheetCode. push_back (
//         #include "./stylesheets/HackBook.qss"
//	);
/* ------------------------------------------------- */
//	unreadable, very dark
//	vecStyleSheetName. push_back ("Incrypt");
//	vecStyleSheetCode. push_back (
//         #include "./stylesheets/Incrypt.qss"
//	);
/* ------------------------------------------------- */
//	very bright, scope colors would need adaptions
//	vecStyleSheetName. push_back ("Integrid");
//	vecStyleSheetCode. push_back (
//         #include "./stylesheets/Integrid.qss"
//	);
/* ------------------------------------------------- */
//	some elements needs adaptions
//	vecStyleSheetName. push_back ("Perstfic");
//	vecStyleSheetCode. push_back (
//         #include "./stylesheets/Perstfic.qss"
//	);
/* ------------------------------------------------- */
//	defective?
//	vecStyleSheetName. push_back ("Photoxo");
//	vecStyleSheetCode. push_back (
//         #include "./stylesheets/Photoxo.qss"
//	);
/* ------------------------------------------------- */
//	defective?
//	vecStyleSheetName. push_back ("VisualScript");
//	vecStyleSheetCode. push_back (
//         #include "./stylesheets/VisualScript.qss"
//	);
/* ------------------------------------------------- */

	assert(vecStyleSheetCode.size() == vecStyleSheetName.size());
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

const 	QString ThemeChoser::get_curr_style_sheet_string() const {
	return vecStyleSheetCode.at(currIdx);
}

int	ThemeChoser::get_style_sheet_size() const {
	return vecStyleSheetCode.size();
}

int ThemeChoser::get_curr_style_sheet_idx() const {
	return currIdx;
}

void ThemeChoser::set_curr_style_sheet_idx(std::size_t ci) {
	assert(ci < vecStyleSheetCode.size());
	currIdx = ci;
}


//ThemeChoser sThemeChoser;
