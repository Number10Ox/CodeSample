//
//  LDBQueryNearbyGender.cpp
//  Jon Edwards Code Sample
//
//  Created by Jon Edwards on 12/6/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#include <math.h>

#include "LDBQueryNearbyGender.h"

BEGIN_NAMESPACE(LDB)

const string QueryNearbyGender::s_queryName = "nearby_gender";

//----------------------------------------------------------------------------
// QueryNearbyGender::Construct : Parses query parameters from a string
// and constructs query.
//
// Expects a series of attribute assignments of the form "variable=value"
// where variable can be one of:
//
//		distance	:	integer value
//		gender		:	string delineated by quotes
//----------------------------------------------------------------------------o
bool QueryNearbyGender::Construct(const string &queryParameters)
{
	m_results.clear();

	vector<string> tokens;
	vector<string>::const_iterator tokensItr;
    LDBUtil::TokenizeString(queryParameters, tokens, " =\t\n", "=");

	int32_tdistance;
	string gender;
    
    tokensItr = tokens.begin();
	while (tokensItr != tokens.end())
	{
		const string token = (*tokensItr);
		if (token == "distance")
		{
			tokensItr++;
		 	bool result = LDBQueryUtil::ParseQuerySint32Parameter(distance,
		 		tokens, tokensItr, s_queryName, "distance");
			if (!result)
			{
				return false;	
			}
		}
		else if (token == "gender")
		{
			tokensItr++;
			bool result = LDBQueryUtil::ParseQueryStringParameter(gender, tokens,
		 		tokensItr, s_queryName, "gender");
			if (!result)
			{
				return false;
			}
		}
		else
		{
			LDBLogError("Error: Unrecognized query parameter found: '%s'\n", token.c_str());
			return false;
		}
	} 

	bool result = Construct(distance, gender);
	return result;
}

//----------------------------------------------------------------------------
// QueryTargetedLikes::Construct : Constructs query, filling in parameters
// so that it may be excuted.
//----------------------------------------------------------------------------
bool QueryNearbyGender::Construct(uint32_t distance, const string &gender)
{
	m_results.clear();

	m_distance = distance;
	m_gender = gender;

	m_isValid = true;

	return true;
}
  
//----------------------------------------------------------------------------
// QueryNearbyGender::Execute : Execute the query using the parameters
// that have been set. 
// 
// Pairs of neighbors (within a distance) of the same gender are going to
// be found by iterating through the users in the database. For each user
//	
//		a. That is not in the list of user records that have been visited
//		b. Meets the users data search critera (i.e., gender)
//
//	A depth first search (DFS) is done from that user to find all other
//	users  matching the criteria within the search range using
//	QueryUsersInRange. Every matching user found in range in the DFS
//  is pushed onto a search stack. When the DFS completes we contiue
//  iterating through the list of all users to find the next unvisited
//	matching candidate to do another DFS from.
//
// See TODO in class header for ideas other possible search approaches
// and implementations
//----------------------------------------------------------------------------
bool QueryNearbyGender::Execute(Database &database)
{
	m_results.clear();

    // Query is not in valid state, e.g., not been constructed yet
	if (!m_isValid)
	{
		return false;
	}
    
    // Cache hash to gender string used for comparison against records
	m_genderHash = database.GenerateHash(m_gender);
	if (m_genderHash == kInvalidHashKey)
	{
		m_isValid = false;
		return false;
	}

	// Start with clear list
	m_searchVisitedList.clear();

	// Iterate through all user records in the database
	for (Database::UserRecordIterator itr(database); !itr.IsDone(); ++itr)
	{
		LDBHashKey candidateHashKey = itr.GetHashKey();

		// Check that meets criteria
		bool meetsCriteria = UserMeetsSearchCriteria(database, candidateHashKey);	
		if (!meetsCriteria)
		{
			continue;	
		}
		// Check to make sure this isn't a user we've visited before in the search
		unordered_set<LDBHashKey>::iterator visitedItr;
		visitedItr = m_searchVisitedList.find(candidateHashKey);
		if (visitedItr != m_searchVisitedList.end())
		{
			continue;
		}

		// Do DFS starting with this user as the root
		ProcessDFSUserSearch(database, candidateHashKey);
		ASSERT(m_dfsSearchStack.empty(), "INTERNAL ERROR: DFS search stack isn't empty after completion of DFS\n")
		if (!m_dfsSearchStack.empty())
		{
			while (!m_dfsSearchStack.empty())
			{
				m_dfsSearchStack.pop();
			}
			return false;	
		}
	}

    return true;
}

void QueryNearbyGender::ProcessDFSUserSearch(Database &database, LDBHashKey rootCandidateHashKey)
{
	// Push candidate onto top of stack
	m_dfsSearchStack.push(rootCandidateHashKey);

	while (!m_dfsSearchStack.empty())
	{
		LDBHashKey candidateHashKey = m_dfsSearchStack.top();		
		m_dfsSearchStack.pop();

		// Mark as visited
		m_searchVisitedList.insert(candidateHashKey);

		// Find all neighbors in range
		const UserRecord &candidateRecord = database.LookupUserRecordByKey(candidateHashKey);
		vector<LDBHashKey> candidateNeighbors;
	    int count = database.QueryUsersInRange(candidateRecord.xLoc, candidateRecord.yLoc,
	    	m_distance, candidateNeighbors);

	    // For neighbors in range we haven't already visited check to see if they
	    // match the criteria and if they do add them to the DFS search stack
	   	for (int i = 0; i < count; i++)
	   	{
	   		LDBHashKey neighborHashKey = candidateNeighbors[i];
	   		unordered_set<LDBHashKey>::const_iterator itr = m_searchVisitedList.find(neighborHashKey);
	   		if (itr == m_searchVisitedList.end())
	   		{
				const UserRecord &neighborRecord = database.LookupUserRecordByKey(neighborHashKey);
				bool meetsCriteria = UserMeetsSearchCriteria(database, neighborRecord);
				if (meetsCriteria)
				{
					// We've found a matching result - add it to list of results
					AddResult(candidateRecord, neighborRecord);
					// Push the neighbor on the stack
					m_dfsSearchStack.push(neighborHashKey);
				}
			}
	   	} 
	}
}

//----------------------------------------------------------------------------
// QueryNearbyGender : We've found matching reseult. Add a result record
// to the list of results.
//----------------------------------------------------------------------------
void QueryNearbyGender::AddResult(const UserRecord &userRecord1, const UserRecord &userRecord2)
{
	NearbyGenderResult result;

	result.user1 = userRecord1.userNameHash;
	result.user2 = userRecord2.userNameHash;

	Uint32 distSquared = ((userRecord1.xLoc - userRecord2.xLoc) * (userRecord1.xLoc - userRecord2.xLoc))
			+ ((userRecord1.yLoc - userRecord2.yLoc) * (userRecord1.yLoc - userRecord2.yLoc));
	result.distance = sqrt(distSquared);

	m_results.push_back(result);
}

//----------------------------------------------------------------------------
// QueryNearbyGender::UserMeetsSearchCriteria : Checks whether the user
// meets the search criteria. (Note that this check could be a parameter
// to a templatized implementation of this query so it wouldn't be
// specific to checking gender).
//----------------------------------------------------------------------------
bool QueryNearbyGender::UserMeetsSearchCriteria(Database &database,
	LDBHashKey userHashKey)
{
	const UserRecord &userRecord = database.LookupUserRecordByKey(userHashKey);
	return UserMeetsSearchCriteria(database, userRecord);
}

bool QueryNearbyGender::UserMeetsSearchCriteria(Database &database,
	const UserRecord &userRecord)
{
	if (userRecord.genderHash != m_genderHash)
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
// QueryNearbyGender::WriteResultsToFile : Write result of query to file
// in CSV format
//----------------------------------------------------------------------------
bool QueryNearbyGender::WriteResultsToFile(Database &database, FILE *file)
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
		const NearbyGenderResult &searchResult = m_results[i];
		const UserRecord &userRecord1 = database.LookupUserRecordByKey(searchResult.user1);
		const UserRecord &userRecord2 = database.LookupUserRecordByKey(searchResult.user2);

		string user1Name;
		bool result = database.LookupHashString(userRecord1.userNameHash, user1Name);
		if (!result)
		{
			LDBLogError("INTERNAL ERROR: Name string not found in HashManager\n");	
		}

		string user2Name;
		result = database.LookupHashString(userRecord2.userNameHash, user2Name);
		if (!result)
		{
			LDBLogError("INTERNAL ERROR: Name string not found in HashManager\n");	
		}

		LDBLogMessage("%s, %s, %f\n", user1Name.c_str(), user2Name.c_str(), searchResult.distance);
	}
    
    return true;
}

END_NAMESPACE(LDB)
