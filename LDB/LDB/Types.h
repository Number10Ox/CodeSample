//
//  Types.h
//  Jon Edwards Code Sample
//
//  Like Database utility declarations and functions 
// 
//  Created by Jon Edwards on 12/3/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#ifndef LDB_TYPES_H
#define LDB_TYPES_H

namespace LDB 
{
	typedef int32_t LocCoord;
	typedef uint64_t HashKey;

	struct Vector
	{
		float x;
		float y;
		float z;
	};

	struct BoundBox
	{
		Vector min;
		Vector max;
	};
}

#endif // LDB_TYPES_H
