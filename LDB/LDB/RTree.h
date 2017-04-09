//
//  RTree.h
//  Jon Edwards Code Sample
//
// 	R-Tree implementation for spatial sorting. This implementation is
//  based on the  original paper by Antonin Guttman: R-Trees: A Dynamic
// 	Index Structure for Spatial Searching. 
//
//  This is written this to support 3D queries and not using STL to store
//  nodes so it can be of possible future use. It currently calls 'new'
//  to allocate tree nodes. I can be extended to take custom allocators.
//
// Parameters:
//
//		minBound, maxBound: The minimum and maximum extents of the world
//		along each axis, e.g., (0.0, 0.0, 0.0) to (1.0, 1.0, 1.0) would
//		be a unit-coordinate world space. Do not insert objects outside
//		these bounds.
//	    
//		fillFactor: The minimum number of nodes an r-tree node may contain
//		is fillFactor * nodeCapacity.
//		
//		nodeCapacity: Maximum number of children each r-tree node
//		may contain.
// 
// TODO Custom node allocator
//
//  Created by Jon Edwards on 12/3/13.
//  Copyright (c) 2013 Jon Edwards. All rights reserved.
//

#ifndef LDB_RTREE_H
#define LDB_RTREE_H

#include <vector>
#include "Util.h"

using namespace std;

BEGIN_NAMESPACE(LDB)

// TODO: Use templates to make more flexible
typedef uint64_t RTreeObjectIdType_t;
typedef uint32_t RTreeObjectCategoryType_t;

//-----------------------------------------------------------------------------
// RTree
//-----------------------------------------------------------------------------
class RTree
{
public:
	RTree();
	~RTree();

	void Initialize(float minBound, float maxBound, float fillFactor = 0.60f,
		uint32_t nodeCapacity = 6, uint32_t maxNodeCount = 1024);
	void Shutdown();

	// Insert an element in the Rtree with the specified bounding box, object
	// category, and object id. Object is expected to be unique for the
	// specified category.
	void Insert(const BoundBox &boundingBox, RTreeObjectCategoryType_t category,
		RTreeObjectIdType_t id);

	// Generates a list of object ids for elements in Rtree within the specified bounding
	// box Returns number of elements contained. Categories array specifies the
	// category of each of the ids.
	uint32_t IntersectsQuery(BoundBox &boundingBox,
		vector<RTreeObjectCategoryType_t> &objectCategories, vector<RTreeObjectIdType_t> &objectIds);

	//------------------------------------------------------------------------------
	// Debug routines
	void CheckConsistency();
	uint32_t DebugGetNodeData(BoundBox *boundingBoxes, RTreeObjectCategoryType_t *categories,
		RTreeObjectIdType_t *ids, uint32_t *nodeHeights, uint32_t max);
	
private:
	static const uint32_t kPathBufferLimit = 64;
	static const uint32_t kActiveBranchListSize = 16;

    struct RTreeNode
	{
		BoundBox boundingBox;
		RTreeNode *leftChild;
		RTreeNode *rightSibling;

		RTreeObjectCategoryType_t category;
		RTreeObjectIdType_t id;
	};
	
	struct RTreeBranchListNode
	{
		RTreeNode *node;
		float minDist;
	};

	enum QueryType
	{
		kQueryType_Invalid,
		kQueryType_Intersects
	};

	RTreeNode *ChooseLeaf(RTreeNode *node, const BoundBox &boundingBox);
	RTreeNode *FindLeastEnlargement(RTreeNode *node, const BoundBox &boundingBox);
	void AdjustTree(RTreeNode *node, RTreeNode *child);
	void AdjustTree(RTreeNode *node, RTreeNode *child, RTreeNode *splitSibling);
	RTreeNode *SplitNode(RTreeNode *node);
	void PickSeeds(RTreeNode *parent, RTreeNode **first, RTreeNode **second);
	RTreeNode *PickNext(RTreeNode *originalParent,
		BoundBox &group1BoundingBox, BoundBox &group2BoundingBox);

	RTreeNode *FindLeaf(RTreeNode *node, const BoundBox &boundingBox, RTreeObjectCategoryType_t category, RTreeObjectIdType_t id);
	void CondenseTree(RTreeNode *leaf, RTreeNode *orphans);

	uint32_t RangeQuery(QueryType queryType, BoundBox &boundingBox,
                      vector<RTreeObjectCategoryType_t> &categories, vector<RTreeObjectIdType_t> &objectIds);

	RTreeNode *NodeAllocate();
	void NodeDeallocate(RTreeNode *node);
	void NodeInitialize(RTreeNode *node);

	RTreeNode *NodeInsertData(RTreeNode *node, const BoundBox &boundingBox, 
		RTreeObjectCategoryType_t catagory, RTreeObjectIdType_t id);
	bool NodeDeleteData(RTreeNode *node, const BoundBox &boundingBox, 
		RTreeObjectCategoryType_t category, RTreeObjectIdType_t id);

	RTreeNode *NodeGetNthChild(RTreeNode *node, uint32_t n);		// Gets child 0,1,2,...
	uint32_t NodeGetNumChildren(RTreeNode *node);
	void NodeAddChild(RTreeNode *node, RTreeNode *child);
	bool NodeDeleteChild(RTreeNode *node, RTreeNode *child);
	void NodeCalculateBoundingBox(RTreeNode *node);
	void NodeResetBoundingBox(RTreeNode *node);
	bool NodeIsLeaf(RTreeNode *node);
	bool NodeIsData(RTreeNode *node);

	RTreeNode *PathStackGetTop() const;
	bool PathStackIsEmpty() const;
	void PathStackPush(RTreeNode *node);
	void PathStackPop();
	void PathStackPopAll();

	float m_minBound;
	float m_maxBound;
	float m_fillFactor;
	uint32_t m_nodeCapacity;
	uint32_t m_minNodeCount;
	float m_maxVolume;

	RTreeNode *m_root;
	RTreeNode *m_pathStack[kPathBufferLimit];
	uint32_t m_pathStackPtr;
};

END_NAMESPACE(LDB)

#endif // LDB_RTREE_H
