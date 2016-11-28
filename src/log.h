/*
 * Extio for Airspy
 *
 * Copyright 2015 by Andrea Montefusco IW0HDV
 *
 * Licensed under GNU General Public License 3.0 or later. 
 * Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 */
 
 /** 
 * @file log.h
 * @brief Header for generic log functions and macros
 * @author Andrea Montefusco IW0HDV
 * @version 0.0
 * @date 2013-09-23
 */

#if !defined __LOG_H__
#define __LOG_H__


#define LOG_DLG_SIZE 32768


#if !defined NDEBUG || defined FLOG

#include "util.h"
#include "message_allocator.h"

struct LogImpl;

class Log: public MsgAllocator {
public:
	void open (const char *pszLogFileName, int n = 1);
	void log_printf(const char *format, ...);
	void log_printf_mod(const char *pszFile, int nLine);
	void log_funcname_printf(int f, const char *function, int line, const char *fmt, ...);
	void LogPostMsg(const char *pszText, bool timestamp);
	void close();
	Log();
	~Log();

private:
	void CreateLogDialog(const char *pszName);

	LogImpl *pi;
};

//
// http://stackoverflow.com/a/1008112
//
template <class X>
X& Singleton()
{
	static X x;

	return x;
}

//
// http://stackoverflow.com/questions/8130602/using-extern-template-c0x
//
#if !defined _DONT_DECLARE_TEMPLATE_
extern template Log &Singleton<Log>();
#endif

#define LOG_OPEN(file,n) Singleton<Log>().open(file, n) 
#define LOGT(fmt, ...)   Singleton<Log>().log_funcname_printf (1, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define LOGX(fmt, ...)   Singleton<Log>().log_funcname_printf (0, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define LOG_CLOSE	     Singleton<Log>().close()

#else

#define LOG_OPEN(file)
#define LOGT(fmt, ...)
#define LOGX(fmt, ...)
#define LOG_CLOSE

#endif

#endif // if !defined __LOG_H__

