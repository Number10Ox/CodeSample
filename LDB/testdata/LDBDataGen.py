#! /usr/bin/python

###################################################################################################
# File:		LDBDataGen.py
# Project:	Like Database
# Created:	Tuesday, 12/10/13 at 11:42 AM
#
#   Created by Jon Edwards on 12/3/13.
#   Copyright (c) 2013 Jon Edwards. All rights reserved.
###################################################################################################

import sys
import os
import getopt
import random

##############################################################################
# ReadNamesFromFile: Returns list of names read from filename
##############################################################################
def ReadNamesFromFile(fileName):
	nameList = []
	try:
		f = open(fileName, "r")
		try:
			nameList = f.readlines()
		finally:
			f.close()
	except:
		print("Error: Could not open or read names file ", fileName)
		raise

	for n in range(0, len(nameList)):	
		nameList[n].strip()

	return nameList

##############################################################################
# Generate User Name List: Reads first and last names files and returns a list
# of <numUserNames> random names by combining names from the first and
# last name lists.
##############################################################################
def GenerateUserNameList(firstNamesFile, lastNamesFile, numUserNames):

	userNamesList = []

	firstNames = ReadNamesFromFile(firstNamesFile)
	lastNames = ReadNamesFromFile(lastNamesFile)

	for n in range(0, numUserNames):	

		firstNameIndex = random.randint(0, len(firstNames) - 1)
		lastNameIndex = random.randint(0, len(lastNames) - 1)

		firstName = firstNames[firstNameIndex].strip()
		lastName = lastNames[lastNameIndex].strip()
		userName = firstName + ' ' + lastName

		# print("Username[{0}] = {1}".format(n, userName))
		userNamesList.append(userName)

	return userNamesList

##############################################################################
# Generate Users Data File
##############################################################################
def GenerateUsersDataFile(usersDataOutputFile, userNamesList, minXRange,
	maxXRange, minYRange, maxYRange):

	try:
		f = open(usersDataOutputFile, "w")
	except:
		print("Error: Could not open or write users data output file ",
			usersDataOutputFile)
		raise

	try:
		for userName in userNamesList:
	
			x = random.randint(minXRange, maxXRange)
			y = random.randint(minYRange, maxYRange)
	
			phoneNumber = "999-999-999"
			gender = random.choice(["male", "female"])
	
			entry = "\"{0}\", \"{1}\", {2}, {3}, \"{4}\"\n".format(userName,
				phoneNumber, x, y, gender)

			f.write(entry)
	finally:
		f.close()


##############################################################################
# Generate Likes Data File
##############################################################################
def GenerateLikesDataFile(likesDataOutputFile, userNamesList, likeNamesList):

	try:
		f = open(likesDataOutputFile, "w")
	except:
		print("Error: Could not open or write likes data output file ",
			likesDataOutputFile)
		raise

	try:
		for userName in userNamesList:

			workingLikeNamesList = likeNamesList
			numLikes = random.randint(0, 3)
			for n in range(0, numLikes - 1):

				likeName = random.choice(workingLikeNamesList).strip()
				workingLikeNamesList = [value for value in workingLikeNamesList if value != likeName]

				entry = "\"{0}\", \"{1}\"\n".format(userName, likeName)
				f.write(entry)
	finally:
		f.close()

##############################################################################
#								Main
##############################################################################
def main(argv):

	firstNamesInputFile = 'raw/CSV_Database_of_First_Names.csv'
	lastNamesInputFile = 'raw/CSV_Database_of_Last_Names.csv'
	likesInputFile = "raw/CSV_Database_of_Likes.csv"

	usersDataOutputFile = 'big_data_users.csv'
	likesDataOutputFile = 'big_data_likes.csv'

	minXRange = 0
	maxXRange = 1000
	minYRange = 0
	maxYRange = 1000

	numUsers = 5000	# Generate <numUsers>

	# Get list of user names
	userNamesList = GenerateUserNameList(firstNamesInputFile,
		lastNamesInputFile, numUsers)

	# Get list of likes
	likeNamesList = ReadNamesFromFile(likesInputFile)

    # Generate users.csv
	GenerateUsersDataFile(usersDataOutputFile, userNamesList,
		minXRange, maxXRange, minYRange, maxYRange)

    # Generate likes.csv
	GenerateLikesDataFile(likesDataOutputFile, userNamesList, likeNamesList)

	return 0

if __name__ == "__main__":
    main(sys.argv)