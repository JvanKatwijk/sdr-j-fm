#pragma once

#include	<vector>
#include	<QString>

class ThemeChoser {
public:
		ThemeChoser ();
		~ThemeChoser () = default;

	using TStringVec = std::vector<QString>;

	int	get_style_sheet_size		() const;
const	TStringVec & get_style_sheet_names	() const;

	int	get_idx_of_sheet_name		(const QString &) const;

	void	set_curr_style_sheet_idx	(std::size_t ci);
	int	get_curr_style_sheet_idx	() const;
const	QString	get_curr_style_sheet_string	() const;

private:
	TStringVec vecStyleSheetCode;
	TStringVec vecStyleSheetName;
	int        currIdx = 0;
};

