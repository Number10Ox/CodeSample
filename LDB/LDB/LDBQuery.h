//
//  LDBQuery.h
//  Jon Edwards Code Sample
//
//  Created by Jon Edwards on 12/6/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#ifndef LDBQUERY_H
#define LDBQUERY_H

#include "LDBDatabase.h"

using namespace std;

BEGIN_NAMESPACE(LDB)

//----------------------------------------------------------------------------
// LDBQuery : Abstract base class for LDB database queries
//----------------------------------------------------------------------------
class LDBQuery
{
public:
	LDBQuery() : m_isValid(false) { }
	virtual ~LDBQuery() { }

	// Parses query parameters from a string and constructs query.
	virtual bool Construct(const std::string &queryParameters) = 0;
	virtual bool Execute(Database &database) = 0;
	virtual bool WriteResultsToFile(Database &database, FILE *file) = 0;

	bool IsValid() const { return m_isValid; }

protected:
	bool m_isValid;
};

namespace LDBQueryUtil
{
	using namespace std;

 	bool ParseQuerySint32Parameter(int32_t &parameterValue, const vector<string> &tokens,
		vector<string>::const_iterator &tokensItr, const string &queryName, const string &parameterName);
 	
 	bool ParseQueryStringParameter(string &parameterValue, const vector<string> &tokens,
		vector<string>::const_iterator &tokensItr, const string &queryName, const string &parameterName);
}

END_NAMESPACE(LDB)

#endif // LDBQUERY_H
