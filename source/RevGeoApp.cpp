#include "RevGeoApp.hpp"

RevGeoApp::RevGeoApp()
{

}

RevGeoApp::~RevGeoApp()
{

}

bool RevGeoApp::Initialize()
{
	return IApp::Initialize();
}

bool RevGeoApp::Update(f32 dt)
{
	Log("Test");
	return true;
}

bool RevGeoApp::Shutdown()
{
	return IApp::Shutdown();
}
