#include "RevGeoApp.hpp"
#include "location/libGeoWeb.hpp"

RevGeoApp::RevGeoApp()
{
	if (this->Initialize())
		this->Process();
}

RevGeoApp::~RevGeoApp()
{
	Shutdown();
}

bool RevGeoApp::Initialize()
{
	mongo::client::initialize();
	return true;
}

bool RevGeoApp::Process()
{
	LibGeoWeb geoWeb;
	Query query = QUERY("endereco" << "null");
	int countToUpdate = this->GetCountPosition(query);
	std::cout << "[DATE] " << "Registros pendentes de atualizacao na collection posicao: " << countToUpdate << std::endl;

	// Get Position
	ScopedDbConnection conn("mngdbsascloud.sasweb-fleet.net");
	std::auto_ptr<DBClientCursor> cursor = conn->query("murilo.posicao", query);

	while (cursor->more())
	{
		BSONObj query_res = cursor->next();
		int veiculo = query_res.getField("veiculo").Int();
		double lon = query_res.getFieldDotted("coordenadas.coordinates.0").Double();
		double lat = query_res.getFieldDotted("coordenadas.coordinates.1").Double();

		endereco_posicao_mapa data;

		if(geoWeb.revGeoWeb(lat, lon, &data))
		{
			std::cout << "Rua: " << data.rua << std::endl;
			std::cout << "Bairro: " << data.bairro << std::endl;
			std::cout << "Municipio: " << data.municipio << std::endl;
			std::cout << "Estado: " << data.uf << std::endl;
			std::cout << "Numero: " << data.numero << std::endl;
			std::cout << "PAIS: " << data.pais << std::endl;

			this->UpdatePosition(&data, veiculo);
		}
		else continue;

		std::cout << "---------------------------" << std::endl;
	}

	/*try {
		mongo::ScopedDbConnection conn("mngdbsascloud.sasweb-fleet.net");

		auto_ptr<DBClientCursor> cursor = conn->query(mongoUltimaPosicao, QUERY("veiculo" << veiculoId));

		std::cout << "connected ok" << std::endl;

		return EXIT_SUCCESS;
	} catch( const mongo::DBException &e ) {
		std::cout << "caught " << e.what() << std::endl;
	}
	return EXIT_SUCCESS;*/
}

bool RevGeoApp::Shutdown()
{
	return true;//IApp::Shutdown();
}

int RevGeoApp::GetCountPosition(Query query)
{
	// $query = array('$and' => array(array('$or' => array(array("endereco" => null),array("endereco" => "null"),array("endereco" => ""))),array('$or' => array(array("coordenadas.coordinates.0" => array('$lt' => 0)),array("coordenadas.coordinates.1" => array('$lt' => 0))))));

	ScopedDbConnection conn("mngdbsascloud.sasweb-fleet.net");
	return conn->query("murilo.posicao", query).get()->itcount();
}

void RevGeoApp::UpdatePosition(struct endereco_posicao_mapa *data, int veiculoId)
{
	ScopedDbConnection conn("mngdbsascloud.sasweb-fleet.net");
	Query query = QUERY("veiculo" << veiculoId);

	char updateSet[1000];

	sprintf(updateSet, "{ $set: { ");

	sprintf(updateSet, "\"endereco\": \"%s\",", data->rua);
	sprintf(updateSet, "\"bairro\": \"%s\",", data->bairro);
	sprintf(updateSet, "\"municipio\": \"%s\",", data->municipio);
	sprintf(updateSet, "\"estado\": \"%s\",", data->uf);
	sprintf(updateSet, "\"numero\": \"%s\",", data->numero);
	sprintf(updateSet, "\"pais\": \"%s\",", data->pais);

	sprintf(updateSet, "} }");

	std::cout << updateSet << std::endl;

	//BSONObj updatePacket = fromjson(updateSet);
	//conn->update("murilo.posicao", query, updatePacket);
}
