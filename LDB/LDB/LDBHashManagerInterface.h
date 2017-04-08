//
//  LDBHashManagerInterface.h
//  Jon Edwards Code Sample
//
//  Interface for managing string hashes
// 
//  Created by Jon Edwards on 4/7/17.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#ifndef HASHMANAGERINTERFACE_H
#define HASHMANAGERINTERFACE_H

#include <map>
#include <unordered_map>

#include "LDBUtil.h"

using namespace std;

BEGIN_NAMESPACE(LDB)

const LDBHashKey kInvalidHashKey = 0;
const string kInvalidString = "(invalid string)";

//----------------------------------------------------------------------------
// LDBHashManger interface : Class for managing string hashes for Database.
// Used for tracking string hashes. Can look up string for hash and detect 
// hash collisions.
//
// TODO: Can templatize to take size hash algorithm as parameter
//----------------------------------------------------------------------------
class LDBHashManagerInterface
{
public:
	virtual ~LDBHashManagerInterface() = 0;

	virtual LDBHashKey GenerateHash(const std::string &str) = 0;
	virtual bool LookupHashString(LDBHashKey key, string &str) = 0;
};

END_NAMESPACE(LDB)

#endif // HASHMANAGERINTERFACE_H
