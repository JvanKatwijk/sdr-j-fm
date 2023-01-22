#pragma once

#include <vector>

class ThemeChoser {
public:
	ThemeChoser();
	~ThemeChoser() = default;

	using TStringVec = std::vector<const char *>;

	int               get_style_sheet_size() const;
	const TStringVec & get_style_sheet_names() const;

	void              set_curr_style_sheet_idx(std::size_t ci);
	int               get_curr_style_sheet_idx() const;
	const char *      get_curr_style_sheet_string() const;




private:
	TStringVec vecStyleSheetCode;
	TStringVec vecStyleSheetName;
	int        currIdx = 0;
};

extern ThemeChoser sThemeChoser;
