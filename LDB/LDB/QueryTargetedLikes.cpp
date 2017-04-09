//
//  QueryTargetedLikes.cpp
//  Jon Edwards Code Sample
//
//  Created by Jon Edwards on 12/6/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#include "QueryTargetedLikes.h"

BEGIN_NAMESPACE(LDB)

const string QueryTargetedLikes::s_queryName = "targeted_likes";

//----------------------------------------------------------------------------
// QueryTargetedLikes::Construct : Parses query parameters from a string
// and constructs query.
//
// Expects a series of attribute assignments of the form "variable=value"
// where variable can be one of:
//
//		distance	:	integer value
//		x			: 	integer value
//		y			: 	integer value
//		like		:	string delineated by quotes with possibly 
//						= and \" characters within it
//
// TODO Can handle this declaratively and reduce boilerplate code
//----------------------------------------------------------------------------
bool QueryTargetedLikes::Construct(const string &queryParameters)
{
	m_results.clear();

	vector<string> tokens;
	vector<string>::const_iterator tokensItr;
    Util::TokenizeString(queryParameters, tokens, " =\t\n", "=");

	LocCoord xLoc = 0;;
	LocCoord yLoc = 0;;
	int32_t distance = 0;;
	string like;

    tokensItr = tokens.begin();
	while (tokensItr != tokens.end())
	{
		const string token = (*tokensItr);
		if (token == "distance")
		{
			tokensItr++;
		 	bool result = QueryUtil::ParseQuerySint32Parameter(distance,
		 		tokens, tokensItr, s_queryName, "distance");
			if (!result)
			{
				return false;	
			}
		}
		else if (token == "x")
		{
			tokensItr++;
		 	bool result = QueryUtil::ParseQuerySint32Parameter(xLoc, tokens,
		 		tokensItr, s_queryName, "x");
			if (!result)
			{
				return false;
			}
		}
		else if (token == "y")
		{
			tokensItr++;
			bool result = QueryUtil::ParseQuerySint32Parameter(yLoc, tokens,
		 		tokensItr, s_queryName, "y");
			if (!result)
			{
				return false;
			}
		}
		else if (token == "like")
		{
			tokensItr++;
			bool result = QueryUtil::ParseQueryStringParameter(like, tokens,
		 		tokensItr, s_queryName, "like");
			if (!result)
			{
				return false;
			}
		}
		else
		{
			LogError("Error: Unrecognized query parameter found: '%s'\n", token.c_str());
			return false;
		}
	}

	bool result = Construct(xLoc, yLoc, distance, like);
	return result;
}

//----------------------------------------------------------------------------
// QueryTargetedLikes::Construct : Constructs query, filling in parameters
// so that it may be excuted.
//----------------------------------------------------------------------------
bool QueryTargetedLikes::Construct(LocCoord x, LocCoord y, uint32_t distance, const string &like)
{
	m_results.clear();

	m_xLoc = x;
	m_yLoc = y;
	m_distance = distance;
	m_like = like;

	m_isValid = true;
	return true;
}

//----------------------------------------------------------------------------
// QueryTargetedLikes::Execute : Execute the query using the parameters
// that have been set.
//----------------------------------------------------------------------------
bool QueryTargetedLikes::Execute(Database &database)
{
	m_results.clear();

    // Query is not in valid state, e.g., not been constructed yet
	if (!m_isValid)
	{
		return false;
	}

	// Query all users in range
	vector<HashKey> userKeys;
    int count = database.QueryUsersInRange(m_xLoc, m_yLoc, m_distance, userKeys);
    
    // Iterate through users and find those with the like being queried
    if (count > 0)
    {
    	HashKey desireLikeHash = database.GenerateHash(m_like);

    	for (int i = 0; i < count; i++)
   		{
    		// Iterate through the user's list of likes 
        	const UserRecord &userRecord = database.LookupUserRecordByKey(userKeys[i]);
        	for (int likeNum = 0; likeNum < userRecord.userLikes.size(); likeNum++)
        	{
        		HashKey candidateLikeHash = userRecord.userLikes[likeNum];

    			/*
    			string desiredLikeString;
    			string candidateLikeString;
		    	database.LookupHashString(desireLikeHash, desiredLikeString);
		    	database.LookupHashString(candidateLikeHash, candidateLikeString);
		    	LogMessage("desireLikeHash string = '%s', candidateLikeHash string = '%s'\n",
		    		desiredLikeString.c_str(), candidateLikeString.c_str());
    			*/

        		if (candidateLikeHash == desireLikeHash)
        		{
        			m_results.push_back(userKeys[i]);
        		}
        	}
	    }
	}

    return true;
}

//----------------------------------------------------------------------------
// QueryTargetedLikes::WriteResultsToFile : Write result of query to file
// in CSV format
//----------------------------------------------------------------------------
bool QueryTargetedLikes::WriteResultsToFile(Database &database, FILE *file)
{		
	if (!m_isValid)
	{
		return false;
	}
	if (m_results.empty())
	{
		fprintf(file, "Query found no results.\n");
		return true;
	}

	for (int i = 0; i < m_results.size(); i++)
	{
		HashKey userNameHash = m_results[i];
		const UserRecord& userRecord = database.LookupUserRecordByKey(userNameHash);
		ASSERT(!database.IsNullUserRecord(userRecord), "Cannot find user referenced in query in the database");
		if (database.IsNullUserRecord(userRecord))
		{
			LogError("Error: Cannot find user referenced in query in the database\n");
			continue;
		}

		// Write out user data to file in csv format
		bool found = false;
		string userName;
		string phoneNumber;
		string gender;

		found = database.LookupHashString(userRecord.userNameHash, userName);
		ASSERT(found, "User name string not found in HashManager");
		found = database.LookupHashString(userRecord.phoneNumberHash, phoneNumber);
		ASSERT(found, "Phone number string not found in HashManager");
		found = database.LookupHashString(userRecord.genderHash, gender);
 		ASSERT(found, "Gender string not found in HashManager");

		fprintf(file, "%s, %s, %d, %d, %s\n", userName.c_str(), phoneNumber.c_str(), userRecord.xLoc,
			userRecord.yLoc, gender.c_str());
	}
    
    return true;
}

END_NAMESPACE(LDB)
