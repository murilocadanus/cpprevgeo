#include "RevGeoApp.hpp"
#include "location/libGeoWeb.hpp"
#include "Configuration.hpp"

#define REVGEO_TAG "[RevGeo] "

const BSONObj RevGeoApp::kQueryGet = BSON("$and" << BSON_ARRAY(
										OR(	BSON("endereco" << ""),
											BSON("endereco" << "null"),
											BSON("endereco" << BSON(std::string("$type") << jstNULL))) <<
										OR(
											BSON("coordenadas.coordinates.0" << BSON("$lt" << 0)),
											BSON("coordenadas.coordinates.1" << BSON("$lt" << 0)))
									));

RevGeoApp::RevGeoApp()
{
}

RevGeoApp::~RevGeoApp()
{
}

bool RevGeoApp::Initialize()
{
	Info(REVGEO_TAG "Initializing...");

	// Init mongo client
	mongo::client::initialize();

	return true;
}

bool RevGeoApp::Process()
{
	Info(REVGEO_TAG "Processing...");
	LibGeoWeb geoWeb;

	try
	{
		// Get total position
		int countToUpdate = this->GetCountPosition();
		Info(REVGEO_TAG "Total data gathered from position collection: %d", countToUpdate);

		// Get position
		ScopedDbConnection conn(pConfiguration->GetMongoDBHost());
		std::auto_ptr<DBClientCursor> cursor = conn->query(pConfiguration->GetMongoDBCollection(), kQueryGet);

		// Iterate all position to update
		while (cursor->more())
		{
			BSONObj query_res = cursor->next();

			// Get mongodb attributes to use as entry data
			int veiculo = query_res.getField("veiculo").Int();
			double lon = query_res.getFieldDotted("coordenadas.coordinates.0").Double();
			double lat = query_res.getFieldDotted("coordenadas.coordinates.1").Double();

			// Init the struct with empty values to storage callback values
			endereco_posicao_mapa data = (const struct endereco_posicao_mapa){ NULL };

			Info(REVGEO_TAG "--------------------");
			Info(REVGEO_TAG "OID: %d", query_res.getField("_id"));
			Info(REVGEO_TAG "Veiculo: %d", veiculo);

			// Call a WS for reverse geolocation
			if(geoWeb.revGeoWeb(lat, lon, &data))
			{
				Info(REVGEO_TAG "Rua: %s", data.rua);
				Info(REVGEO_TAG "Bairro: %s", data.bairro);
				Info(REVGEO_TAG "Municipio: %s", data.municipio);
				Info(REVGEO_TAG "Uf: %s", data.uf);
				Info(REVGEO_TAG "Numero: %s", data.numero);
				Info(REVGEO_TAG "Pais: %s", data.pais);

				// Update mongodb position data
				this->UpdatePosition(&data, veiculo);
			}
			else
			{
				Info(REVGEO_TAG "Can't get JSON data");
				continue;
			}

			Info(REVGEO_TAG "--------------------");
		}
		conn.done();

		return EXIT_SUCCESS;
	}
	catch(const mongo::DBException &e)
	{
		Error(REVGEO_TAG "Failed to connect to mongodb: %s", e.what());
		return EXIT_FAILURE;
	}
}

bool RevGeoApp::Shutdown()
{
	Info(REVGEO_TAG "Shutting down...");
	return true;
}

int RevGeoApp::GetCountPosition()
{
	ScopedDbConnection conn(pConfiguration->GetMongoDBHost());
	int totalPosition = conn->query(pConfiguration->GetMongoDBCollection(), kQueryGet).get()->itcount();
	conn.done();

	return totalPosition;
}

void RevGeoApp::UpdatePosition(struct endereco_posicao_mapa *data, int veiculoId)
{
	ScopedDbConnection conn(pConfiguration->GetMongoDBHost());
	Query query = QUERY("veiculo" << veiculoId);

	// Verify data gathered from WS
	std::string numero = ((data->numero != NULL) && (data->numero[0] != '\0')) ? data->numero : "0";
	std::string velocidadeVia = "";
	std::string endereco = ((data->rua != NULL) && (data->rua[0] != '\0')) ? data->rua : "";
	std::string distancia = "";
	std::string bairro = ((data->bairro != NULL) && (data->bairro[0] != '\0')) ? data->bairro : "";
	std::string municipio = ((data->municipio != NULL) && (data->municipio[0] != '\0')) ? data->municipio : "";
	std::string uf = ((data->uf != NULL) && (data->uf[0] != '\0')) ? data->uf : "";
	std::string pais = ((data->pais != NULL) && (data->pais[0] != '\0')) ? data->pais : "";

	// Create update query
	BSONObj querySet = BSON("$set" << BSON(
										"numero"		<< numero	<< "velocidadeVia"	<< velocidadeVia <<
										"endereco"		<< endereco	<< "distancia"		<< distancia <<
										"bairro"		<< bairro	<< "municipio"		<< municipio <<
										"estado"		<< uf		<< "pais"			<< pais
										)
							);

	Info(REVGEO_TAG "Update querySet: %s", querySet.toString().c_str());

	// Updating based on vehicle id with multiple parameter
	conn->update(pConfiguration->GetMongoDBCollection(), query, querySet, false, true);
	conn.done();
}
