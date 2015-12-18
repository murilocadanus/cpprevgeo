#ifndef REVGEOAPP_HPP
#define REVGEOAPP_HPP

#include "Defines.hpp"

using namespace Sascar;

class RevGeoApp : public IApp
{
	public:
		RevGeoApp();
		virtual ~RevGeoApp();

		virtual bool Initialize();
		virtual bool Update(f32 dt);
		virtual bool Shutdown();
};

#endif // REVGEOAPP_HPP
