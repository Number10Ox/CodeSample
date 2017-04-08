//
//  LDBQuery.cpp
//  Jon Edwards Code Sample
//
//  Created by Jon Edwards on 12/6/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#include "LDBQuery.h"

BEGIN_NAMESPACE(LDB)

namespace LDBQueryUtil
{
 	bool ParseQuerySint32Parameter(int32_t &parameterValue,
		const vector<string> &tokens, vector<string>::const_iterator &tokensItr,
		const string &queryName, const string &parameterName)
	{
		if (tokensItr == tokens.end())
		{
			LDBLogError("Error: Missing '=' for '%s' parameter of %s' query'\n",
				parameterName.c_str(), queryName.c_str());
			return false;
		}

		const std::string &assignmentToken = (*tokensItr);
		if (assignmentToken != "=")
		{
			LDBLogError("Error: Missing '=' for '%s' parameter of '%s' query, instead found: '%s'\n", 
				parameterName.c_str(), queryName.c_str(), assignmentToken.c_str());
			return false;
		}
	
		tokensItr++;
		if (tokensItr == tokens.end())
		{
			LDBLogError("Error: Missing value for '%s' parameter of '%s' query'\n",
				parameterName.c_str(), queryName.c_str());
			return false;
		}
	
		const std::string &valueToken = (*tokensItr);
		bool result = LDBUtil::GetLocCoordFromString(valueToken, parameterValue); \
		if (!result)
		{
			LDBLogError("Error: invalid numerical value '%s' for '%s' parameter of %s' query\n", 
				valueToken.c_str(), parameterName.c_str(), queryName.c_str());
			return false;
		}
	
		tokensItr++;

		return true;
	}

 	bool ParseQueryStringParameter(string &parameterValue,
		const vector<string> &tokens, vector<string>::const_iterator &tokensItr,
		const string &queryName, const string &parameterName)
	{
		if (tokensItr == tokens.end())
		{
			LDBLogError("Error: Missing '=' for '%s' parameter of %s' query'\n",
				parameterName.c_str(), queryName.c_str());
			return false;
		}

		const std::string &assignmentToken = (*tokensItr);
		if (assignmentToken != "=")
		{
			LDBLogError("Error: Missing '=' for '%s' parameter of '%s' query, instead found: '%s'\n", 
				parameterName.c_str(), queryName.c_str(), assignmentToken.c_str());
			return false;
		}
	
		tokensItr++;
		if (tokensItr == tokens.end())
		{
			LDBLogError("Error: Missing value for '%s' parameter of '%s' query'\n",
				parameterName.c_str(), queryName.c_str());
			return false;
		}
	
		parameterValue += "\"";
		parameterValue += (*tokensItr);
		parameterValue += "\"";

		tokensItr++;

		return true;
	}	
}

END_NAMESPACE(LDB)
