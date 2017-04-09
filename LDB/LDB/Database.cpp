//  Database.cpp
//  Jon Edwards Code Sample
//
//  Class to store data for Likes Database application. Stores
//  user records containing data about users.
// 
//  Created by Jon Edwards on 12/3/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#include "Util.h"
#include "HashManager.h"
#include "Database.h"

#include <iostream>
#include <string>
#include <sstream>

BEGIN_NAMESPACE(LDB)

// Types of elements stored in the database (currently only user records)
enum RTreeElementTypes
{
	ElemType_Invalid = 0,
	ElemType_UserRecord = 1
};

// sNullUserRecord : Instance to represent a "null" user record
const UserRecord sNullUserRecord;

Database::~Database()
{
	Shutdown();
}

//----------------------------------------------------------------------------
// Database::Initialize : Needs to be called before database is used.
//----------------------------------------------------------------------------
void Database::Initialize()
{
    
    // Initialize R-tree extents to the full from of LocCoord values
    LocCoord locCoordMin = numeric_limits<LocCoord>::min();
    LocCoord locCoordMax = numeric_limits<LocCoord>::max();
    m_rTree.Initialize((float)locCoordMin, (float)locCoordMax);
    
	m_initialized = true;
}

//----------------------------------------------------------------------------
// Database::Shutdown : Frees database memory and goes back to 
// uninitialized state.
//----------------------------------------------------------------------------
void Database::Shutdown()
{
	m_initialized = false;
	m_rTree.Shutdown();
}

//----------------------------------------------------------------------------
// Database::LookupUserRecordByName : Looks up user record by user name.
// Returns pointer to user record if found, otherwise return sNullUserRecord
//----------------------------------------------------------------------------
const UserRecord &Database::LookupUserRecordByName(const string &userName)
{
	HashKey key = m_hashManager->GenerateHash(userName);
	return LookupUserRecordByKey(key);
}

//----------------------------------------------------------------------------
// Database::LookupUserRecordByName : Looks up user record by key, which
// should be a hash of the user name hash. Returns pointer to user record if
// found, otherwise NULL.
//----------------------------------------------------------------------------
const UserRecord &Database::LookupUserRecordByKey(HashKey key)
{
	ASSERT(key != kInvalidHashKey, "Invalid hash key encountered");
	if (key == kInvalidHashKey)
	{
		return sNullUserRecord;
	}

	UserRecordList::iterator itr = m_userRecords.find(key);
	if (itr == m_userRecords.end())
	{
		return sNullUserRecord;
	}

	UserRecord &record = (*itr).second;
	return record;
}	

//----------------------------------------------------------------------------
// Database::AddnewUserRecord : A new user record to the database.
//----------------------------------------------------------------------------
void Database::AddNewUserRecord(UserRecord &record)
{
	// Add to list of records
	m_userRecords[record.userNameHash] = record;

	// Add to Rtree	
	BoundBox bbox;
	bbox.min.x = record.xLoc;
	bbox.min.y = record.yLoc;
	bbox.min.z = 0.0f;
	bbox.max.x = record.xLoc;
	bbox.max.y = record.yLoc;
	bbox.max.z = 0.0f;
	m_rTree.Insert(bbox, ElemType_UserRecord, record.userNameHash);
}

//----------------------------------------------------------------------------
// Database::AddnewUserRecord : Update the contents of a user record
// in database.
//----------------------------------------------------------------------------
bool Database::UpdateUserRecord(const UserRecord &record)
{
	UserRecordList::iterator itr = m_userRecords.find(record.userNameHash);
	if (itr == m_userRecords.end())
	{
		return false;
	}

	UserRecord &existingRecord = (*itr).second;
	existingRecord = record;

	return true;
}

//----------------------------------------------------------------------------
// Database::IsNullUserRecord : Check if valid user record
//----------------------------------------------------------------------------
bool Database::IsNullUserRecord(const UserRecord &record)
{
	return record == sNullUserRecord;
}

uint32_t Database::QueryUsersInRange(LocCoord x, LocCoord y, uint32_t range, vector<HashKey> &userList)
{
	// Find candidates in bounding box encompassing the point and radius
	BoundBox bbox;
	bbox.min.x = (float)x - (float)range;
	bbox.min.y = (float)y - (float)range;
	bbox.min.z = 0.0f;
	bbox.max.x = (float)x + (float)range;
	bbox.max.y = (float)y + (float)range;
	bbox.max.z = 0.0f;

	vector<HashKey> candidateUsers;
    vector<RTreeObjectCategoryType_t> categories;
	uint32_t count = m_rTree.IntersectsQuery(bbox, categories, candidateUsers);

	// Find candidates actually within range distance
	uint32_t rangeSquared = range * range;
    uint32_t inRangeCount = 0;
	for (int i = 0; i < count; i++)
	{
		const UserRecord &candidate = LookupUserRecordByKey(candidateUsers[i]);

		int32_t distSquared = ((x - candidate.xLoc) * (x - candidate.xLoc))
			+ ((y - candidate.yLoc) * (y - candidate.yLoc));
		if (distSquared <= rangeSquared)
		{
			userList.push_back(candidateUsers[i]);
            inRangeCount++;
		}
	}

	return inRangeCount;
}

//============================================================================
//
//							Database Loading
//
//============================================================================

//----------------------------------------------------------------------------
// Database::LoadUserDataFromCSVFile : Load and and process a file adding
// new users to the database.
//----------------------------------------------------------------------------
bool Database::LoadUserDataFromCSVFile(const char *fileName)
{
	if (!m_initialized)
	{
		LogError("Error: Database::LoadUserDataFromCSVFile -  database not initialized\n");
		return false;
	}

	FILE *file = fopen(fileName, "r");
	if (file == NULL)
	{
		LogError("Error: Database::LoadUserDataFromCSVFile -  could not open file '%s'\n", fileName);
		return false;
	}

	char inputLine[kMaxInputLineLen];
	uint32_t lineNum = 0;
	while (fgets(inputLine, kMaxInputLineLen, file))
	{
		ProcessUserDataRecordCSV(fileName, lineNum, inputLine);
		lineNum++;
	}

	fclose(file);

    return true;
}

//----------------------------------------------------------------------------
// Database::LoadLikesDataFromCSVFile : Load and process a multiple line
// CSV file that contains data about user likes. Users should already be
// registered in the system. Database is updated with the like information.
//----------------------------------------------------------------------------
bool Database::LoadLikesDataFromCSVFile(const char *fileName)
{
	if (!m_initialized)
	{
		LogError("Error: Database::LoadLikeDataFromCSVFile -  database not initialized\n");
		return false;
	}

	FILE *file = fopen(fileName, "r");
	if (file == NULL)
	{
		LogError("Error: Database::LoadLikeDataFromCSVFile -  could not open file '%s'\n", fileName);
		return false;
	}

	char inputLine[kMaxInputLineLen];
	unsigned int lineNum = 0;
	while (fgets(inputLine, kMaxInputLineLen, file))
	{
		ProcessLikesDataRecordCSV(fileName, lineNum, inputLine);
		lineNum++;
	}

	fclose(file);

    return true;
}

//----------------------------------------------------------------------------
// Database::ProcessUserDataRecordCSV : Loads a user record, which is
// expected to be of the format
//
//		"User Name", "Phone number", xloc(int), yloc(int), "gender"
//
// TODO Can generalize this more elegantly than these macros, e.g.,
//		automate reading of data using a schema and filling a data structure,
//		create a template algorithm
//----------------------------------------------------------------------------
#define PARSE_STRING_TOKEN(TOKEN_LIST, TOKENS_ITR, VARIABLE, VARIABLENAME, FILENAME, LINENUM, INPUTLINE) \
	{ \
		if (TOKENS_ITR == TOKEN_LIST.end()) \
		{ \
			LogError("Error reading data file '%s' (line: %d) could not find expected string variable '%s' in input line '%s'\n", \
				FILENAME, LINENUM, VARIABLENAME, INPUTLINE); \
			return;	\
		} \
		VARIABLE = m_hashManager->GenerateHash(*TOKENS_ITR); \
	}

#define PARSE_COORD_TOKEN(TOKEN_LIST, TOKENS_ITR, VARIABLE, VARIABLENAME, FILENAME, LINENUM, INPUTLINE) \
	{ \
		if (TOKENS_ITR == TOKEN_LIST.end()) \
		{ \
			LogError("Error reading data file '%s' (line: %d) could not find expected integer variable '%s' in input line '%s'\n", \
				FILENAME, LINENUM, VARIABLENAME, INPUTLINE); \
			return;	\
		} \
		bool result = Util::GetLocCoordFromString(*TOKENS_ITR, VARIABLE); \
		if (!result) \
		{ \
			LogError("Error reading data file '%s' (line: %d): found a value for variable <%s> that was not valid integer in input line '%s'\n", \
				FILENAME, LINENUM, VARIABLENAME, INPUTLINE); \
			return; \
		} \
	}

void Database::ProcessUserDataRecordCSV(const char *fileName, uint32_t lineNum, const char *str)
{
	//* DEBUG */ LogMessage("ProcessUserDataRecordCSV: line read '%s'\n", str);

	string inputLine(str);
	vector<string> tokens;

    Util::TokenizeString(inputLine, tokens, " ,\t\n", "");

	// Blank line - return
	if (tokens.size() == 0)
	{
		return;	
	}

	vector<string>::const_iterator tokensItr = tokens.begin();
	UserRecord newRecord;
	bzero(&newRecord, sizeof (newRecord));

	PARSE_STRING_TOKEN(tokens, tokensItr, newRecord.userNameHash, "<user name>", fileName, lineNum, inputLine.c_str());

	// Check to see if there's already a user record with the same user name.
	// If there is then skip this input line.
    const UserRecord &existingRecord = LookupUserRecordByKey(newRecord.userNameHash);
    if (!IsNullUserRecord(existingRecord))
	{
		LogError("Error: Cannot add new user '%s', already exists.\n", (*tokensItr).c_str());
		return;
    }
	tokensItr++;

	PARSE_STRING_TOKEN(tokens, tokensItr, newRecord.phoneNumberHash, "<phone number>", fileName, lineNum, inputLine.c_str());
	tokensItr++;
	PARSE_COORD_TOKEN(tokens, tokensItr, newRecord.xLoc, "<xLoc>", fileName, lineNum, inputLine.c_str());
	tokensItr++;
	PARSE_COORD_TOKEN(tokens, tokensItr, newRecord.yLoc, "<yLoc>", fileName, lineNum, inputLine.c_str());
	tokensItr++;
	PARSE_STRING_TOKEN(tokens, tokensItr, newRecord.genderHash, "<gender>", fileName, lineNum, inputLine.c_str());
	tokensItr++;

	// Add to database
	AddNewUserRecord(newRecord);

	//* DEBUG */ LogUserRecord(newRecord);
}

//----------------------------------------------------------------------------
// Database::ProcessLikesDataRecordCSV : Read likes.csv and add user likes
// to users in database.
// 
// Note: An error message is generated when an entry is encountered that isn't
// for a registered user.
//----------------------------------------------------------------------------
void Database::ProcessLikesDataRecordCSV(const char *fileName, uint32_t lineNum, const char *str)
{
	//* DEBUG */ LogMessage("LoadLikesDataFromCSVFile: line read '%s'\n", str);

	string inputLine(str);
	vector<string> tokens;

	Util::TokenizeString(inputLine, tokens, " ,\t\n", "");
    
	// Blank line - return
	if (tokens.size() == 0)
	{
		return;	
	}

	vector<string>::const_iterator tokensItr = tokens.begin();

	// Read user name 
	HashKey userNameHash; 
	PARSE_STRING_TOKEN(tokens, tokensItr, userNameHash, "<user name>", fileName, lineNum, inputLine.c_str());

	// Make sure user is registered in system
	const string userName = (*tokensItr);
	tokensItr++;

	const UserRecord &existingRecord = LookupUserRecordByName(userName);
    if (IsNullUserRecord(existingRecord))
	{
		LogError("Error reading file '%s' (line: %d) : Cannot find user '%s' in database. Skipping input line: '%s'\n",
			fileName, lineNum, userName.c_str(), inputLine.c_str());
		return;
    }

    HashKey userLikeHash;
	PARSE_STRING_TOKEN(tokens, tokensItr, userLikeHash, "<user like>", fileName, lineNum, inputLine.c_str());
	if (userLikeHash != kInvalidHashKey)
	{
		// Copy existing record, make modifications, and update record in database
		UserRecord updatedRecord = existingRecord;
		updatedRecord.userLikes.push_back(userLikeHash);

		bool result = UpdateUserRecord(updatedRecord);
		ASSERT(result, "Update to user record failed\n")

	//* DEBUG */ const UserRecord &recordAfterUpdate = LookupUserRecordByKey(existingRecord.userNameHash);
	//* DEBUG */ ASSERT(!IsNullUserRecord(recordAfterUpdate), "Failed to retrieve record after update\n");
	//* DEBUG */ LogUserRecord(recordAfterUpdate);
	}
}

//----------------------------------------------------------------------------
// Database::LogUserRecord : Print out database user record to log
//----------------------------------------------------------------------------
void Database::LogUserRecord(const UserRecord &record)
{
	LogMessage("User record:\n");
	bool found = false;
	string userName;
	string phoneNumber;
	string gender;

	found = LookupHashString(record.userNameHash, userName);
	ASSERT(found, "User name string not found in HashManager");
	found = LookupHashString(record.phoneNumberHash, phoneNumber);
	ASSERT(found, "Phone number string not found in HashManager");
	found = LookupHashString(record.genderHash, gender);
 	ASSERT(found, "Gender string not found in HashManager");

	LogMessage("\tkey='%llu', userName='%s', phoneNumber='%s', xLoc='%d', yLoc='%d', gender='%s'\n",
		record.userNameHash, userName.c_str(), phoneNumber.c_str(), record.xLoc, record.yLoc, gender.c_str());

	UserRecord::UserLikeList::const_iterator itr = record.userLikes.begin();
	if (itr != record.userLikes.end())
	{
		LogMessage("\tLikes: ");
		for (int i = 0; i < record.userLikes.size(); i++)
		{
			string like;
			found = LookupHashString(record.userLikes[i], like);
			ASSERT(found, "Like string not found in HashManager");
			LogMessage("%s ", like.c_str());
		}
		LogMessage("\n");
	}
	else
	{
		LogMessage("\t[No Likes]\n");
	}
}


//----------------------------------------------------------------------------
//					class Database::UserRecordIterator
//
// TODO Smart reference in case database gets destroyed
// TODO Create generic iterator wrapper using templates (or use Boost)
//----------------------------------------------------------------------------
Database::UserRecordIterator::UserRecordIterator(const Database &database)
 : m_database(database)
{
	Reset(); 
}

bool Database::UserRecordIterator::IsDone() const
{
	if (m_itr == m_database.m_userRecords.end())
		return true;
	else
		return false;
}

void  Database::UserRecordIterator::Reset()
{
	m_itr = m_database.m_userRecords.begin();
}

Database::UserRecordIterator&
Database::UserRecordIterator::operator++()
{
    if (!IsDone())
    {
         ++m_itr;
    }
	return *this;
}

Database::UserRecordIterator
Database::UserRecordIterator::operator++(int)
{
	Database::UserRecordIterator retVal = *this;
	++(*this);
	return retVal;
}

HashKey Database::UserRecordIterator::GetHashKey() const
{
	if (IsDone())
		return kInvalidHashKey;

	HashKey hashKey = (*m_itr).first;
	return hashKey;
}

END_NAMESPACE(LDB)
