//
//  LDBQueryNearbyGender.h
//  Jon Edwards Code Sample
//
//  Created by Jon Edwards on 12/6/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//
// This query finds all pairs of users thatin a DISTANCE of each other
// that are both of GENDER. 
//
// TODO This can be generalized through use of templates to take an evaluation 
// function to determine whether nodes are candidates.
//
// TODO Mark records for search instead of building a list we want to visit?
// Would save space. Could perhaps cleverly  have a wrapping counter in
// UserRecord with new values for each search so we don't have to reset the
// "mark" variable(s)?
//
// TODO Perhap an algorithm like Tarjan's algorithm would be a better solution

#ifndef QUERYNEARBYGENDER_H
#define QUERYNEARBYGENDER_H

#include <unordered_set>
#include <stack>

#include "LDBDatabase.h"
#include "LDBQuery.h"

using namespace std;

BEGIN_NAMESPACE(LDB)

class QueryNearbyGender : public LDBQuery
{
public:
	struct NearbyGenderResult
	{
		LDBHashKey 	user1;
		LDBHashKey 	user2;
		float		distance;
	};

	QueryNearbyGender() : m_distance(0) { }
	~QueryNearbyGender() { }

	virtual bool Construct(const string &queryParameters);
	virtual bool Execute(Database &database);
	virtual bool WriteResultsToFile(Database &database, FILE *file);

	bool Construct(uint32_t distance, const string &gender);

	const vector<NearbyGenderResult> &GetResults();

	static const string &GetQueryName() { return s_queryName; }

private:
	bool UserMeetsSearchCriteria(Database &database, LDBHashKey userHashKey);
	bool UserMeetsSearchCriteria(Database &database, const UserRecord &userRecord);
	void ProcessDFSUserSearch(Database &database, LDBHashKey userHashKey);
	void AddResult(const UserRecord &userRecord1, const UserRecord &userRecord2);

	static const string s_queryName;
	uint32_t m_distance;
	string m_gender;
	LDBHashKey m_genderHash;

	unordered_set<LDBHashKey> m_searchVisitedList;
	stack<LDBHashKey> m_dfsSearchStack;

	vector<NearbyGenderResult> m_results;
};

END_NAMESPACE(LDB)

#endif // QUERYNEARBYGENDER_H
