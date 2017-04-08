//
//  LDBHashManager.h
//  Jon Edwards Code Sample
//
//  Class manage string hashes for Database
// 
//  Created by Jon Edwards on 12/6/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#ifndef HASHMANAGER_H
#define HASHMANAGER_H

#include <map>
#include <unordered_map>

#include "fruit/fruit.h"
#include "LDBHashManagerInterface.h"
#include "LDBUtil.h"

using namespace std;

BEGIN_NAMESPACE(LDB)

//----------------------------------------------------------------------------
// LDBHashManger class : Manages string hashes for Database. Used for
// tracking string hashes. Can look up string for hash and detect hash
// collisions.
//
// TODO: Can templatize to take size hash algorithm as parameter
//----------------------------------------------------------------------------
class HashManager : public LDBHashManagerInterface
{
public:
	INJECT(HashManager()) = default;
	~HashManager();

	LDBHashKey GenerateHash(const std::string &str);
	bool LookupHashString(LDBHashKey key, string &str);

private:
	LDBHashKey HashString(const char *str, uint32_t seed = 0);

	unordered_map<LDBHashKey, string> m_stringHashTable;
};

END_NAMESPACE(LDB)

#endif // HASHMANAGER_H
