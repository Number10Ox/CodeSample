//
//  LDBTree.cpp
//  Jon Edwards Code Sample
//
// 	R-Tree implementation for spatial sorting. This implementation is
//  based on the  original paper by Antonin Guttman: R-Trees: A Dynamic
// 	Index Structure for Spatial Searching.
//
//  This is written this to support 3D queries and doesn't use STL for
//  storage so it can be of possible future use. It currently calls
// 'new' to allocate tree
// 	nodes.
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
//		maxNodeCount: The maximum number of nodes the R-tree may contain.
//	
//  TODO Custom allocator
//  TODO Contains query
//  TODO Deletion
//
//  Created by Jon Edwards on 12/3/13.
//  Copyright (c) 2017 Jon Edwards. All rights reserved.

#include <math.h>
#include <stdlib.h>

#include "LDBRTree.h"

BEGIN_NAMESPACE(LDB)

namespace RTreeUtil
{
	float GetBoundingBoxVolume(const LDBBoundBox &boundingBox)
	{
		return (boundingBox.max.x - boundingBox.min.x)
			* (boundingBox.max.y - boundingBox.min.y)
			* (boundingBox.max.z - boundingBox.min.z);
	}

	float GetMinDistanceToBoundingBox(const LDBVector &pos, const LDBBoundBox &boundingBox)
	{
		float minDist = 0.0f;
		float r;

		if (pos.x < boundingBox.min.x)
		{
			r = boundingBox.min.x;
		}
		else if (pos.x > boundingBox.max.x)
		{
			r = boundingBox.max.x;
		}
		else
		{
			r = pos.x;
		}

		minDist += pow(fabs(pos.x - r), 2.0f);

		if (pos.y < boundingBox.min.y)
		{
			r = boundingBox.min.y;
		}
		else if (pos.y > boundingBox.max.y)
		{
			r = boundingBox.max.y;
		}
		else
		{
			r = pos.y;
		}

		minDist += pow(fabs(pos.y - r), 2.0f);

		if (pos.z < boundingBox.min.z)
		{
			r = boundingBox.min.z;
		}
		else if (pos.z > boundingBox.max.z)
		{
			r = boundingBox.max.z;
		}
		else
		{
			r = pos.z;
		}

		minDist += pow(fabs(pos.z - r), 2.0f);

		return minDist;
	}
} // namespace RTreeUtil


RTree::RTree() 
	: m_minBound(0.0f), m_maxBound(1.0f), m_fillFactor(0.30f),
	m_nodeCapacity(6), m_minNodeCount(0), m_maxVolume(0.0f), m_root(NULL), 
	m_pathStackPtr(0)
{

}

RTree::~RTree()
{
	Shutdown();
}

void RTree::Initialize(float minBound, float maxBound, float fillFactor,
	uint32_t nodeCapacity, uint32_t maxNodeCount)
{
	m_minBound = minBound;
	m_maxBound = maxBound;
	m_fillFactor = fillFactor;
	m_nodeCapacity = nodeCapacity;
	m_minNodeCount = (uint32_t)(nodeCapacity * fillFactor);
	m_maxVolume = ((m_maxBound - m_minBound) * (m_maxBound - m_minBound) 
		* (m_maxBound - m_minBound));

	// Add a root node that encompasses the entire bounds
	m_root = NodeAllocate();
	NodeInitialize(m_root);
	m_root->boundingBox.min.x = minBound;
	m_root->boundingBox.min.y = minBound;
	m_root->boundingBox.min.z = minBound;
	m_root->boundingBox.max.x = maxBound;
	m_root->boundingBox.max.y = maxBound;
	m_root->boundingBox.max.z = maxBound;
}

void RTree::Shutdown()
{
    if (m_root != NULL)
    {
        NodeDeallocate(m_root);
    }
	m_root = NULL;
}

//---------------------------- QUERY ROUTINES -----------------------------------
//
//-------------------------------------------------------------------------------

uint32_t RTree::IntersectsQuery(LDBBoundBox &boundingBox, 
    vector<RTreeObjectCategoryType_t> &objectCategories,
	vector<RTreeObjectIdType_t> &objectIds)
{
	return RangeQuery(kQueryType_Intersects, boundingBox, objectCategories, objectIds);
}

uint32_t RTree::RangeQuery(QueryType queryType, LDBBoundBox &boundingBox,
	vector<RTreeObjectCategoryType_t> &objectCategories,
	vector<RTreeObjectIdType_t> &objectIds)
{
	ASSERT(queryType == kQueryType_Intersects, "Only Intersects queries are currently supported by Rtree");

	// Traverse the tree, examining branches that intersect/contain the/ bounding
	// box. Add the data children to the types/ids arrays up to the specified
	// maximum.

	uint32_t count = 0;

	PathStackPopAll();

	if (NodeGetNumChildren(m_root) > 0)
	{
		PathStackPush(m_root);
	}

	while (!PathStackIsEmpty())
	{
		RTreeNode *top = PathStackGetTop();
		PathStackPop();
	
		if (!NodeIsData(top))
		{
			// Index nodes: push the children on
			RTreeNode *child = top->leftChild;
			while (child != NULL)
			{
				bool inRange = inRange = LDBUtil::BBoxIntersectsBBox(child->boundingBox, boundingBox);
				if (inRange)
				{
					PathStackPush(child);
				}

				child = child->rightSibling;
			}
		}
		else
		{
			// Data nodes
			bool inRange = LDBUtil::BBoxIntersectsBBox(top->boundingBox, boundingBox);
			if (inRange)
			{
				objectCategories.push_back(top->category);
				objectIds.push_back(top->id);
				count++;
			}
		}
	}

	return count;
}

//-------------------------- DEBUGGING ROUTINES ---------------------------------
//
//-------------------------------------------------------------------------------

void RTree::CheckConsistency()
{
	PathStackPopAll();

	if (NodeGetNumChildren(m_root) > 0)
	{
		PathStackPush(m_root);
	}

	while (!PathStackIsEmpty())
	{
		RTreeNode *top = PathStackGetTop();
		PathStackPop();

		if (!NodeIsData(top))
		{
			RTreeNode *child = top->leftChild;
			while (child != NULL)
			{
				bool contains =  LDBUtil::BBoxContainsBBox(top->boundingBox,
					child->boundingBox);
				ASSERT(contains, "Consistency check failed");

				PathStackPush(child);
				child = child->rightSibling;
			}
		}
	}
}

uint32_t RTree::DebugGetNodeData(LDBBoundBox *boundingBoxes,
	RTreeObjectCategoryType_t *categories, RTreeObjectIdType_t *ids, uint32_t *nodeHeights, uint32_t max)
{
	PathStackPopAll();

	uint32_t height = 0;
	uint32_t count = 0;
	RTreeNode *node = m_root;

	while (node != NULL)
	{
		if (boundingBoxes != NULL)
		{			
			boundingBoxes[count] = node->boundingBox;
		}
		if (categories != NULL)
		{
			categories[count] = node->category;
		}	
		if (ids != NULL)
		{
			ids[count] = node->id;
		}
		if (nodeHeights != NULL)
		{
			nodeHeights[count] = height;
		}
		
		count++;

		if (node->leftChild != NULL)
		{
			// Store next sibling on the stack. Return to it after all
			// children are processed
			if (node->rightSibling != NULL)
			{
				PathStackPush(node->rightSibling);
			}

			height++;
			node = node->leftChild;
		}
		else
		{
			node = node->rightSibling;
			if (node == NULL && !PathStackIsEmpty())
			{
				node = PathStackGetTop();
				PathStackPop();
				height--;
			}
		}
	}

	return count;
}

//-------------------------- INSERTION ROUTINES ---------------------------------
//
//-------------------------------------------------------------------------------

void RTree::Insert(const LDBBoundBox &boundingBox, RTreeObjectCategoryType_t category, RTreeObjectIdType_t id)
{
	//
	// TBD Perhaps should do a sanity check ASSERT in case objects are
	// added that are bigger than the maximum bounds
	//

	// The stack will have the leaf's parent at the top after ChooseLeaf
	PathStackPopAll();
	RTreeNode *leaf = ChooseLeaf(m_root, boundingBox);
	RTreeNode *newChild = NodeInsertData(leaf, boundingBox, category, id);

	uint32_t numChildren = NodeGetNumChildren(leaf);
	if (numChildren > m_nodeCapacity)
	{
		// We have to split the node if it has too many children. Divide
		// up the node's children between it and a new sibling and then
		// adjust the parent node, passing its new child.
		RTreeNode *parent = PathStackGetTop();
		PathStackPop();
		RTreeNode *splitSibling = SplitNode(leaf);
		AdjustTree(parent, leaf, splitSibling);
	}
	else
	{
		// Adjust the tree from bottom to top, updating bounding boxes
		AdjustTree(leaf, newChild);
	}

	CheckConsistency();
}

RTree::RTreeNode *RTree::ChooseLeaf(RTreeNode *node, const LDBBoundBox &boundingBox)
{
	// Descend down the tree, picking an index node at each level that
	// needs to be enlarged the least to incorporate the new bounding
	// box. Stop when we hit a leaf index node.

	if (NodeIsLeaf(node))
	{
		return node;
	}

	PathStackPush(node);
	RTreeNode *child = FindLeastEnlargement(node, boundingBox);
	RTreeNode *leaf = ChooseLeaf(child, boundingBox);
	return leaf;
}

RTree::RTreeNode *RTree::FindLeastEnlargement(RTreeNode *node, const LDBBoundBox &boundingBox)
{
	RTreeNode *bestNode = NULL;
	float leastEnlargement = m_maxVolume;

	RTreeNode *child = node->leftChild;
	while (child != NULL)
	{
		LDBBoundBox enlargedBoundingBox;
		LDBUtil::BBoxMerge(&enlargedBoundingBox, const_cast<LDBBoundBox *>(&boundingBox),
			const_cast<LDBBoundBox *>(&child->boundingBox));
		
		float childVolume = RTreeUtil::GetBoundingBoxVolume(child->boundingBox);
		float enlargedVolume = RTreeUtil::GetBoundingBoxVolume(enlargedBoundingBox);
		float enlargement = enlargedVolume - childVolume;

		if (enlargement < leastEnlargement)
		{
			leastEnlargement = enlargement;
			bestNode = child;
		}
		else if (enlargement == leastEnlargement)
		{
			// Resolve ties by choosing the entry with the smallest volume
			float bestVolume = RTreeUtil::GetBoundingBoxVolume(bestNode->boundingBox);
			if (childVolume < bestVolume)
			{
				bestNode = child;
			}
		}

		child = child->rightSibling;
	}

	return bestNode;
}

void RTree::AdjustTree(RTreeNode *node, RTreeNode *child)
{
	// If needed, adjust covering rectangle so it tightly encloses all of
	// its children and continue adjusting up the tree.
	if (node != NULL && !LDBUtil::BBoxContainsBBox(node->boundingBox, child->boundingBox))
	{
		NodeCalculateBoundingBox(node);
	}

	if (!PathStackIsEmpty())
	{
		RTreeNode *parent = PathStackGetTop();
		PathStackPop();
		AdjustTree(parent, node);
	}
}

void RTree::AdjustTree(RTreeNode *node, RTreeNode *child, RTreeNode *splitSibling)
{
	// Stop if we've reached the root node. If a split occurred then we
	// need a new root.
	if (node == NULL)
	{
		RTreeNode *newRoot = NodeAllocate();
		NodeInitialize(newRoot);
		newRoot->boundingBox.min.x = m_minBound;
		newRoot->boundingBox.min.y = m_minBound;
		newRoot->boundingBox.min.z = m_minBound;
		newRoot->boundingBox.max.x = m_maxBound;
		newRoot->boundingBox.max.y = m_maxBound;
		newRoot->boundingBox.max.z = m_maxBound;
		newRoot->rightSibling = NULL;
		newRoot->leftChild = child;
		newRoot->leftChild->rightSibling = splitSibling;

		m_root = newRoot;
		return;
	}

	// We've added a child to a node and a split has occurred
	NodeAddChild(node, splitSibling);
	NodeCalculateBoundingBox(node);

	uint32_t numChildren = NodeGetNumChildren(node);
	if (numChildren > m_nodeCapacity)
	{
		RTreeNode *newSplitSibling = SplitNode(node);
		RTreeNode *parent = PathStackGetTop();
		PathStackPop();
		AdjustTree(parent, node, newSplitSibling);	
	}
	else
	{
		RTreeNode *parent = PathStackGetTop();
		PathStackPop();
		AdjustTree(parent, node);	
	}
}

RTree::RTreeNode *RTree::SplitNode(RTreeNode *node)
{
	RTreeNode *newNode = NodeAllocate();
	NodeInitialize(newNode);
	NodeResetBoundingBox(newNode);
	NodeResetBoundingBox(node);

	uint32_t remainingNodes = NodeGetNumChildren(node);

	// We need to divide the node's children into two groups. Pick the
	// first element of each group using PickSeeds.
	RTreeNode *group1Head;
	RTreeNode *group2Head;
	PickSeeds(node, &group1Head, &group2Head);
	NodeDeleteChild(node, group1Head);
	remainingNodes--;
	NodeDeleteChild(node, group2Head);
	remainingNodes--;

	// Initialize group1 and group 2's bounding box's
	uint32_t group1Count = 0;
	uint32_t group2Count = 0;
	LDBBoundBox group1BoundingBox = group1Head->boundingBox;
	LDBBoundBox group2BoundingBox = group2Head->boundingBox;

	while (node->leftChild != NULL)
	{
		if (group1Count + remainingNodes == m_minNodeCount)
		{
			// If group 1 has so few entries that all the rest must be assigned
			// for it to have the minimum, assign them and stop
			RTreeNode *nextNode = node->leftChild;
			while (nextNode != NULL)
			{
				NodeDeleteChild(node, nextNode);
				remainingNodes--;	
				nextNode->rightSibling = group1Head;
				group1Head = nextNode;	
				nextNode = node->leftChild;	
				group1Count++;
			}
		}
		else if (group2Count + remainingNodes == m_minNodeCount)
		{
			// If group 2 has so few entries that all the rest must be assigned
			// for it to have the minimum, assign them and stop
			RTreeNode *nextNode = node->leftChild;
			while (nextNode != NULL)
			{
				NodeDeleteChild(node, nextNode);
				remainingNodes--;	
				nextNode->rightSibling = group2Head;
				group2Head = nextNode;	
				nextNode = node->leftChild;	
				group2Count++;
			}
		}
		else
		{
			// Invoke PickNext() to choose the next entry to assign. Add it to
			// the group whose covering rectangle will have to be enlarged least
			// to accommodate it. Break ties by adding it to the entry with
			// the smaller volume.

			RTreeNode *nextNode = PickNext(node, group1BoundingBox, group2BoundingBox);
			NodeDeleteChild(node, nextNode);
			remainingNodes--;

			float volume1 = RTreeUtil::GetBoundingBoxVolume(group1BoundingBox);
			float volume2 = RTreeUtil::GetBoundingBoxVolume(group2BoundingBox);

			LDBBoundBox merged1;
			LDBUtil::BBoxMerge(&merged1, &group1BoundingBox, &nextNode->boundingBox);
			float merged1Volume = RTreeUtil::GetBoundingBoxVolume(merged1);
			float difference1 = merged1Volume - volume1;
			
			LDBBoundBox merged2;
			LDBUtil::BBoxMerge(&merged2, &group2BoundingBox, &nextNode->boundingBox);
			float merged2Volume = RTreeUtil::GetBoundingBoxVolume(merged2);
			float difference2 =  merged2Volume - volume2;

			if (difference1 < difference2)
			{
				LDBUtil::BBoxMerge(&group1BoundingBox, &group1BoundingBox, &nextNode->boundingBox);
				nextNode->rightSibling = group1Head;
				group1Head = nextNode;
				group1Count++;
			}
			else if (difference1 > difference2)
			{
				LDBUtil::BBoxMerge(&group2BoundingBox, &group2BoundingBox, &nextNode->boundingBox);
				nextNode->rightSibling = group2Head;
				group2Head = nextNode;
				group2Count++;
			}
			else
			{
				if (volume1 <= volume2)
				{
					LDBUtil::BBoxMerge(&group1BoundingBox, &group1BoundingBox, &nextNode->boundingBox);
					nextNode->rightSibling = group1Head;
					group1Head = nextNode;
					group1Count++;
				}
				else
				{
					LDBUtil::BBoxMerge(&group2BoundingBox, &group2BoundingBox, &nextNode->boundingBox);
					nextNode->rightSibling = group2Head;
					group2Head = nextNode;
					group2Count++;
				}
			}
		}
	}

	node->leftChild = group1Head;
	NodeCalculateBoundingBox(node);
	newNode->leftChild = group2Head;
	NodeCalculateBoundingBox(newNode);

	return newNode;
}

void RTree::PickSeeds(RTreeNode *parent, RTreeNode **first, RTreeNode **second)
{
	*first = NULL;
	*second = NULL;

	// Quadratic-Cost Algorithm
	float worstWaste = -m_maxVolume;

	uint32_t numChildren = NodeGetNumChildren(parent);
	for (uint32_t i = 0; i < numChildren; i++)
	{
		for (uint32_t j = 0; j < i; j++)
		{
			RTreeNode *node1 = NodeGetNthChild(parent, i);
			RTreeNode *node2 = NodeGetNthChild(parent, j);
			LDBBoundBox merged;
			LDBUtil::BBoxMerge(&merged, &node1->boundingBox, &node2->boundingBox);
			float mergedVolume = RTreeUtil::GetBoundingBoxVolume(merged);
			float volume1 = RTreeUtil::GetBoundingBoxVolume(node1->boundingBox);
			float volume2 = RTreeUtil::GetBoundingBoxVolume(node2->boundingBox);
			float waste = mergedVolume - volume1 - volume2;
			if (waste >= worstWaste)
			{
				worstWaste = waste;
				*first = node1;
				*second = node2;
			}
		}
	}

	ASSERT(((*first != NULL) && (*second != NULL)), "RTree PickSeeds failed");
}

RTree::RTreeNode *RTree::PickNext(RTreeNode *originalParent, LDBBoundBox &group1BoundingBox,
	LDBBoundBox &group2BoundingBox)
{
	// Quadratic-Cost Algorithm: Find entry with greatest preference 
	// for one group

	float maxDifference = -m_maxVolume;
	RTreeNode *nextNode = NULL;

	RTreeNode *child = originalParent->leftChild;
	while (child != NULL)
	{
		float group1Volume = RTreeUtil::GetBoundingBoxVolume(group1BoundingBox);
		float group2Volume = RTreeUtil::GetBoundingBoxVolume(group2BoundingBox);
		
		LDBBoundBox merged1;
		LDBUtil::BBoxMerge(&merged1, &group1BoundingBox, &child->boundingBox);
		float merged1Volume = RTreeUtil::GetBoundingBoxVolume(merged1);
		LDBBoundBox merged2;
		LDBUtil::BBoxMerge(&merged2, &group2BoundingBox, &child->boundingBox);
		float merged2Volume = RTreeUtil::GetBoundingBoxVolume(merged2);

		float group1VolumeIncrease = merged1Volume - group1Volume;
		float group2VolumeIncrease = merged2Volume - group2Volume;
		
		float difference = group1VolumeIncrease > group2VolumeIncrease
			? group1VolumeIncrease - group2VolumeIncrease
			: group2VolumeIncrease - group1VolumeIncrease;

		if (difference > maxDifference)
		{
			maxDifference = difference;
			nextNode = child;
		}

		child = child->rightSibling;
	}

	return nextNode;
}

//---------------------------- NODE ROUTINES ------------------------------------
// 
//-------------------------------------------------------------------------------

RTree::RTreeNode *RTree::NodeAllocate()
{
	return new RTreeNode;
}

void RTree::NodeDeallocate(RTreeNode *node)
{
	if (node->leftChild != NULL)
	{
		NodeDeallocate(node->leftChild);
	}
	
	if (node->rightSibling != NULL)
	{
		NodeDeallocate(node->rightSibling);
	}

	delete node;
}

void RTree::NodeInitialize(RTreeNode *node)
{
	NodeResetBoundingBox(node);
	node->leftChild = NULL;
	node->rightSibling = NULL;
	node->category = 0;
	node->id = 0;
}

RTree::RTreeNode *RTree::NodeInsertData(RTreeNode *node, const LDBBoundBox &boundingBox,
	RTreeObjectCategoryType_t category, RTreeObjectIdType_t id)
{
	RTreeNode *dataNode = NodeAllocate();
	NodeInitialize(dataNode);
	dataNode->category = category;
	dataNode->id = id;
	dataNode->boundingBox = boundingBox;
	dataNode->leftChild = NULL;
	dataNode->rightSibling = NULL;

	if (node->leftChild != NULL)
	{
		RTreeNode *n = node->leftChild;
		while (n->rightSibling != NULL)
			n = n->rightSibling;
		n->rightSibling = dataNode;
	}
	else
		node->leftChild = dataNode;

	LDBUtil::BBoxMerge(&node->boundingBox, &node->boundingBox, &dataNode->boundingBox);

	return dataNode;
}

bool RTree::NodeDeleteData(RTreeNode *node, const LDBBoundBox &boundingBox,
	RTreeObjectCategoryType_t category, RTreeObjectIdType_t id)
{
	RTreeNode *child = node->leftChild;
	RTreeNode *prevChild = NULL;
	bool found = false;
	while (child != NULL && !found)
	{
		bool inRange = LDBUtil::BBoxContainsBBox(child->boundingBox, boundingBox);
		if (inRange && child->category == category && child->id == id)
		{
			if (prevChild == NULL)
				node->leftChild = child->rightSibling;
			else
				prevChild->rightSibling = child->rightSibling;

			delete child;
			return true;
		}

		prevChild = child;
		child = child->rightSibling;
	}

	return false;
}

void RTree::NodeAddChild(RTreeNode *node, RTreeNode *child)
{
	if (node->leftChild != NULL)
	{
		RTreeNode *n = node->leftChild;
		while (n->rightSibling != NULL)
			n = n->rightSibling;
		n->rightSibling = child;
	}
	else
	{
		node->leftChild = child;
	}
}

bool RTree::NodeDeleteChild(RTreeNode *node, RTreeNode *child)
{
	bool found = false;
	if (node->leftChild == child)
	{
		node->leftChild = child->rightSibling;
		child->rightSibling = NULL;
		found = true;
	}
	else
	{
		for (RTreeNode *n = node->leftChild; n != NULL && !found; n = n->rightSibling)
		{
			if (n->rightSibling == child)
			{
				n->rightSibling = child->rightSibling;
				child->rightSibling = NULL;
				found = true;
			}
		}
	}

	return found;
}

RTree::RTreeNode *RTree::NodeGetNthChild(RTreeNode *node, uint32_t n)
{
	RTreeNode *child = node->leftChild;
	uint32_t count = 0;
	while (count < n && child != NULL)
	{
		child = child->rightSibling;
		count++;
	}

	return child;
}

uint32_t RTree::NodeGetNumChildren(RTreeNode *node)
{
	uint32_t count = 0;
	RTreeNode *child = node->leftChild;
	while (child != NULL)
	{
		child = child->rightSibling;
		count++;
	}

	return count;
}

void RTree::NodeCalculateBoundingBox(RTreeNode *node)
{
	NodeResetBoundingBox(node);
	RTreeNode *n = node->leftChild;
	while (n != NULL)
	{
		LDBUtil::BBoxMerge(&node->boundingBox, &node->boundingBox, &n->boundingBox);
		n = n->rightSibling;
	}
}

void RTree::NodeResetBoundingBox(RTreeNode *node)
{
	node->boundingBox.min.x = m_maxBound;
	node->boundingBox.min.y = m_maxBound;
	node->boundingBox.min.z = m_maxBound;
	node->boundingBox.max.x = m_minBound;
	node->boundingBox.max.y = m_minBound;
	node->boundingBox.max.z = m_minBound;
}

bool RTree::NodeIsLeaf(RTreeNode *node)
{
	if ((node == m_root && node->leftChild == NULL)
		|| NodeIsData(node->leftChild))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool RTree::NodeIsData(RTreeNode *node)
{
	if (node->leftChild == NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//----------------------------- PATH BUFFER -------------------------------------
//  Implements a stack of R-tree nodes. Used for recording a traversal
//  down the tree.
//-------------------------------------------------------------------------------
RTree::RTreeNode *RTree::PathStackGetTop() const
{
	if (m_pathStackPtr > 0)
	{
		RTreeNode *node = m_pathStack[m_pathStackPtr - 1];
		return node;
	}
	else
	{
		return NULL;
	}
}

bool RTree::PathStackIsEmpty() const
{
	return (m_pathStackPtr == 0);
}

void RTree::PathStackPush(RTree::RTreeNode *node)
{	
	ASSERT(m_pathStackPtr < kPathBufferLimit, "Path stack overflow");

	m_pathStack[m_pathStackPtr] = node;
	m_pathStackPtr++;
}

void RTree::PathStackPop()
{
	if (m_pathStackPtr > 0)
	{
		m_pathStackPtr--;
		ASSERT((m_pathStackPtr >= 0), "Path buffer underflow");
		m_pathStack[m_pathStackPtr] = NULL;
	}
}

void RTree::PathStackPopAll()
{	
	while (m_pathStackPtr)
	{
		PathStackPop();
	}
}

END_NAMESPACE(LDB)
