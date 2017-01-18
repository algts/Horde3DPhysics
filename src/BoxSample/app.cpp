// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Application
// ---------------------------------
// Copyright (C) 2006 Nicolas Schulz
//
//
// This sample source file is not covered by the LGPL as the rest of the SDK
// and may be used without any restrictions
//
// *************************************************************************************************

#include "app.h"
#include <Horde3D/Horde3D.h>
#include <Horde3D/Horde3DUtils.h>
#include <Horde3DPhysics.h>
#include <math.h>
#include <iomanip>

using namespace std;


// Convert from degrees to radians
inline float degToRad( float f ) 
{
	return f * (3.1415926f / 180.0f);
}


Application::Application(const string &contentDir )
{
	for( unsigned int i = 0; i < 256; ++i ) _keys[i] = false;

	_x = -30; _y = 36.5f; _z = 4.63f; _rx = -44.69f; _ry = -86.70f; _velocity = 0.3f;
	_curFPS = 30; _timer = 0;
	_freeze = true; _showFPS = false; _debugViewMode = false; _wireframeMode = false;
	
	_content = contentDir;

}


bool Application::init()
{	
	// Initialize engine
	if( !h3dInit( H3DRenderDevice::OpenGL2 ) )
	{	
		h3dutDumpMessages();
		return false;
	}

	Horde3DPhysics::initPhysics();

	// Set options
	h3dSetOption( H3DOptions::LoadTextures, 1 );
	h3dSetOption( H3DOptions::TexCompression, 0 );
	h3dSetOption( H3DOptions::MaxAnisotropy, 8 );
	h3dSetOption( H3DOptions::ShadowMapSize, 2048 );
	
	// Add resources
	// Font
	_fontMatRes = h3dAddResource( H3DResTypes::Material, "overlays/font.material.xml", 0 );
	_panelMatRes = h3dAddResource( H3DResTypes::Material, "overlays/panel.material.xml", 0 );
	_logoMatRes = h3dAddResource( H3DResTypes::Material, "overlays/logo.material.xml", 0 );

	// Skybox
	H3DRes skyBoxRes = h3dAddResource( H3DResTypes::SceneGraph, "models/skybox.scene.xml", 0 );
	// Model
	H3DRes physicsDemoRes = h3dAddResource( H3DResTypes::SceneGraph, "models/domino.scene.xml", 0 );

	H3DRes forwardPipeRes = h3dAddResource( H3DResTypes::Pipeline, "pipelines/forward.pipeline.xml", 0 );

	// Load resources
	h3dutLoadResourcesFromDisk( _content.c_str() );

	// Add scene nodes
	// Add camera
	_cam = h3dAddCameraNode( H3DRootNode, "Camera", forwardPipeRes );
	// Add skybox
	H3DNode sky = h3dAddNodes( H3DRootNode, skyBoxRes );
	h3dSetNodeTransform( sky, 0, 0, 0, 0, 0, 0, 500, 500, 500 );
	// Add pyhsics demo
	h3dAddNodes( H3DRootNode, physicsDemoRes );
	// Add light source
	H3DNode light = h3dAddLightNode( H3DRootNode, "Light1", 0, "LIGHTING", "SHADOWMAP" );
	h3dSetNodeTransform( light, 0, 90, -25, -120, 0, 0, 1, 1, 1 );
	h3dSetNodeParamF( light, H3DLight::RadiusF, 0, 120 );
	h3dSetNodeParamF( light, H3DLight::FovF, 0, 110 );
	h3dSetNodeParamI( light, H3DLight::ShadowMapCountI, 3 );
	h3dSetNodeParamF( light, H3DLight::ShadowSplitLambdaF, 0, 0.9f );
	h3dSetNodeParamF( light, H3DLight::ShadowMapBiasF, 0, 0.01f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 0, 0.9f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 1, 0.7f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 2, 0.75f );

	// Add light source
	light = h3dAddLightNode( H3DRootNode, "Light2", 0, "LIGHTING", "SHADOWMAP" );
	h3dSetNodeTransform( light, 0, 90, 30, -60, 0, 0, 1, 1, 1 );
	h3dSetNodeParamF( light, H3DLight::RadiusF, 0, 120 );
	h3dSetNodeParamF( light, H3DLight::FovF, 0, 110 );
	h3dSetNodeParamI( light, H3DLight::ShadowMapCountI, 3 );
	h3dSetNodeParamF( light, H3DLight::ShadowMapBiasF, 0, 0.01f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 0, 0.9f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 1, 0.7f );
	h3dSetNodeParamF( light, H3DLight::ColorF3, 2, 0.75f );
	
	// Init attachments
	int nodes = h3dFindNodes( H3DRootNode, "", H3DNodeTypes::Undefined );
	for (int i = 0; i < nodes; ++i)
	{
		H3DNode node = h3dGetNodeFindResult(i);
		Horde3DPhysics::createPhysicsNode( h3dGetNodeParamStr(node, 2), node );
	}

	return true;
}


void Application::mainLoop( float fps )
{
	_curFPS = fps;
	_timer += 1 / fps;

	keyHandler();

	// Set camera parameters
	h3dSetNodeTransform( _cam, _x, _y, _z, _rx ,_ry, 0, 1, 1, 1 );
	
	if( _showFPS )
	{
		// Avoid updating FPS text every frame to make it readable
		if( _timer > 0.3f )
		{
			_fpsText.str( "" );
			_fpsText << "FPS: " << fixed << setprecision( 2 ) << _curFPS;
			_timer = 0;
		}
		// Show text
		h3dutShowText( _fpsText.str().c_str(), 0, 0.01f, 0.03f, 1, 1, 1, _fontMatRes );
	}

	// Show logo
	//h3dShowOverlay( 0.75f, 0, 0, 0, 1, 0, 1, 0,
	//					  1, 0.2f, 1, 1, 0.75f, 0.2f, 0, 1,
	//					  7, _logoMatRes );
	h3dutShowText( "Press space to start / reset", 0.0f,0.95f, 0.03f, 1, 1, 1, _fontMatRes );
	
	// Render scene
	h3dRender( _cam );

	if( !_freeze )
	{
		Horde3DPhysics::updatePhysics();
	}
	else
	{
		Horde3DPhysics::reset();
		Horde3DPhysics::updatePhysics();
	}

	// Finish rendering of frame
	h3dFinalizeFrame();
	// Remove all overlays
	h3dClearOverlays();
	
	// Write all mesages to log file
	h3dutDumpMessages();
}


void Application::release()
{
	// Release Physics
	Horde3DPhysics::releasePhysics();
	// Release engine
		h3dRelease();
}


void Application::resize( int width, int height )
{
	// Resize viewport
	h3dSetNodeParamI(_cam, H3DCamera::ViewportXI, 0);
	h3dSetNodeParamI(_cam, H3DCamera::ViewportYI, 0);
	h3dSetNodeParamI(_cam, H3DCamera::ViewportWidthI, width);
	h3dSetNodeParamI(_cam, H3DCamera::ViewportHeightI, height);

	// Set virtual camera parameters
	h3dSetupCameraView(_cam, 45.0f, (float)width / height, 0.1f, 1000.0f );
	h3dResizePipelineBuffers( h3dFindResource( H3DResTypes::Pipeline, "pipelines/forward.pipeline.xml" ), width, height);
}

void Application::keyPressEvent(int key)
{
	if( key == 32 )		// Space
		_freeze = !_freeze;

	if( key == 296 )	// F7
	{
		_debugViewMode = !_debugViewMode;
		h3dSetOption( H3DOptions::DebugViewMode, _debugViewMode ? 1.0f : 0.0f );
	}
	if( key == 297 )	// F8
	{
		_wireframeMode = !_wireframeMode;
		h3dSetOption( H3DOptions::WireframeMode, _wireframeMode ? 1.0f : 0.0f );
	}

	if( key == 298 )	// F9
		_showFPS = !_showFPS;

}

void Application::keyHandler()
{
	float curVel = _velocity * (30 / _curFPS);
	
	if( _keys[87] )	// W
	{
		// Move forward
		_x -= sinf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
		_y -= sinf( -degToRad( _rx ) ) * curVel;
		_z -= cosf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
	}

	if( _keys[83] )	// S
	{
		// Move backward
		_x += sinf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
		_y += sinf( -degToRad( _rx ) ) * curVel;
		_z += cosf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
	}

	if( _keys[65] )	// A
	{
		// Strafe left
		_x += sinf( degToRad( _ry - 90) ) * curVel;
		_z += cosf( degToRad( _ry - 90 ) ) * curVel;
	}

	if( _keys[68] )	// D
	{
		// Strafe right
		_x += sinf( degToRad( _ry + 90 ) ) * curVel;
		_z += cosf( degToRad( _ry + 90 ) ) * curVel;
	}
}


void Application::mouseMoveEvent( float dX, float dY )
{
	// Look left/right
	_ry -= dX / 100 * 30;
	
	// Loop up/down but only in a limited range
	_rx += dY / 100 * 30;
	if( _rx > 90 ) _rx = 90; 
	if( _rx < -90 ) _rx = -90;
}