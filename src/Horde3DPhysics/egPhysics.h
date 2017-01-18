// *************************************************************************************************
//
// Bullet Physics Integration into Horde3D 
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
//
// Updated to Horde3D v1.0 beta4 by Afanasyev Alexei and Giatsintov Alexander
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// *************************************************************************************************

#pragma once

#include <Horde3D/Horde3D.h>

#include <vector>
#include <Bullet/btBulletDynamicsCommon.h>

/// Helper struct for loading collision objects
struct CollisionShape
{
	enum Type {Box, Sphere, Mesh};
	Type type;
	float mass;
	bool kinematic;

	union
	{
		float extents[3];
		float radius;
	};

	CollisionShape() : type(Mesh), mass(0.0f), kinematic(false) {}
};

/**
 * \brief Horde3D Attachement Node for Physics
 */
class PhysicsNode
{
	friend class Physics;

public:
	/** 
	 * Constructor
	 * @param shape information data about the collision shape
	 * @param meshNodeID id of the mesh node needed in case the collision shape is of type mesh
	 */
	PhysicsNode( CollisionShape shape, int hordeID);
	/// Destructor
	virtual ~PhysicsNode();
	
	void reset();

	/**
	 * Will be called by Physics to update the transformation of the node this node is 
	 * attached to. The transformation calculated by the physics engine will be transfered to
	 * this node.
	 */
	void update();

private:
		
	/// Motion state for dynamic objects
	btDefaultMotionState*			m_motionState;
	/// The main rigid body physics object
	btRigidBody*					m_rigidBody;
	/// The collision shape used for the physics engine
	btCollisionShape*				m_collisionShape;
	/// Triangle Collision Mesh, allocated only if the collision shape is of type Mesh
	btTriangleMesh*					m_btTriangleMesh;
	/// needed to avoid strange effects when physics transformation get updated from the outside
	bool							m_selfUpdate;
	/// ID within the Horde3D scenegraph
	int								m_hordeID;
};

/**
 * \brief Class for managing physics world
 * 
 * This class creates the btDynamicsWorld instance that will handle all physic objects
 * It is also responsible for calling the update method of each physics object
 */
class Physics
{

public:
	/**
	 * Returns a singleton instance of the Physics Manager
	 */
	static Physics* instance();

	/**
	 * Deletes the singleton instance of the Physics Manager
	 */
	static void release();	

	/**
	 * Calling this method will update the physic world state and their objects' transformations
	 */
	void render();

	/**
	 * Adds a node to the world
	 * @param node pointer to a phyiscs node
	 */
	void addNode(PhysicsNode* node);

	/**
	 * Removes the node from the world (does not delete the instance)
	 * @param node pointer to the instance that should be removed
	 */
	void removeNode( PhysicsNode* node );


	/**
	 * Resets the world
	 */
	void reset();

	/**
	 * Static function that will be called by the Horde3D engine when a physics attachment has been found
	 * @param hordeID the id of the Horde3D node the attachment node was attached to
	 * @param xmlText code of the attachment node
	 */
	static void createPhysicsNode( int hordeID, const char *xmlText);

	/**
	 * Static function that has to be called when a node with a physics attachment has been removed from the scene graph
	 * @param hordeID the id of the Horde3D node
	 */
	static void removePhysicsNode( int hordeID);

private:
	/// Private constructor (Singleton)
	Physics();
	/// Private destructor 
	~Physics();

private:

	btDynamicsWorld*			m_physicsWorld;
	// Default Collision Configuration (TODO: what can be configured using this variable?)
	btDefaultCollisionConfiguration* m_configuration;
	btCollisionDispatcher*		m_dispatcher;
	btBroadphaseInterface*		m_pairCache;
	btConstraintSolver*			m_constraintSolver;
	btClock*					m_clock;
	std::vector<PhysicsNode*>	m_physicsNodes;
	
	static Physics*				m_instance;
};


