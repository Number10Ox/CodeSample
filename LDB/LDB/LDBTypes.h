//
//  LDBTypes.h
//  Jon Edwards Code Sample
//
//  Like Database utility declarations and functions 
// 
//  Created by Jon Edwards on 12/3/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.
//

#ifndef LDBTYPES_H
#define LDBTYPES_H

namespace LDB 
{
	typedef int32_t LDBLocCoord;
	typedef int64_t LDBHashKey;

	struct LDBVector
	{
		float x;
		float y;
		float z;
	};

	struct LDBBoundBox
	{
		LDBVector min;
		LDBVector max;
	};
}

#endif // LDBTYPES_H
