#include "themechoser.h"

const char * ThemeChoser::styleSheet_1_Adaptic =
      #include "./stylesheets/Adaptic.qss"
         ;
const char * ThemeChoser::styleSheet_2_Combinear =
      #include "./stylesheets/Combinear.qss"
         ;

//int ThemeChoser::currentIdx = 0;

std::vector<const char *> ThemeChoser::get_style_sheet_names() const {
	TNameList list;
	list.push_back("Default");
	list.push_back("Adaptic");
	list.push_back("Combinear");
	//return std::move(list);
	return list; // should be a move by the compiler
}

const char * ThemeChoser::get_curr_style_sheet_string() const {
	switch (currentIdx) {
	case 0: return ""; // this will reset the style sheet to the default
	case 1: return styleSheet_1_Adaptic;
	case 2: return styleSheet_2_Combinear;
	default: throw;
	}
}

int ThemeChoser::get_style_sheet_max_idx() const {
	return 2;
}

int ThemeChoser::get_curr_style_sheet_idx() const {
	return currentIdx;
}

void ThemeChoser::set_curr_style_sheet_idx(int ci) {
	if (ci > get_style_sheet_max_idx())
		throw;
	currentIdx = ci;
}


ThemeChoser sThemeChoser;
