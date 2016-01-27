#ifndef REVGEOAPP_HPP
#define REVGEOAPP_HPP

#include "Defines.hpp"
#include <cstdlib>
#include <iostream>
#include "mongo/client/dbclient.h" // for the driver
#include "api/curl/CurlWrapper.hpp"
#include "revgeo.hpp" // TODO: Change to Entity

using namespace Sascar;
using namespace mongo;

class RevGeoApp : public IApp
{
	public:
		RevGeoApp();
		virtual ~RevGeoApp();

		virtual bool Initialize();
		virtual bool Process() override;
		virtual bool Shutdown() override;

		static const BSONObj kQueryGet;
		char urlWithParams[2048];

	private:
		int GetCountPosition();
		void UpdatePosition(struct endereco_posicao_mapa *data, int veiculoId);
		void Url(const char *message, ...);

		DBClientConnection cDBConnection;
		CurlWrapper cService;

};

#endif // REVGEOAPP_HPP
