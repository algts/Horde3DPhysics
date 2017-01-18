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

#include "Horde3DPhysics.h"
#include "egPhysics.h"
#include <Horde3D/Horde3D.h>

namespace Horde3DPhysics
{

	HORDEPHYSICS_API void initPhysics()
	{
		Physics::instance();
	}

	HORDEPHYSICS_API void releasePhysics()
	{
		Physics::release();
	}

	HORDEPHYSICS_API void updatePhysics()
	{
		Physics::instance()->render();
	}

	HORDEPHYSICS_API void reset()
	{
		Physics::instance()->reset();
	}

	HORDEPHYSICS_API void createPhysicsNode( const char* xmlData, int hordeID )
	{
		Physics::createPhysicsNode( hordeID, xmlData );
	}

	HORDEPHYSICS_API void removePhysicsNode( int hordeID )
	{
		Physics::removePhysicsNode( hordeID );
	}
}