//
//  QueryTargetedLikes.h
//  Jon Edwards Code Sample
//
//  Created by Jon Edwards on 12/6/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//
// This query finds all users within DISTANCE radius of (X, Y)
// that like a particular LIKE.
//

#ifndef LDB_QUERYTARGETEDLIKES_H
#define LDB_QUERYTARGETEDLIKES_H

#include <stack>

#include "Database.h"
#include "Query.h"

using namespace std;

BEGIN_NAMESPACE(LDB)

class QueryTargetedLikes : public Query
{
public:
	QueryTargetedLikes() : m_xLoc(0), m_yLoc(0), m_distance(0) { }
	~QueryTargetedLikes() { }

	virtual bool Construct(const string &queryParameters);
	virtual bool Execute(Database &database);
	virtual bool WriteResultsToFile(Database &database, FILE *file);

	bool Construct(LocCoord x, LocCoord y, uint32_t distance, const string &like);

	const vector<HashKey> &GetResults();

	static const string &GetQueryName() { return s_queryName; }

private:
	static const string s_queryName;

	LocCoord m_xLoc;
	LocCoord m_yLoc;
	uint32_t m_distance;
	string m_like;

	vector<HashKey> m_results;
};

END_NAMESPACE(LDB)

#endif // LDB_QUERYTARGETEDLIKES_H
