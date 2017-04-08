//
//  LDBQueryTargetedLikes.h
//  Jon Edwards Code Sample
//
//  Created by Jon Edwards on 12/6/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//
// This query finds all users within DISTANCE radius of (X, Y)
// that like a particular LIKE.
//

#ifndef QUERYTARGETEDLIKES_H
#define QUERYTARGETEDLIKES_H

#include <stack>

#include "LDBDatabase.h"
#include "LDBQuery.h"

using namespace std;

BEGIN_NAMESPACE(LDB)

class QueryTargetedLikes : public LDBQuery
{
public:
	QueryTargetedLikes() : m_xLoc(0), m_yLoc(0), m_distance(0) { }
	~QueryTargetedLikes() { }

	virtual bool Construct(const string &queryParameters);
	virtual bool Execute(Database &database);
	virtual bool WriteResultsToFile(Database &database, FILE *file);

	bool Construct(LDBLocCoord x, LDBLocCoord y, uint32_t distance, const string &like);

	const vector<LDBHashKey> &GetResults();

	static const string &GetQueryName() { return s_queryName; }

private:
	static const string s_queryName;

	LDBLocCoord m_xLoc;
	LDBLocCoord m_yLoc;
	uint32_t m_distance;
	string m_like;

	vector<LDBHashKey> m_results;
};

END_NAMESPACE(LDB)

#endif // QUERYTARGETEDLIKES_H
