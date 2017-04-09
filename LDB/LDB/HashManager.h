//
//  HashManager.h
//  Jon Edwards Code Sample
//
//  Class manage string hashes for Database
// 
//  Created by Jon Edwards on 12/6/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#ifndef LDB_HASHMANAGER_H
#define LDB_HASHMANAGER_H

#include <map>
#include <unordered_map>

#include "fruit/fruit.h"
#include "HashManagerInterface.h"
#include "Util.h"

using namespace std;

BEGIN_NAMESPACE(LDB)

//----------------------------------------------------------------------------
// HashManger class : Manages string hashes for Database. Used for
// tracking string hashes. Can look up string for hash and detect hash
// collisions.
//----------------------------------------------------------------------------
class HashManager : public HashManagerInterface
{
public:
	INJECT(HashManager()) = default;
	~HashManager();

	HashKey GenerateHash(const std::string &str);
	bool LookupHashString(HashKey key, string &str);

private:
	HashKey HashString(const char *str, uint32_t seed = 0);

	unordered_map<HashKey, string> m_stringHashTable;
};

END_NAMESPACE(LDB)

#endif // LDB_HASHMANAGER_H
