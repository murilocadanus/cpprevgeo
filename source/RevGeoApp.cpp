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
	BSONObj query = BSON( "$and" << BSON_ARRAY(
						OR(	BSON("endereco" << ""),
							BSON("endereco" << "null"),
							BSON("endereco" << BSON(std::string("$type") << jstNULL))) <<
						OR(
							BSON("coordenadas.coordinates.0" << BSON("$lt" << 0)),
							BSON("coordenadas.coordinates.1" << BSON("$lt" << 0)))
					));

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

		std::cout << query_res.getField("_id") << std::endl;
		std::cout << "Veiculo: " << veiculo << std::endl;

		endereco_posicao_mapa data = (const struct endereco_posicao_mapa){ NULL };

		if(geoWeb.revGeoWeb(lat, lon, &data))
		{
			std::cout << "Rua: " << data.rua << std::endl;
			std::cout << "Bairro: " << data.bairro << std::endl;
			std::cout << "Municipio: " << data.municipio << std::endl;
			std::cout << "Estado: " << data.uf << std::endl;
			std::cout << "Numero: " << data.numero << std::endl;
			std::cout << "Pais: " << data.pais << std::endl;

			this->UpdatePosition(&data, veiculo);
		}
		else continue;

		std::cout << "---------------------------" << std::endl;
	}
	conn.done();

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
	int totalPosition = conn->query("murilo.posicao", query).get()->itcount();
	conn.done();

	return totalPosition;
}

void RevGeoApp::UpdatePosition(struct endereco_posicao_mapa *data, int veiculoId)
{
	ScopedDbConnection conn("mngdbsascloud.sasweb-fleet.net");
	Query query = QUERY("veiculo" << veiculoId);

	std::stringstream updateSet;

	updateSet << "{ $set: { ";

	std::string numero;
	if((data->numero != NULL) && (data->numero[0] != '\0')) numero = data->numero;
	else numero = "0";
	updateSet << "\"numero\": " << "\"" << numero << "\", ";

	std::string velocidadeVia = "";
	updateSet << "\"velocidadeVia\": " << "\"" << velocidadeVia << "\", ";

	std::string endereco;
	if((data->rua != NULL) && (data->rua[0] != '\0')) endereco = data->rua;
	else endereco = "";
	updateSet << "\"endereco\": " << "\"" << endereco << "\", ";

	std::string distancia = "";
	updateSet << "\"distancia\": " << "\"" << distancia << "\", ";

	std::string bairro;
	if((data->bairro != NULL) && (data->bairro[0] != '\0')) bairro = data->bairro;
	else bairro = "";
	updateSet << "\"bairro\": " << "\"" << bairro << "\", ";

	std::string municipio;
	if((data->municipio != NULL) && (data->municipio[0] != '\0')) municipio = data->municipio;
	else municipio = "";
	updateSet << "\"municipio\": " << "\"" << municipio << "\", ";

	std::string uf;
	if((data->uf != NULL) && (data->uf[0] != '\0')) uf = data->uf;
	else uf = "";
	updateSet << "\"estado\": " << "\"" << data->uf << "\", ";

	std::string pais;
	if((data->pais != NULL) && (data->pais[0] != '\0')) pais = data->pais;
	else pais = "";
	updateSet << "\"pais\": " << "\"" << pais << "\"";

	updateSet << "} }";

	std::cout << updateSet.str() << std::endl;

	BSONObj updatePacket = fromjson(updateSet.str());
	conn->update("murilo.posicao", query, updatePacket, false, true);
}
