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

#pragma once

#include <Horde3D/Horde3D.h>
#include <sstream>
using namespace std;

class Application
{
private:

	bool			_keys[256];
	float			_x, _y, _z, _rx, _ry;	// Viewer position and orientation
	float			_velocity;				// Velocity for movement
	float			_curFPS, _timer;
	stringstream	_fpsText;

	bool			_freeze, _showFPS, _debugViewMode, _wireframeMode;

	// Engine objects
	H3DRes		_fontMatRes, _logoMatRes, _panelMatRes;

	H3DNode		_cam;

	string _content;
	void keyHandler();

public:
	
	Application(const string &contentDir );

	void setKeyState( unsigned char key, bool state ) { _keys[key] = state; }
	
	bool init();
	void mainLoop( float fps );
	void release();
	void resize( int width, int height );

	void keyPressEvent( int key );
	void keyStateChange( int key, bool state ) { if( key >= 0 && key < 320 ) _keys[key] = state; }

	void mouseMoveEvent( float dX, float dY );
};


