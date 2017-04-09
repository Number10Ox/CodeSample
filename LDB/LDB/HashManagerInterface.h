//
//  HashManagerInterface.h
//  Jon Edwards Code Sample
//
//  Interface for managing string hashes
// 
//  Created by Jon Edwards on 4/7/17.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#ifndef LDB_HASHMANAGERINTERFACE_H
#define LDB_HASHMANAGERINTERFACE_H

#include <map>
#include <unordered_map>

#include "Util.h"

using namespace std;

BEGIN_NAMESPACE(LDB)

const HashKey kInvalidHashKey = 0;
const string kInvalidString = "(invalid string)";

//----------------------------------------------------------------------------
// HashManger interface : Class for managing string hashes for Database.
// Used for tracking string hashes. Can look up string for hash and detect 
// hash collisions.
//----------------------------------------------------------------------------
class HashManagerInterface
{
public:
    virtual ~HashManagerInterface() { };

	virtual HashKey GenerateHash(const std::string &str) = 0;
	virtual bool LookupHashString(HashKey key, string &str) = 0;
};

END_NAMESPACE(LDB)

#endif // LDB_HASHMANAGERINTERFACE_H
