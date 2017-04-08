//
//  LDBDatabaseInterface.h
//  Jon Edwards Code Sample
//
//  Class to store Likes Database data
// 
//  Created by Jon Edwards on 12/3/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#ifndef LDBDATABASEINTERFACE_H
#define LDBDATABASEINTERFACE_H

#include <string>
#include <unordered_map>

#include "LDBHashManager.h"
#include "LDBRTree.h"

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
	typedef vector<LDBHashKey> UserLikeList;		

	UserRecord() : userNameHash(0), phoneNumberHash(0), genderHash(0), xLoc(0), yLoc(0) { }
    
	LDBHashKey userNameHash;     // 64-bit hash of user name
	LDBHashKey phoneNumberHash;	 // 64-bit hash of phone number string
	LDBHashKey genderHash;       // 64-bit hash of gender name
 	LDBLocCoord	xLoc;
	LDBLocCoord	yLoc;
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
// TODO Inject dependency upon HashManager into database
//----------------------------------------------------------------------------
class Database
{
    typedef unordered_map<LDBHashKey, UserRecord> UserRecordList;
	typedef unordered_map<LDBHashKey, UserRecord>::const_iterator UserRecordListIterator;
    
public:
	INJECT(Database(LDBHashManagerInterface *hashManager)) : m_hashManager(hashManager), m_initialized(false) { }
	~Database();

	void Initialize();	
	void Shutdown();	

    bool LoadUserDataFromCSVFile(const char *fileName);
    bool LoadLikesDataFromCSVFile(const char *fileName);

    // Returns UserRecord for user name, otherwise kNullUserRecord
    const UserRecord& LookupUserRecordByName(const string &userName);
    const UserRecord& LookupUserRecordByKey(LDBHashKey key);

    // Checks is user record returned by Lookup function is valid
    bool IsNullUserRecord(const UserRecord &record);

    // Update contents of a user record using values in the specified record
    bool UpdateUserRecord(const UserRecord &record);

    //------------------------------------------------------------------------
    // Query support
    uint32_t QueryUsersInRange(LDBLocCoord x, LDBLocCoord y, uint32_t range, vector<LDBHashKey> &userList);

    //------------------------------------------------------------------------
    // String hash support
	LDBHashKey GenerateHash(const string &str) { return m_hashManager->GenerateHash(str); }
   	bool LookupHashString(LDBHashKey key, string &str) { return m_hashManager->LookupHashString(key, str); }

   	void LogUserRecord(const UserRecord &record);

    //----------------------------------------------------------------------------
	// Iterator for iterating through user records in databsae
	class UserRecordIterator
	{
	public:
		UserRecordIterator(const Database &database);
        
		UserRecordIterator& operator++();
		UserRecordIterator operator++(int32_t);
		LDBHashKey GetHashKey() const;
		bool IsDone() const;
		void Reset();
        
	private:
		const Database &m_database;			// TODONOW Something like auto_ptr would be useful here
		UserRecordListIterator m_itr;
	};

private:
	void AddNewUserRecord(UserRecord &record);

	void ProcessUserDataRecordCSV(const char *fileName, int32_t lineNum, const char *str);
	void ProcessLikesDataRecordCSV(const char *fileName, int32_t lineNum, const char *str);

	bool m_initialized;

	LDBHashManagerInterface *m_hashManager;
	UserRecordList m_userRecords;
	RTree m_rTree;
};

END_NAMESPACE(LDB)

#endif //Database_H
