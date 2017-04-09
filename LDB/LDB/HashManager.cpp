//  HashManager.cpp
//  Jon Edwards Code Sample
//
//  Class to store Likes Database data
// 
//  Created by Jon Edwards on 12/6/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#include "HashManager.h"

BEGIN_NAMESPACE(LDB)
	
HashManager::~HashManager()
{
	// Nothing to do
}

HashKey HashManager::GenerateHash(const string &str)
{
	if (str.empty())
	{
		return kInvalidHashKey;
	}

	HashKey key = HashString(str.c_str());
	ASSERT(key != kInvalidHashKey, "Valid hash generated same value as kInvalidHashKey");
	if (key == kInvalidHashKey)
	{
		LogError("Error: String '%s' generated invalid hash value\n", str.c_str());
	}
	else
	{
		string registeredString;
		bool found = LookupHashString(key, registeredString);
		if (found)
		{
			// We've found key in string has table. Make sure we don't have a collision.
			if (str != registeredString)
			{
				LogError("Error: Discovered hash conflict for strings '%s' and '%s'\n",
					str.c_str(), registeredString.c_str());
				key = kInvalidHashKey;
			}
		}
		else
		{
			// Record string in table
			m_stringHashTable[key] = str;
		}		
	}

	return key;
}

bool HashManager::LookupHashString(HashKey key, string &str)
{
	unordered_map<HashKey, string>::const_iterator itr;
	itr = m_stringHashTable.find(key);
	if (itr == m_stringHashTable.end())
	{
		str = kInvalidString;
		return false;
	}

	str = (*itr).second;

	return true;
}

//---------------------------------------------------------------------------
// HashString
//
// Uses 64-CRC Murmur public domain hash algorithm taken from
// https://sites.google.com/site/murmurhash/
//---------------------------------------------------------------------------
HashKey HashManager::HashString(const char *str, uint32_t seed /* = 0 */)
{
	const uint64_t m = 0xc6a4a7935bd1e995;
	const int r = 47;
	const size_t len = strlen(str);

	uint64_t h = seed ^ (len * m);

	const uint64_t * data = (const uint64_t *)str;
	const uint64_t * end = data + (len / 8);

	while (data != end)
	{
		uint64_t k = *data++;

		k *= m; 
		k ^= k >> r; 
		k *= m; 
		
		h ^= k;
		h *= m; 
	}

	const unsigned char * data2 = (const unsigned char*)data;

	switch(len & 7)
	{
		case 7: h ^= uint64_t(data2[6]) << 48;
		case 6: h ^= uint64_t(data2[5]) << 40;
		case 5: h ^= uint64_t(data2[4]) << 32;
		case 4: h ^= uint64_t(data2[3]) << 24;
		case 3: h ^= uint64_t(data2[2]) << 16;
		case 2: h ^= uint64_t(data2[1]) << 8;
		case 1: h ^= uint64_t(data2[0]);
		        h *= m;
	};
 
	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
} 

END_NAMESPACE(LDB)
