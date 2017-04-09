//
//  Util.h
//  Jon Edwards Code Sample
//
//  Like Database utility declarations and functions 
// 
//  Created by Jon Edwards on 12/3/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#ifndef LDB_UTIL_H
#define LDB_UTIL_H

#include <stdio.h>
#include <assert.h>

#include <string>
#include <vector>
#include <limits>
#include <iostream>

#include "Types.h"

using namespace std;

//---------------------------------------------------------------------------
// Utility macros
//---------------------------------------------------------------------------

#define BEGIN_NAMESPACE(NAMESPACE) namespace NAMESPACE {
#define END_NAMESPACE(NAMESPACE) }

#define LogError(...) fprintf(stderr, __VA_ARGS__);
#define LogMessage(...) printf(__VA_ARGS__);

#ifndef ASSERT
#define ASSERT(CONDITION, MESSAGE) \
	{ \
		if (!(CONDITION)) \
		{ \
			fprintf(stderr, "--------------------- ASSERT ------------------------\n"); \
		 	fprintf(stderr, "ASSERT ERROR MESSAGE: %s\n", MESSAGE); \
			fprintf(stderr, "-----------------------------------------------------\n"); \
			assert(true); \
		} \
	}
#endif

BEGIN_NAMESPACE(LDB)
BEGIN_NAMESPACE(Util)

//----------------------------------------------------------------------------
// Util::TrimString : Remove leading and trailing white space from string
//----------------------------------------------------------------------------
inline void TrimString(string &str, const string &whitespace)
{
	// Trim leading white space
	size_t start = str.find_first_not_of(whitespace);
	if (start != string::npos)
	{
		str.substr(start).swap(str);
	}
	else
	{
		str.clear();	
		return;
	}

	// Trim trailling white space
	size_t end = str.find_last_not_of(whitespace);
	if (end != string::npos)
	{
		str.substr(0, end + 1).swap(str);
	}
}

//----------------------------------------------------------------------------
// Tokenize : Break a string into a list of tokens. Tokenizing will make
// use of the following:
//
//		delimiterChars (defaults to ",") - Delimiters that separate tokens.
//
//		delimiterTokenChars (defaults to "") : Chars from the set of
//		delimiters that should be preserved as tokens.
//
//		quoteChars ("defaults to "\"") : Characters used to detected
//		quoted strings. Should be matching. Quotes are included
//		in tokens.
//        
//		escapeChars (defaults to "\\") : A character that follows an escape
//		character is 
//		
// Example: 
// 			"Test string", 5,, 2, "Funky string with a , and a \""
//			--1st token-- -2nd-3rd--------------4th---------------  
//			"Test string"  5   2  "Funky string with a , and a ""
//----------------------------------------------------------------------------
inline void TokenizeString(const string str, vector<string> &tokens,
	const string &delimiterChars = " ,\t\n",
	const string &delimiterTokenChars = "",
	const string &quoteChars = "\"",
	const string &escapeChars = "\\")
{
	using namespace std;

	string token;
	bool escaped = false;
	char currentQuoteChar = 0;

	tokens.clear();
	int pos = 0;
	string::size_type len = str.length();
	while (pos < len)
	{
		char c = str.at(pos);

		if (escaped)												// Previous was escape char - add to token
		{
			token.push_back(c);
			escaped = false;
		}
		else if (escapeChars.find_first_of(c) != string::npos)		// Escape char - add next char to token
		{
			escaped = true;
		}
		else if (currentQuoteChar)								// We're in a quote
		{
			if (c == currentQuoteChar)							// Closing quote, no longer in quote
			{
				currentQuoteChar = 0;	
			}
			token.push_back(c);									// Add character within quote to token
		}
		else if (quoteChars.find_first_of(c) != string::npos)		// Start of quote
		{
			token.push_back(c);
			currentQuoteChar = c;	
		}
		else if (delimiterChars.find_first_of(c) != string::npos)	// Reached a delimiter
		{
			// Add complete token to token list
			if (!token.empty())
			{
				tokens.push_back(token);
				token.clear();
			}

			// Add delimiter as token
			if (delimiterTokenChars.find_first_of(c) != string::npos)
			{
				token.push_back(c);
				tokens.push_back(token);
				token.clear();
			}
		}
		else														// Add non-special character to current token
		{
			token.push_back(c);
		}

		// Advance to next character in string
		pos++;			
	}

	// Reached the end of the input - add last token, if any
	if (!token.empty())
	{
		tokens.push_back(token);			
	}
}

//----------------------------------------------------------------------------
// Util::GetPosFromString : Get user position coordinate from string
// representation
//----------------------------------------------------------------------------
inline bool GetLocCoordFromString(const string &str, LocCoord &number)
{
	char *end;
	long longNumber = strtol(str.c_str(), &end, 10);

	ASSERT(longNumber <= numeric_limits<LocCoord>::max()
		&& longNumber >=numeric_limits<LocCoord>::min(),
		"Read coord out of range of LocCoord type");
	
	if (longNumber > numeric_limits<LocCoord>::max()
		|| longNumber < numeric_limits<LocCoord>::min()
		|| end == str || *end != '\0' || errno == ERANGE)
	{
		return false;
	}
	else
	{
		number = (LocCoord)longNumber;
		return true;
	}
}

//----------------------------------------------------------------------------
// Util::BBBoxMerge : Merges two bounding boxes and stores the result
// in a destination bounding box. Returns point to destination bounding box.
//----------------------------------------------------------------------------
inline BoundBox* BBoxMerge(BoundBox *pDestBBox, const BoundBox *pBBox1, const BoundBox *pBBox2)
{
	pDestBBox->min.x = pBBox1->min.x < pBBox2->min.x ? pBBox1->min.x : pBBox2->min.x;
	pDestBBox->max.x = pBBox1->max.x > pBBox2->max.x ? pBBox1->max.x : pBBox2->max.x;

	pDestBBox->min.y = pBBox1->min.y < pBBox2->min.y ? pBBox1->min.y : pBBox2->min.y;
	pDestBBox->max.y = pBBox1->max.y > pBBox2->max.y ? pBBox1->max.y : pBBox2->max.y;

	pDestBBox->min.z = pBBox1->min.z < pBBox2->min.z ? pBBox1->min.z : pBBox2->min.z;
	pDestBBox->max.z = pBBox1->max.z > pBBox2->max.z ? pBBox1->max.z : pBBox2->max.z;

	return pDestBBox;
}

//----------------------------------------------------------------------------
// Util::BBoxContainsBBox : Given two bounding boxes, returns whether
// the first fits within the second.
//----------------------------------------------------------------------------
inline bool BBoxContainsBBox(const BoundBox &pBBox1, const BoundBox &pBBox2)
{
	if ( (pBBox2.min.x >= pBBox1.min.x) && (pBBox2.max.x <= pBBox1.max.x) )
		if ( (pBBox2.min.y >= pBBox1.min.y) && (pBBox2.max.y <= pBBox1.max.y) )
			if ( (pBBox2.min.z >= pBBox1.min.z) && (pBBox2.max.z <= pBBox1.max.z) )
				return true;

	return false;
}

inline bool BBoxIntersectsBBox(const BoundBox &pBBox1, const BoundBox &pBBox2)
{
	return !(pBBox1.min.x > pBBox2.max.x || pBBox2.min.x > pBBox1.max.x ||
		pBBox1.min.y > pBBox2.max.y || pBBox2.min.y > pBBox1.max.y ||
		pBBox1.min.z > pBBox2.max.z || pBBox2.min.z > pBBox1.max.z);
}

END_NAMESPACE(Util)
END_NAMESPACE(LDB)

#endif // LDB_UTIL_H
