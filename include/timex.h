//--------------------------------------------------------------------------------
// Copyright 2007 (c) Quanta Sciences, Rama Hoetzlein, ramakarl.com
//
// * Derivative works may append the above copyright notice but should not remove or modify earlier notices.
//
// MIT License:
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
// associated documentation files (the "Software"), to deal in the Software without restriction, including without 
// limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS 
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
#ifndef DEF_TIMEX
  #define DEF_TIMEX

	#include <stdio.h>

	#include "common_defs.h"

	//----------------- PERFORMANCE MARKERS

	#if !defined(_WIN32)
	#	define __stdcall /* no __stdcall stuff on non-Windows platforms. */
	#endif

	#ifdef USE_NVTX
		typedef int ( __stdcall *nvtxRangePushFunc)(const char* msg);
		typedef int ( __stdcall *nvtxRangePopFunc)(void);
	#endif

	HELPAPI void PERF_PUSH ( const char* msg );
	HELPAPI float PERF_POP ();
	HELPAPI void  PERF_START ();
	HELPAPI float PERF_STOP ();
	HELPAPI void PERF_INIT ( int buildbits, bool cpu, bool gpu, bool cons, int lev, const char* fname );
	HELPAPI void PERF_SET(bool cpu, int lev, bool gpu, char* fname);
	HELPAPI void PERF_PRINTF ( char* format, ... );

	HELPAPI float strToDateF(std::string s, int mp = 0, int mc = 2, int dp = 3, int dc = 2, int yp = 6, int yc = 4);
	HELPAPI void strFromDateF(float f, int& m, int& d, int& y);


	// Time Class
	// Copyright Rama Hoetzlein (C) 2007
	// This code is released under BSD 3-Clause License. (https://opensource.org/licenses/BSD-3-Clause)
	//
	// Overview:
	//  There is a need in many systems to represent both very small (nanoseconds) and 
	//  very large (millenia) timescales accurately. Modified Julian Date accurate represents 
	//  individual days over +/- about 30,000 yrs. However, MJD represents fractions of a day
	//  as a floating point fraction. This is inaccurate for any timing-critical applications.	
	//  The Time class here uses an 8-byte (64 bit) integer called SJT, Scaled Julian Time.
	//      SJT = MJD * DAY_SCALAR + UT (nanoseconds).
	//  SJT is the Modified Julian Date scaled by a integer factor, and added to Universal Time
	//  represented in nanoseconds.
	//
	// Features:
	// - Accurately represents individual nanoseconds over +/- 30,000 yrs.
	// - Correct rollover of tiny time scales on month, day, year boundaries.
	//     e.g. Set date/time to 11:59:59.9999, on Feb 28th, 
	// - Accurately gives day of the week for any date
	// - Accurately compares two dates (days elapsed) even across leap-years.
	// - Adjust sec/nsec independently from month/day/year (work at scale you desire)	
	//
	// Implementation Notes:
	// JD = Julian Day is the number of days elapsed since Jan 1, 4713 BC in the proleptic Julian calendar.
	//     http://en.wikipedia.org/wiki/Julian_day
	// MJD = Modified Julian Date. Most modern dates, after 19th c., have Julian Date which are greater
	//       than 2400000.5. MJD is an offset. MJD = JD - 2400000.5
	//       It shifts the epoch date (start date) to Nov 17, 1858.
	// UT = Universal Time. This is the time of day in hours as measured from Greenwich England.
	//       For non-astronomic uses, this is: UT = Local Time + Time Zone.
	// SJT = Scaled Julian Time = MJD * DAY_SCALAR + UT (in nanoseconds).
	//
	// Julian Dates (and their MJD and SJT equivalents)
	// ------------
	// Jan 1,  4713 BC = JD 0			= MJD -2400000	= SJT 
	// Jan 1,  1500 AD = JD 2268933.5	= MJD -131067	= SJT 
	// Nov 16, 1858 AD = JD 2400000.5	= MJD 0			= SJT 0
	// Jan 1,  1960 AD = JD 2436935.5	= MJD 36935		= SJT 3,191,184,000,000
	// Jan 1,  2005 AD = JD 2453372.5	= MJD 53372		= SJT 4,611,340,800,000
	// Jan 1,  2100 AD = JD 2488070.5	= MJD 88070		= SJT 7,609,248,000,000
	//
	//
	// 
	// 32/64-Bit Integer Ranges
	//    32-bit Integer Min:              –2,147,483,648   ( 4 bytes )
	//    32-bit Integer Max:               2,147,483,647 
	//    SJT 2005:                     4,611,340,800,000
	//    64-bit Integer Min:  –9,223,372,036,854,775,808 
	//    64-bit Integer Max:   9,223,372,036,854,775,807	( 8 bytes )
	//
	// SJT Range
	// ---------
	//   * USING DAY_SCALAR = 86,400,000 (millisec accuracy)
	//   SJT Range = (+/-9,223,372,036,854,775,807 SJT / 86,400,000 DAY_SCALAR)
	//   SJT Range (in Julian Days) = +2400000.5 + (+/-106,751,991,167 MJD) 	
	//   SJT Range (in Julian Days) = +/- 292278883 years, with 1 millisecond accuracy.
	//
	//   * USING DAY_SCALAR = 86,400,000,000,000 (nanosec accuracy)	
	//   SJT Range = (+/-9,223,372,036,854,775,807 SJT / 86,400,000,000,000 DAY_SCALAR)	
	//   SJT Range (in Julian Days) = +2400000.5 + (+/-106,751 MJD)	
	//   SJT Range (in Julian Days) = 1566 AD to 2151 AD, with 1 nanosecond accuracy.

	#include <string>	
	#ifdef _MSC_VER
		#include <windows.h>
		typedef __int64		mstime;
		typedef __int64		sjtime;
	#endif
	#ifdef __linux__
		#include <linux/types.h>
		typedef __s64       mstime;
		typedef __s64       sjtime;
	#endif

	#ifdef __CYGWIN__	
		#include <largeint.h>	
		typedef __int64       mstime;
		typedef __int64       sjtime;
	#endif	

	#include <string>	
	#ifdef _MSC_VER
		#define MSEC_SCALAR		       1000000i64
		#define SEC_SCALAR        1000000000i64
		#define MIN_SCALAR       60000000000i64
		#define HR_SCALAR      3600000000000i64
		#define	DAY_SCALAR    86400000000000i64			
		#define F_YEAR_MULT   33177600i64
		#define F_MONTH_MULT  2764800i64
		#define F_DAY_MULT    86400i64
		#define F_HR_MULT		  3600i64
		#define F_MIN_MULT    60i64
		#define F_SEC_MULT    1i64
		#pragma warning ( disable : 4522 )
		#pragma warning ( disable : 4996 )		// sprintf warning
	#endif
	#ifdef __linux__
		#define MSEC_SCALAR		1000000LL
		#define SEC_SCALAR		1000000000LL
		#define MIN_SCALAR		60000000000LL
		#define HR_SCALAR		3600000000000LL
		#define	DAY_SCALAR		86400000000000LL
		#define F_YEAR_MULT		33177600LL
		#define F_MONTH_MULT	2764800LL
		#define F_DAY_MULT		86400LL
		#define F_HR_MULT		3600LL
		#define F_MIN_MULT		60LL
		#define F_SEC_MULT		1LL
	#endif
	#ifdef __CYGWIN__	
		#define MSEC_SCALAR		1000000LL
		#define SEC_SCALAR		1000000000LL
		#define MIN_SCALAR		60000000000LL
		#define HR_SCALAR		3600000000000LL
		#define	DAY_SCALAR		86400000000000LL
	#endif	

	#define ACC_SEC			0
	#define ACC_MSEC		1
	#define ACC_NSEC		2

	#define NSEC_SCALAR		1

	#define DATE_NULL		-10000.0f

	class HELPAPI TimeX {
	public:
		TimeX ();
		TimeX ( sjtime t )			{ m_CurrTime = t; }
		TimeX ( int sec, int msec )	{ m_CurrTime = 0; SetTime ( sec, msec ); }
		bool operator<(const TimeX& other) const { return m_CurrTime < other.m_CurrTime; }

		// Set time or date
		bool SetTime ( int sec );							// Set seconds
		bool SetTime ( int sec, int msec );		// Set seconds, msecs				
		bool SetTime ( TimeX& t )	{ m_CurrTime = t.GetSJT(); return true;} // Set to another Time object				
		bool SetTime ( std::string line );		// Set time from string (hr,min,sec)			
		void Clear();													// Clear date & time to 0 (empty).			
		void ClearTime ();										// Clear time to 00:00:00 while keeping date
		bool isEmpty(); 
		bool isSameDay(TimeX& op);								// Check if two TimeX datetimes have the same date (day).
			

		void SetSystemTime ();										// Set date/time to system clock		
		void SetTimeNSec ();

		void SleepNSec ( float ns );

		static sjtime GetSystemMSec ();
		static sjtime GetSystemNSec ();

		void SetSJT ( sjtime t )	{ m_CurrTime = t ;}		// Set Scaled Julian Time directly

		// Float storage
		// Set current date/time as a float
		float SetDateF ( std::string line,  int mp, int mc, int dp, int dc, int yp, int yc );
		float SetDateF ( int m, int d, int y );
		void GetDateF ( float f, int& m, int& d, int& y );
		
		// Get time		
		void GetTime (int& sec, int& msec, int& nsec );
		void GetTime (int& hr, int& min, int& m, int& d, int& y);				
		void GetTime (int& hr, int& min, int& m, int& d, int& y, int& s, int& ms, int& ns);
		double GetDays();
		double GetSec ();
		double GetMSec ();
			
		// Read & Write Date/Time: YYYY-MM-DD HH:MM:SS		
		bool SetDateTime (int yr, int mo, int day, int hr=0, int min=0, int sec=0, int msec=0, int nsec=0 );
		bool ReadDateTime ( std::string line );
		std::string WriteDateTime ();
		bool SetDate(int mo, int day, int yr);				// Set date while keeping time
		bool SetTime(int hr, int min, int sec);				// Set time while keeping date
			
			
		std::string GetReadableTime ();		
		std::string GetReadableTime ( int fmt );
		std::string GetReadableSJT ();
		std::string GetDayOfWeekName ();
		sjtime GetSJT ()			{ return m_CurrTime; } 			

		// Advance Time
		void Advance ( TimeX& t );
		void AdvanceMinutes ( float n);
		void AdvanceHours ( float n );
		void AdvanceDays ( float n );
		void AdvanceSec ( float n );
		void AdvanceMins ( float n);
		void AdvanceMSec ( float n );
		
		// Utility functions 
		// (these do the actual work, but should not be private as they may be useful to user)
		sjtime GetScaledJulianTime ( int hr, int min, int m, int d, int y );
		sjtime GetScaledJulianTime ( int hr, int min, int m, int d, int y, int s, int ms, int ns );
		void GetTime ( sjtime t, int& hr, int& min, int& m, int& d, int& y);
		void GetTime ( sjtime t, int& hr, int& min, int& m, int& d, int& y, int& s, int& ms, int& ns);
		
		// Get/Set Julian Date and Modified Julain Date
		void SetJD ( double jd );
		void SetMJD ( int jd );
		double GetJD ();
		int GetMJD ();

		// Time operators
		TimeX& operator= ( const TimeX& op );
		TimeX& operator= ( TimeX& op );
		bool operator< ( const TimeX& op );
		bool operator< ( TimeX& op );
		bool operator> ( const TimeX& op );
		bool operator> ( TimeX& op );		
		bool operator<= ( const TimeX& op );
		bool operator<= ( TimeX& op );
		bool operator>= ( const TimeX& op );
		bool operator>= ( TimeX& op );	
		bool operator== ( const TimeX& op );
		bool operator!= ( TimeX& op );
		TimeX operator- ( TimeX& op );
		TimeX operator+ ( TimeX& op );

		// Elapsed Times
		float GetElapsedSec ( TimeX& base );
		float GetElapsedMSec ( TimeX& base );
		float GetElapsedMin (TimeX& base);
		float GetElapsedDays ( TimeX& base );		
		float GetElapsedWeeks ( TimeX& base );
		float GetElapsedMonths ( TimeX& base );
		float GetElapsedYears ( TimeX& base );
		long GetFracDay ( TimeX& base );			// Return Unit = 5 mins
		long GetFracWeek ( TimeX& base );			// Return Unit = 1 hr
		long GetFracMonth ( TimeX& base );		// Return Unit = 4 hrs
		long GetFracYear ( TimeX& base );			// Return Unit = 1 day
		int GetDayOfWeek ();
		int GetWeekOfYear ();

		void RegressionTest ();

	private:		
		static const int	m_DaysInMonth[13];
		static bool			m_Started;

		sjtime				m_CurrTime;		
	};

	// Used for precise system time (Win32)
	void start_timing ( sjtime base );


#endif

