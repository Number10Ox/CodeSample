//
//  Database.h
//  Jon Edwards Code Sample
//
//  Class to store Likes Database data
// 
//  Created by Jon Edwards on 12/3/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#ifndef LDB_DATABASE_H
#define LDB_DATABASE_H

#include <string>
#include <unordered_map>

#include "HashManager.h"
#include "RTree.h"

using namespace std;

BEGIN_NAMESPACE(LDB)

// Constants
const int kMaxInputLineLen = 256;	// Maximum line length that will be 
									// processed in input files

//----------------------------------------------------------------------------
// UserRecord: Contains data for a user in the database. 
//---------------------------------------------------------------------------
struct UserRecord
{
	typedef vector<HashKey> UserLikeList;		

	UserRecord() : userNameHash(0), phoneNumberHash(0), genderHash(0), xLoc(0), yLoc(0) { }
    
	HashKey userNameHash;       // 64-bit hash of user name
	HashKey phoneNumberHash;    // 64-bit hash of phone number string
	HashKey genderHash;         // 64-bit hash of gender name
 	LocCoord	xLoc;
	LocCoord	yLoc;
	UserLikeList userLikes;
};

extern const UserRecord sNullUserRecord;

inline bool operator==(const UserRecord &lhs, const UserRecord &rhs) 
{
	return lhs.userNameHash == rhs.userNameHash;
}

//----------------------------------------------------------------------------
// Database: Database that contains information about users and user
// likes. User data can be loaded from csv files.
// 
// Notes about database loading:
// 
//	1. Assumes that user names are unique, which is an unrealistic
//     assumption in the real world. User name is the only key shared
//	   between "user" and "likes" input data. 
//
//	2. Loading will report an error if a duplicate user is encountered
//	   CSV user data. Records with the same user name will be ignored
//	   after the first.
//
//----------------------------------------------------------------------------
class Database
{
    typedef unordered_map<HashKey, UserRecord> UserRecordList;
	typedef unordered_map<HashKey, UserRecord>::const_iterator UserRecordListIterator;
    
public:
	INJECT(Database(HashManagerInterface *hashManager)) : m_hashManager(hashManager), m_initialized(false) { }
	~Database();

	void Initialize();	
	void Shutdown();	

    bool LoadUserDataFromCSVFile(const char *fileName);
    bool LoadLikesDataFromCSVFile(const char *fileName);

    // Returns UserRecord for user name, otherwise kNullUserRecord
    const UserRecord& LookupUserRecordByName(const string &userName);
    const UserRecord& LookupUserRecordByKey(HashKey key);

    // Checks is user record returned by Lookup function is valid
    bool IsNullUserRecord(const UserRecord &record);

    // Update contents of a user record using values in the specified record
    bool UpdateUserRecord(const UserRecord &record);

    //------------------------------------------------------------------------
    // Query support
    uint32_t QueryUsersInRange(LocCoord x, LocCoord y, uint32_t range, vector<HashKey> &userList);

    //------------------------------------------------------------------------
    // String hash support
	HashKey GenerateHash(const string &str) { return m_hashManager->GenerateHash(str); }
   	bool LookupHashString(HashKey key, string &str) { return m_hashManager->LookupHashString(key, str); }

   	void LogUserRecord(const UserRecord &record);

    //----------------------------------------------------------------------------
	// Iterator for iterating through user records in databsae
	class UserRecordIterator
	{
	public:
		UserRecordIterator(const Database &database);
        
		UserRecordIterator& operator++();
		UserRecordIterator operator++(int32_t);
		HashKey GetHashKey() const;
		bool IsDone() const;
		void Reset();
        
	private:
		const Database &m_database;			// TODONOW Something like auto_ptr would be useful here
		UserRecordListIterator m_itr;
	};

private:
	void AddNewUserRecord(UserRecord &record);

	void ProcessUserDataRecordCSV(const char *fileName, uint32_t lineNum, const char *str);
	void ProcessLikesDataRecordCSV(const char *fileName, uint32_t lineNum, const char *str);

	bool m_initialized;

	HashManagerInterface *m_hashManager;
	UserRecordList m_userRecords;
	RTree m_rTree;
};

END_NAMESPACE(LDB)

#endif //LDB_DATABASE_H
