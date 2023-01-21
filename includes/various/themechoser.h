#pragma once

#include <vector>

class ThemeChoser {
public:
	//ThemeChoser() = default;
	//~ThemeChoser() = default;

	using TNameList = std::vector<const char *>;

	int				get_style_sheet_max_idx() const;
	TNameList		get_style_sheet_names() const;

	void				set_curr_style_sheet_idx(int ci);
	int				get_curr_style_sheet_idx() const;
	const char *	get_curr_style_sheet_string() const;




private:
	static const char *	styleSheet_1_Adaptic;
	static const char *	styleSheet_2_Combinear;
	int						currentIdx = 0;
};

extern ThemeChoser sThemeChoser;

