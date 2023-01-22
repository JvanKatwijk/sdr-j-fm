#include "themechoser.h"
#include <cassert>

ThemeChoser::ThemeChoser() {
	vecStyleSheetCode. push_back (""); // this will reset the style sheet to the default

	vecStyleSheetCode. push_back (
         #include "./stylesheets/Adaptic.qss"
	);

	vecStyleSheetCode. push_back (
         #include "./stylesheets/Combinear.qss"
	);

	vecStyleSheetName. push_back ("Default");
	vecStyleSheetName. push_back ("Adaptic");
	vecStyleSheetName. push_back ("Combinear");

	assert(vecStyleSheetCode.size() == vecStyleSheetName.size());
}

const ThemeChoser::TStringVec & ThemeChoser::get_style_sheet_names() const {
	return vecStyleSheetName;
}

const char * ThemeChoser::get_curr_style_sheet_string() const {
	return vecStyleSheetCode.at(currIdx);
}

int ThemeChoser::get_style_sheet_size() const {
	return vecStyleSheetCode.size();
}

int ThemeChoser::get_curr_style_sheet_idx() const {
	return currIdx;
}

void ThemeChoser::set_curr_style_sheet_idx(std::size_t ci) {
	assert(ci < vecStyleSheetCode.size());
	currIdx = ci;
}


ThemeChoser sThemeChoser;
