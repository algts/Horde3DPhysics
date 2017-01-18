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

#include "egPhysics.h"
//#include <iostream>
#include "Horde3D/Horde3D.h"
// #include <Horde3d/Horde3DTerrain.h>
#include <Horde3D/utMath.h>
#include <algorithm>
#include "utXmlParser.h"

using namespace std;
using namespace Horde3D;

PhysicsNode::PhysicsNode(CollisionShape shape, int hordeID) : 
m_motionState(0), m_collisionShape(0), m_btTriangleMesh(0), m_rigidBody(0), m_selfUpdate(false), m_hordeID(hordeID)
{
	switch (shape.type)
	{
	case CollisionShape::Box: // Bounding Box Shape
		m_collisionShape = new btBoxShape(btVector3(shape.extents[0], shape.extents[1],shape.extents[2]));
		break;
	case CollisionShape::Sphere: // Sphere Shape			
		m_collisionShape = new btSphereShape(shape.radius);	
		break;
	case CollisionShape::Mesh: // Mesh Shape
		{
			H3DRes geoResource = 0;
			int vertexOffset = 0, indexOffset = 0;
			unsigned int numVertices = 0, numTriangleIndices = 0, vertRStart = 0;

			switch(h3dGetNodeType(m_hordeID))
			{
			case H3DNodeTypes::Mesh:
				geoResource = h3dGetNodeParamI(h3dGetNodeParent(m_hordeID), H3DModel::GeoResI);
				numVertices = h3dGetNodeParamI(m_hordeID, H3DMesh::VertREndI) - h3dGetNodeParamI(m_hordeID, H3DMesh::VertRStartI) + 1;
				numTriangleIndices = h3dGetNodeParamI(m_hordeID, H3DMesh::BatchCountI);		
				vertRStart = h3dGetNodeParamI(m_hordeID, H3DMesh::VertRStartI);
				vertexOffset = vertRStart * 3;
				indexOffset = h3dGetNodeParamI(m_hordeID, H3DMesh::BatchStartI);
				break;
			case H3DNodeTypes::Model:
				geoResource = h3dGetNodeParamI(m_hordeID, H3DModel::GeoResI);
				numVertices = h3dGetResParamI(geoResource, 200, m_hordeID,H3DGeoRes::GeoVertexCountI);
				numTriangleIndices = h3dGetResParamI(geoResource, 200, m_hordeID, H3DGeoRes::GeoIndexCountI);		
				break;
// 			case H3DEXT_NodeType_Terrain:
// 				/*  if( m_terrainGeoRes != 0 )
// 				{
// 					h3dRemoveResource( m_terrainGeoRes );
// 					h3dReleaseUnusedResources();
// 					m_terrainGeoRes = 0;
// 				}
// 				*/
// 				geoResource = h3dextCreateTerrainGeoRes( 
// 					m_hordeID, 
// 					h3dGetNodeParamStr( m_hordeID, H3DNodeParams::NameStr ), 
// 					h3dGetNodeParamF( m_hordeID, H3DEXTTerrain::MeshQualityF, 0) );		
// 				numVertices = h3dGetResParamI(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoVertexCountI);
// 				numTriangleIndices = h3dGetResParamI(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoIndexCountI);		
// 				break;
			}

			float* vertexBase = (float*) h3dMapResStream(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoVertPosStream, true, false);
			h3dUnmapResStream(geoResource);
			
			if( vertexBase ) vertexBase += vertexOffset;

			//unsigned int* triangleBase = NULL;//= (char*) h3dMapResStream(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoIndexStream, true, false);

			unsigned int* TriangleBase32 = NULL;
			unsigned short* TriangleBase16 = NULL;

			//Triangle indices, must cope with 16 bit and 32 bit
			if (h3dGetResParamI(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoIndices16I)) 
			{
				unsigned short* tb = (unsigned short*)h3dMapResStream(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoIndexStream, true, false);
				TriangleBase16 = new unsigned short[ numTriangleIndices ];
				memcpy(TriangleBase16, tb+indexOffset, numTriangleIndices * 2);

				h3dUnmapResStream(geoResource);

				delete TriangleBase32;
			} 
			else 
			{
				unsigned int* tb = (unsigned int*)h3dMapResStream(geoResource, H3DGeoRes::GeometryElem, 0, H3DGeoRes::GeoIndexStream, true, false);
				TriangleBase32 = new unsigned int[numTriangleIndices];
				memcpy(TriangleBase32, tb+indexOffset, numTriangleIndices * 4);

				h3dUnmapResStream(geoResource);

				delete TriangleBase16;
			}


			if(	vertexBase && (TriangleBase32 || TriangleBase16))
			//if(	vertexBase && triangleBase)
			{
				// Create new mesh in physics engine
				m_btTriangleMesh = new btTriangleMesh();
				int offset = 3;
				//if (triangleMode == 5) // Triangle Strip
				//	offset = 1;

				// copy mesh from graphics to physics
				bool index16 = false;
				if (TriangleBase16)
					index16 = true;

				for (unsigned int i = 0; i < numTriangleIndices - 2; i+=offset)
					{
						unsigned int index1 = index16 ? (TriangleBase16[i]   - vertRStart) * 3 : (TriangleBase32[i]   - vertRStart) * 3;
						unsigned int index2 = index16 ? (TriangleBase16[i+1] - vertRStart) * 3 : (TriangleBase32[i+1] - vertRStart) * 3;
						unsigned int index3 = index16 ? (TriangleBase16[i+2] - vertRStart) * 3 : (TriangleBase32[i+2] - vertRStart) * 3;

						//unsigned int index1 = ( triangleBase[i]   - vertRStart) * 3;
						//unsigned int index2 = (triangleBase[i+1] - vertRStart) * 3;
						//unsigned int index3 = (triangleBase[i+2] - vertRStart) * 3;
						m_btTriangleMesh->addTriangle(
							btVector3(vertexBase[index1], vertexBase[index1+1], vertexBase[index1+2] ),
							btVector3(vertexBase[index2], vertexBase[index2+1], vertexBase[index2+2] ),
							btVector3(vertexBase[index3], vertexBase[index3+1], vertexBase[index3+2] )
						); 
					}														

				bool useQuantizedAabbCompression = true;														
				if (shape.mass > 0)
					// You can use GImpact or convex decomposition of bullet to handle more complex meshes
					m_collisionShape = new btConvexTriangleMeshShape(m_btTriangleMesh);				
				else // BvhTriangleMesh can be used only for static objects
					m_collisionShape = new btBvhTriangleMeshShape(m_btTriangleMesh,useQuantizedAabbCompression);
			}
			else
			{
				printf("The mesh data for the physics representation couldn't be retrieved\n");
				return;
			}
		}
	}	

	if( m_collisionShape )
	{
		// Create initial transformation without scale
		const float* x = 0;
		h3dGetNodeTransMats(m_hordeID, 0, &x);
		Matrix4f objTrans( x );	
		Vec3f t, r, s;
		objTrans.decompose(t, r, s);

		objTrans.scale( 1.0f / s.x, 1.0f / s.y, 1.0f / s.z );

		btTransform tr;
		tr.setFromOpenGLMatrix( objTrans.x );
		// Set local scaling in collision shape because Bullet does not support scaling in the world transformation matrices
		m_collisionShape->setLocalScaling(btVector3(s.x,s.y,s.z));			
		btVector3 localInertia(0,0,0);
		//rigidbody is dynamic if and only if mass is non zero otherwise static
		if ( shape.mass != 0)
			m_collisionShape->calculateLocalInertia( shape.mass,localInertia );
		if (shape.mass != 0 || shape.kinematic)
			//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
			m_motionState = new btDefaultMotionState(tr);						

		btRigidBody::btRigidBodyConstructionInfo rbInfo( shape.mass,m_motionState,m_collisionShape,localInertia);
		rbInfo.m_startWorldTransform = tr;	

		m_rigidBody = new btRigidBody(rbInfo);
		m_rigidBody->setUserPointer(this);
		m_rigidBody->setDeactivationTime(2.0f);	

		// Add support for collision detection if mass is zero but kinematic is explicitly enabled
		if( shape.kinematic && shape.mass == 0 )
		{
			m_rigidBody->setCollisionFlags(m_rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);	
			m_rigidBody->setActivationState(DISABLE_DEACTIVATION);
		}
	}
}

PhysicsNode::~PhysicsNode()
{
	Physics::instance()->removeNode(this);
	delete m_rigidBody;
	delete m_motionState;
	delete m_collisionShape;
	delete m_btTriangleMesh;
}

void PhysicsNode::reset()
{
	if (m_rigidBody && m_motionState)
	{			 
		m_motionState->setWorldTransform(m_motionState->m_startWorldTrans);
		m_rigidBody->setWorldTransform( m_motionState->m_startWorldTrans );
		m_rigidBody->setInterpolationWorldTransform( m_motionState->m_startWorldTrans );
		if( !m_rigidBody->isStaticObject() )
		{
			m_rigidBody->setLinearVelocity(btVector3(0,0,0));
			m_rigidBody->setAngularVelocity(btVector3(0,0,0));
			m_rigidBody->activate();
		}	
	}	
}

void PhysicsNode::update()
{	
	btTransform transformation;
	m_motionState->getWorldTransform(transformation);

	float x[16];
	transformation.getBasis().scaled(m_collisionShape->getLocalScaling()).getOpenGLSubMatrix(x);
	x[12] = transformation.getOrigin().x();
	x[13] = transformation.getOrigin().y();
	x[14] = transformation.getOrigin().z();
	x[15] = 1.0f;

	const float* parentMat = 0;
	// since the physics transformation is absolute we have to create a relative transformation matrix for Horde3D
	h3dGetNodeTransMats(h3dGetNodeParent(m_hordeID), 0, &parentMat);		
	if (parentMat) h3dSetNodeTransMat(m_hordeID, (Matrix4f(parentMat).inverted() * Matrix4f(x)).x);
	
	h3dCheckNodeTransFlag( m_hordeID, true );

	//if (parentMat) Physics::removePhysicsNode( m_hordeID );

}


Physics* Physics::m_instance = 0;

Physics* Physics::instance()
{
	if (!m_instance)
		m_instance = new Physics();
	return m_instance;
}

void Physics::release()
{
	delete m_instance;
	m_instance = 0;
}

Physics::Physics()
{
	m_clock = new btClock();
	m_configuration = new btDefaultCollisionConfiguration();
	m_dispatcher = new btCollisionDispatcher(m_configuration);
	btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);
	m_pairCache = new btAxisSweep3(worldMin,worldMax);
	m_constraintSolver = new btSequentialImpulseConstraintSolver();
	m_physicsWorld = new btDiscreteDynamicsWorld(m_dispatcher,m_pairCache,m_constraintSolver, m_configuration);
	m_physicsWorld->setGravity(btVector3(0,-9.81f,0));
}

Physics::~Physics()
{
	delete m_physicsWorld;
	delete m_pairCache;
	delete m_constraintSolver;
	delete m_dispatcher;
	delete m_configuration;
	delete m_clock;
	m_instance = 0;
}

void Physics::reset()
{
	m_clock->reset();
	int numObjects = m_physicsWorld->getNumCollisionObjects();
	for (int i=0;i<numObjects;i++)
	{
		btCollisionObject* colObj = m_physicsWorld->getCollisionObjectArray()[i];
		((PhysicsNode*)((btCollisionObject*) colObj)->getUserPointer())->reset();
	}	
}

void Physics::render()
{
	float dt = m_clock->getTimeMicroseconds() * 0.00001f;
	m_clock->reset();

	m_physicsWorld->stepSimulation(dt);

	vector<PhysicsNode*>::iterator iter = m_physicsNodes.begin();
	while ( iter!= m_physicsNodes.end() )
	{
		(*iter)->update();
		++iter;
		//iter = iter + 1;
	}
}

void Physics::addNode(PhysicsNode* node)
{
	vector<PhysicsNode*>::iterator iter = find(m_physicsNodes.begin(), m_physicsNodes.end(), node);
	if (iter == m_physicsNodes.end())
	{		
		m_physicsWorld->addRigidBody(node->m_rigidBody);
		// add it to the object vector only if it is dynamic
		if (node->m_motionState) m_physicsNodes.push_back(node);
	}
}

void Physics::removeNode(PhysicsNode* node)
{
	// Remove from dynamics physics world
	if (node)
	{
		m_physicsWorld->removeRigidBody(node->m_rigidBody);
		// remove from Physics
		if (node->m_motionState)
		{
			vector<PhysicsNode*>::iterator iter = find(m_physicsNodes.begin(), m_physicsNodes.end(), node);
			if( iter != m_physicsNodes.end() ) m_physicsNodes.erase(iter);
		}
	}
}



void Physics::createPhysicsNode( int hordeID, const char *xmlText)
{
	XMLResults results;
	XMLNode xmlNode = XMLNode::parseString(xmlText, "Attachment", &results);
	if( !xmlNode.isEmpty() && results.error == eXMLErrorNone )
	{		
		XMLNode physicsNode;
		const char* type = xmlNode.getAttribute("type", "");
		if ( _stricmp(type, "GameEngine")==0 && !( physicsNode = xmlNode.getChildNode("BulletPhysics") ).isEmpty() )
		{
			CollisionShape collisionShape;
			const char* shape = physicsNode.getAttribute("shape");
			if (shape && _stricmp(shape, "box")==0)
			{
				collisionShape.type = CollisionShape::Box;
				collisionShape.extents[0] =  static_cast<float>(atof(physicsNode.getAttribute("x")));
				collisionShape.extents[1] =  static_cast<float>(atof(physicsNode.getAttribute("y")));
				collisionShape.extents[2] =  static_cast<float>(atof(physicsNode.getAttribute("z")));
			}
			else if (shape && _stricmp(shape, "sphere")==0)
			{
				collisionShape.type = CollisionShape::Sphere;
				collisionShape.radius =  static_cast<float>(atof(physicsNode.getAttribute("radius")));
			}
			else
				collisionShape.type = CollisionShape::Mesh;
			const char* mass = physicsNode.getAttribute("mass", "0.0");
			collisionShape.mass = static_cast<float>(atof(mass));
			const char* kinematic = physicsNode.getAttribute("kinematic", "false");
			collisionShape.kinematic = _stricmp( kinematic, "true" ) == 0 || _stricmp( kinematic, "1" ) == 0;
			// create new physicsnode: livetime of the node instance will be controlled by the Physics instance
			PhysicsNode* physicsNode = new PhysicsNode(collisionShape, hordeID);
			if (physicsNode->m_rigidBody == 0)
				delete physicsNode;
			else
				instance()->addNode(physicsNode);
		}
	}
#ifdef _DEBUG
	else if( !xmlNode.isEmpty() )
		printf("Error reading XmlNode: %s", XMLNode::getError(results.error));
#endif
}

void Physics::removePhysicsNode( int node )
{
	int numObjects = instance()->m_physicsWorld->getNumCollisionObjects();
	for (int i = 0; i < numObjects; ++i)
	{
		btCollisionObject* colObj = instance()->m_physicsWorld->getCollisionObjectArray()[i];
		PhysicsNode* nodePtr = ((PhysicsNode*)((btCollisionObject*) colObj)->getUserPointer());
		if( nodePtr && nodePtr->m_hordeID == node )
		{
			delete nodePtr;
			break;
		}
	}	
}
