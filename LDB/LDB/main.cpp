//
//  main.cpp
//  Jon Edwards Code Test
//
//  Created by Jon Edwards on 12/3/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#include <iostream>
#include <getopt.h>
#include <string.h>

#include "Util.h"
#include "Database.h"
#include "QueryTargetedLikes.h"
#include "QueryNearbyGender.h"

using namespace LDB;
using namespace std;

static string sUsersDataFileName = "users.csv";
static string sLikesDataFileName = "likes.csv";

static void ParseCommandLine(int argc, const char **argv);
static void ParseCommandLineQuery(int argc, const char **argv);
static void ExecuteCommandLineQuery(Database &database, Query &query, const string &queryParameters);
static bool LoadDatabase(Database &database, const string &usersDataFileName, const string &likesDataFileName);

static void PrintUsage();
static void RunUnitTest();

using fruit::Injector;

//============================================================================
//                                  Main
//============================================================================

int main(int argc, const char *argv[])
{
	ParseCommandLine(argc, argv);
    return 0;
}

static void ParseCommandLine(int argc, const char **argv)
{
    if (argc < 2)
    {
        PrintUsage();
        return;
    }

    bool queryFound = false;

    char c;
    while ((c = getopt(argc, (char **)argv, "qu:l:t")) != -1)
    {
    	switch (c)
    	{
        case 'u':
            sUsersDataFileName = optarg; 
            break;
        case 'l':
            sLikesDataFileName = optarg; 
            break;
    	case 'q':
            if (optind < argc)
            {
                queryFound = true;
                ParseCommandLineQuery(argc - optind, argv + optind);
            }
            else
            {
                PrintUsage();
            }
	    	break;
	    case 't':
            queryFound = true;
            RunUnitTest();
	    	break;
    	default:
			PrintUsage();
    	}
    }

    if (!queryFound)
    {
        LogMessage("No query found.\n");
        PrintUsage();
    }
}

static void PrintUsage()
{
	LogMessage("Usage: likedb [-u users.csv] [-l likes.csv] [-t] [-q query_string][\n");
    LogMessage("\t-u Load user data CSV file [defaults to 'likes.csv']\n");
    LogMessage("\t-l Load like data CSV file [defaults to 'likes.csv']\n");
    LogMessage("\t-t Runs application internal unit test\n");
    LogMessage("\t-q Executes a database query using args following -q. Query string can be one of:\n");
	LogMessage("\t\ttarget_likes distance=num x=num y=num like=like_value\n");
	LogMessage("\t\tnearby_gender distance=num gender=gender_value\n\n");
}

//----------------------------------------------------------------------------
// getDatabaseComponent: Builds database with hash manger
//----------------------------------------------------------------------------
const fruit::Component<Database>& getDatabaseComponent() {
    static const fruit::Component<Database> databaseComponent = fruit::createComponent()
    .bind<HashManagerInterface, HashManager>();
    return databaseComponent;
}

//----------------------------------------------------------------------------
// LoadDatabase: Load likes database with users and likes data
//----------------------------------------------------------------------------
static bool LoadDatabase(Database &database, const string &usersDataFileName,
    const string &likesDataFileName)
{
    bool result = database.LoadUserDataFromCSVFile(usersDataFileName.c_str());
    if (!result) 
    {
        LogMessage("Error: Couldn't find user data file '%s'\n", usersDataFileName.c_str());
        return false;
    }
    result = database.LoadLikesDataFromCSVFile(likesDataFileName.c_str());
    if (!result) 
    {
        LogMessage("Error: Couldn't find likes data file '%s'\n", usersDataFileName.c_str());
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------
// Parse Command Line Query: A query command has been specified on the
// command line. Determine its type and execute it.
//
// TODO Could make use of query factory
//----------------------------------------------------------------------------
static void ParseCommandLineQuery(int argc, const char **argv)
{
    if (argc == 0)
    {
        PrintUsage();
        return;
    }
    
    // Load database
    Injector<Database> injector(getDatabaseComponent());
    Database *database(injector);
    database->Initialize();

    bool result = LoadDatabase(*database, sUsersDataFileName, sLikesDataFileName);
    if (!result) 
    {
        return;
    }

    // Construct query string from command line arguments
    const string queryName = argv[0];
    string queryParameters;
    for (int i = 1; i < argc; i++)
    {
        queryParameters += argv[i];
        queryParameters += " "; 
    }

    if (queryName == QueryTargetedLikes::GetQueryName())
    {
        QueryTargetedLikes targetedLikesQuery;
        ExecuteCommandLineQuery(*database, targetedLikesQuery, queryParameters);
    }
    else if (queryName == QueryNearbyGender::GetQueryName())
    {
        QueryNearbyGender targetedNearbyGender;
        ExecuteCommandLineQuery(*database, targetedNearbyGender, queryParameters);
    }
    else
    {
        LogMessage("Unknown query type encountered: '%s'\n", queryName.c_str());
        PrintUsage();
    }
}

//----------------------------------------------------------------------------
// Execute Query String : Execute a query specified by a string on the
// command line. 
//----------------------------------------------------------------------------
static void ExecuteCommandLineQuery(Database &database, Query &query,
    const string &queryParameters)
{
    // Construct query of type specified, execute it, and write results to
    // stdout
    bool result = query.Construct(queryParameters);
    if (result)
    {
        result = query.Execute(database);
        if (result)
        {
            result = query.WriteResultsToFile(database, stdout);
        }
    }
}

//============================================================================
//                                  Unit Tests
//============================================================================

static bool RunTokenizeUnitTest();

void RunUnitTest()
{
    bool result = true;

    result = RunTokenizeUnitTest();
    if (!result) 
    {
        LogError("---- UNIT TEST FAILED ----\n");
        return;
    }     

    Injector<Database> injector(getDatabaseComponent());
    Database *database(injector);
    database->Initialize();

    result = LoadDatabase(*database, sUsersDataFileName, sLikesDataFileName);
    if (!result) 
    {
        LogError("---- UNIT TEST FAILED ----\n");
        
    } 

    QueryTargetedLikes targetedLikesQuery;
    string likesParameters = "distance=100 x=27 y=127 like=pizza";
    result = targetedLikesQuery.Construct(likesParameters);
    result = targetedLikesQuery.Execute(*database);
    result = targetedLikesQuery.WriteResultsToFile(*database, stdout);
    if (!result) 	
    {
        LogError("---- UNIT TEST FAILED ----\n");
        return;
    } 

    QueryNearbyGender targetedNearbyGender;
    string nearbyParameters = "distance=20 gender=male";
    result = targetedNearbyGender.Construct(nearbyParameters);
    //result = targetedNearbyGender.Construct(20, "male");
    result = targetedNearbyGender.Execute(*database);
    result = targetedNearbyGender.WriteResultsToFile(*database, stdout);
    if (!result) 
    {
        LogError("---- UNIT TEST FAILED ----\n");
        return;
    } 


    database->Shutdown();

    LogMessage("---- UNIT TEST PASSED ----\n");
}

static const char *sTokenizeTestStrings[] =
{
    "\"Cory Virok\", \"pizza\"",
    "\"Alice Cooper\", \"123-666-0101\", 21, 102, \"female\"",
    "\"Alice Cooper, Jr.\", \"123-666-0101\", 21, 102, \"female\"",
    "\"Bo Jangles\", \"\", 11, 0, \"male\"",
    "nearby_gender distance=20 gender=\"male\"",
    "targeted_likes distance=5 x=27 y=127 like=\"pizza\"",
    "foo\n\nbar\n",
    "\"Test string\", \t5,,   2,   \"Funky string with a , and a \\\"\"",
    ""
};

static bool RunTokenizeUnitTest()
{
    vector<string> tokenList;

    int count = 0;
    string str = sTokenizeTestStrings[count];
    while (!str.empty())
    {
        Util::TokenizeString(str, tokenList, " ,=\t\n", "=");

        LogMessage("Tokenizing string: '%s'\n", str.c_str());
        for (int i = 0; i < tokenList.size(); i++)
        {
            string token = tokenList[i];
            LogMessage("\t'%s'\n", token.c_str());
        }

        LogMessage("\n");
        tokenList.clear();
        str = sTokenizeTestStrings[++count];
    }

    return true;
}
