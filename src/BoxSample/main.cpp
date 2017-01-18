// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Application
// --------------------------------------
// Copyright (C) 2006-2008 Nicolas Schulz
//
//
// This sample source file is not covered by the LGPL as the rest of the SDK
// and may be used without any restrictions
//
// *************************************************************************************************

#include <iostream>
#include "glfw3.h"
#include "app.h"

//#include <iostream>
//#include <cstdlib>
//#include "glfw.h"
//#include "app.h"


// Configuration
const char *caption = "Horde3D Bullet Physics Sample";
const int appWidth = 1280;
const int appHeight = 720;
static bool fullScreen = false;


bool setupWindow( int, int, bool );
static bool running;
static double time0;
static int mx0, my0;
static Application *app;
static GLFWwindow* window;


string generatePath(char p[], const string& file) {
#ifdef __APPLE__
   string s(p);
   for (int i = 0; i < 4; i++)
      s = s.substr(0, s.rfind("/"));   
   return s + "/" + file;
#else
    const string s( p );
    if ( s.find("/") != string::npos) {
        return s.substr( 0, s.rfind( "/" ) ) + "/" + file;
    } else if ( s.find("\\") != string::npos ) {
        return s.substr( 0, s.rfind( "\\" ) ) + "\\" + file;
    } else {
        return file;
    }
#endif
}


void windowCloseListener( GLFWwindow *win )
{
	running = false;
}


void keyPressListener( GLFWwindow *win, int key, int scancode, int action, int mods )
{
	if( !running ) return;

	if( action == GLFW_PRESS )
	{
		int width = appWidth, height = appHeight;
		
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			running = false;
			break;
		case GLFW_KEY_SPACE:
			app->keyPressEvent( key );
			break;
		case GLFW_KEY_F1:
			app->release();
			glfwDestroyWindow( window );
			
			// Toggle fullscreen mode
			fullScreen = !fullScreen;

			if( fullScreen )
			{
				const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
				
				float aspect = mode->width / (float)mode->height;
// 				if( (int)(aspect * 100) == 133 || (int)(aspect * 100) == 125 )	// Standard
// 				{
// 					width = 1280; height = 1024;
// 				}
// 				else if( (int)(aspect * 100) == 160 )							// Widescreen
// 				{
// 					width = 1280; height = 800;
// 				}
// 				else															// Unknown
				{
					// Use desktop resolution
					width = mode->width; height = mode->height;
				}
			}
			
			if( !setupWindow( width, height, fullScreen ) )
			{
				glfwTerminate();
				exit( -1 );
			}
			
			app->init();
			app->resize( width, height );
			time0 = glfwGetTime();
			break;
		default:
			app->keyPressEvent( key );
			break;
		}
	}

	if( key >= 0 ) app->keyStateChange( key, action == GLFW_PRESS );
}


void mouseMoveListener( GLFWwindow *win, double x, double y )
{
	if( !running )
	{
		mx0 = x; my0 = y;
		return;
	}

	app->mouseMoveEvent( (float)(x - mx0), (float)(my0 - y) );
	mx0 = x; my0 = y;
}


void windowResizeListener(GLFWwindow* win, int width, int height)
{
	if (app)
		app->resize( width, height );
}


bool setupWindow( int width, int height, bool fullscreen )
{
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_SAMPLES, 0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	// Create OpenGL window
	if ( fullscreen ) {
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		window = glfwCreateWindow(mode->width, mode->height, caption, glfwGetPrimaryMonitor(), NULL);
	}
	else {
		window = glfwCreateWindow( width, height, caption, NULL, NULL);
	}

	if ( window == nullptr )
	{
		// Fake message box
		glfwDestroyWindow( window );

		window = glfwCreateWindow(800, 50, "Unable to initialize engine - Make sure you have an OpenGL 2.0 compatible graphics card", NULL, NULL);
		double startTime = glfwGetTime();
		while (glfwGetTime() - startTime < 5.0) { /* Sleep */ }

		std::cout << "Unable to initialize window" << std::endl;
		std::cout << "Make sure you have an OpenGL 2.0 compatible graphics card" << std::endl;

		return false;
	}

	if( !fullscreen ) glfwSetWindowTitle( window, caption );
	
	// Disable vertical synchronization
	glfwSwapInterval( 0 );

	glfwMakeContextCurrent( window );
	glfwSetInputMode( window, GLFW_STICKY_KEYS,  true );

	// Disable vertical synchronization
	glfwSwapInterval(0);

	// Set listeners
	glfwSetWindowSizeCallback( window, windowResizeListener);
	glfwSetKeyCallback( window, keyPressListener);
	glfwSetCursorPosCallback( window, mouseMoveListener);
//	glfwSetCursorEnterCallback( window, mouseEnterListener);
	glfwSetWindowCloseCallback( window, windowCloseListener );
	
	return true;
}


int main( int argc, char** argv )
{
	// Initialize GLFW
	glfwInit();
	if( !setupWindow( appWidth, appHeight, fullScreen ) ) return -1;
	
	// Initalize application and engine
	app = new Application( generatePath( argv[0], "/Content" ) );
	if ( !app->init() )
	{
		// Fake message box
		glfwDestroyWindow(window);

		window = glfwCreateWindow(800, 50, "Unable to initialize engine - Make sure you have an OpenGL 2.0 compatible graphics card", NULL, NULL);
		double startTime = glfwGetTime();
		while (glfwGetTime() - startTime < 5.0) { /* Sleep */ }

		std::cout << "Unable to initialize engine" << std::endl;
		std::cout << "Make sure you have an OpenGL 2.0 compatible graphics card" << std::endl;

		glfwTerminate();
		return -1;
	}
	app->resize( appWidth, appHeight );

	glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

	int frames = 0;
	float fps = 30.0f;
	time0 = glfwGetTime();
	running = true;

	// Game loop
	while( running )
	{
		glfwPollEvents();

		// Update key states
		for (int i = 0; i < 320; ++i)
			app->setKeyState(i, glfwGetKey( window, i ) == GLFW_PRESS);

		// Calc FPS
		++frames;
		if( frames >= 3 )
		{
			double t = glfwGetTime();
			fps = frames / (float)(t - time0);
			frames = 0;
			time0 = t;
		}

		// Render
		app->mainLoop( fps );
		glfwSwapBuffers( window );
	}

	glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );

	// Quit
	app->release();
	delete app;
	glfwTerminate();

	return 0;
}
