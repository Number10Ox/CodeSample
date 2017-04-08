
Code Test README.txt Notes

EXPLANATION OF SOLUTION

I. Structure of Database

I implemented a solution that specifically stores "UserRecord" C structs.
The database could potentially store other structures, but it is oriented
towards storing structs defined in code as opposed to a data-driven
solution supporting arbitrary schemas.

In the past, I've implemented game object and game data systems that define
and store arbitrary structures. Implementation of such a solution would take
a lot longer and seemed outside the scope of this test.

The database does not support more than one user with a given user name.
As the like data only specifies a user name, there would be no way to
distinguish between two users with the same name so this didn't seem
necessary. The system easily could be extended to use a more complex keys,
e.g., username + phone number + location.

I opted to hash all the strings in the database with a 64-bit CRC hash and
store strings in a string dictionary. Both storage and comparison of strings
is costly. In the unlikely chance of a collsion, a error message will be
generated when two strings with the same hash number are registered.

III. Spatial Search

There were a lot of possible ways to implement the spatial queries, e.g.,
brute force iterating through all the elements to find those within
a distance with a "like" or using quad-trees, kd-trees, etc.

I opted to build a spatial index using an R-tree. I've implemented R-tree
before, and I like it because they are space efficient, don't require
a lot of tuning, and can support quick nearest neighbor queries and
easily support "contains" queries (an object is contained within a region).
Those are beyond the scope of this test, but an R-tree seemed perfectly
functional.

IV. NEARBY_GENDER

The nearby gender query could be implemented a lot of different ways.
A brute force solution comparing all pairs of users in the database
(N^2) is the most straightforward. I opted to treat it as a graph
problem, using a depth-first-search and R-tree queries to find all
matching neighbors in range.

It would be interesting given a large set of real world data to
measure and trying different techniques. If the database were
persistent, it would worthwhile to index the data in different ways.

V. TIME TAKEN

I track the time I spend on tasks using the Pomodoro system, working in
25 minute time blocks. I recorded that I spent 44 Pomodoros on the test.

The first few hours I spent a fair amount of time familiarizing myself
with X-code since I haven't used it before. Generally, I took my time
having fun with it since there was no time limit specified.

EXTRA FEATURES

1. Additional command line arguments. The users data file defaults to
   'users.csv' and 'likes.csv', but you can specify alternate files
   to load instead

	-u [users.csv file]
	-l [likes.csv file]

2. I implemented general support for queries described as strings
   with parameters of the form "variable=value" specified in any
   order. Currently, queries can only be specified on the command
   line, but could be extended to read from stdin, a file, etc.

 3. The QueryNearbyGender could easily be generalized to perform other
 	checks than gender. However, it currently only checks users
 	of <gender> within <distance> of each other.


OTHER NOTES

I haven't made use of features available in C++11 nor have I made use of
templates. I'm familiar with the C++11 features, but I haven't used it in
production yet. As for templates, I tend to start with non-templatized
solutions and apply templatization where it's useful. I have a few TODOs
in the code for areas that might beneift.

