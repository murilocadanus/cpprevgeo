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
	Info(REVGEO_TAG "%s - Initializing...", pConfiguration->GetTitle().c_str());

	// Init mongo client
	mongo::client::initialize();

	// Connect to mongo client
	try
	{
		cDBConnection.connect(pConfiguration->GetMongoDBHost());
		Info(REVGEO_TAG "%s - Connected to mongodb", pConfiguration->GetTitle().c_str());
		return true;
	}
	catch(const mongo::DBException &e)
	{
		Error(REVGEO_TAG "%s - Failed to connect to mongodb: %s", pConfiguration->GetTitle().c_str(), e.what());
		return false;
	}
}

bool RevGeoApp::Process()
{
	Info(REVGEO_TAG "%s - Start processing...", pConfiguration->GetTitle().c_str());
	LibGeoWeb geoWeb;

	try
	{
		// Get total position
		int countToUpdate = this->GetCountPosition();
		Info(REVGEO_TAG "%s - Total data gathered from position collection: %d", pConfiguration->GetTitle().c_str(), countToUpdate);

		// Get position
		std::auto_ptr<DBClientCursor> cursor = cDBConnection.query(pConfiguration->GetMongoDBCollection(), kQueryGet, pConfiguration->GetQueryLimit());

		// Iterate all position to update
		while (cursor->more())
		{
			BSONObj query_res = cursor->next();

			if(query_res.getField("veiculo").isNumber())
			{
				// Get mongodb attributes to use as entry data
				int veiculo = query_res.getField("veiculo").Number();

				double lon = query_res.getFieldDotted("coordenadas.coordinates.0").Double();
				double lat = query_res.getFieldDotted("coordenadas.coordinates.1").Double();

				// Init the struct with empty values to storage callback values
				endereco_posicao_mapa data = (const struct endereco_posicao_mapa){ NULL };

				Info(REVGEO_TAG "%s - --------------------", pConfiguration->GetTitle().c_str());
				Info(REVGEO_TAG "%s - OID: %d", pConfiguration->GetTitle().c_str(), query_res.getField("_id"));
				Info(REVGEO_TAG "%s - Veiculo: %d", pConfiguration->GetTitle().c_str(), veiculo);

				// Create the URL
				geoWeb.Url(pConfiguration->GetServiceURL().c_str(), lat, lon, pConfiguration->GetServiceKey().c_str());

				// Call a WS for reverse geolocation
				if(geoWeb.revGeoWeb(&data, pConfiguration->GetServiceTimeOut()))
				{
					Info(REVGEO_TAG "%s - Rua: %s", pConfiguration->GetTitle().c_str(), data.rua);
					Info(REVGEO_TAG "%s - Bairro: %s", pConfiguration->GetTitle().c_str(), data.bairro);
					Info(REVGEO_TAG "%s - Municipio: %s", pConfiguration->GetTitle().c_str(), data.municipio);
					Info(REVGEO_TAG "%s - Uf: %s", pConfiguration->GetTitle().c_str(), data.uf);
					Info(REVGEO_TAG "%s - Numero: %s", pConfiguration->GetTitle().c_str(), data.numero);
					Info(REVGEO_TAG "%s - Pais: %s", pConfiguration->GetTitle().c_str(), data.pais);

					// Update mongodb position data
					this->UpdatePosition(&data, veiculo);
				}
				else
				{
					Info(REVGEO_TAG "%s - Can't get JSON data", pConfiguration->GetTitle().c_str());
					continue;
				}

				Info(REVGEO_TAG "%s - --------------------", pConfiguration->GetTitle().c_str());
			}
			else
			{
				Info(REVGEO_TAG "%s - Invalid veiculo field type", pConfiguration->GetTitle().c_str());
				continue;
			}
		}
		Info(REVGEO_TAG "%s - Finish processing...", pConfiguration->GetTitle().c_str());

		// Config a time to wait until the next process
		sleep(pConfiguration->GetSleepProcessInterval());

		return EXIT_SUCCESS;
	}
	catch(const mongo::DBException &e)
	{
		Error(REVGEO_TAG "%s - Failed to connect to mongodb: %s", pConfiguration->GetTitle().c_str(), e.what());

		// Force to ignore excptions
		//return EXIT_FAILURE;

		return EXIT_SUCCESS;
	}
}

bool RevGeoApp::Shutdown()
{
	Info(REVGEO_TAG "%s - Shutting down...", pConfiguration->GetTitle().c_str());

	// Desconnect from mongodb
	BSONObj info;
	cDBConnection.logout(pConfiguration->GetMongoDBHost(), info);
	Info(REVGEO_TAG "%s - Disconnected from mongodb...", pConfiguration->GetTitle().c_str());

	return true;
}

int RevGeoApp::GetCountPosition()
{
	int totalPosition = cDBConnection.query(pConfiguration->GetMongoDBCollection(), kQueryGet).get()->itcount();
	return totalPosition;
}

void RevGeoApp::UpdatePosition(struct endereco_posicao_mapa *data, int veiculoId)
{
	Query query = QUERY("veiculo" << veiculoId);

	// Verify data gathered from WS
	std::string numero = ((data->numero != NULL) && (data->numero[0] != '\0')) ? data->numero : "0";
	std::string velocidadeVia = "";
	std::string endereco = ((data->rua != NULL) && (data->rua[0] != '\0')) ? data->rua : "SEM ENDERECO";
	std::string distancia = "";
	std::string bairro = ((data->bairro != NULL) && (data->bairro[0] != '\0')) ? data->bairro : "";
	std::string municipio = ((data->municipio != NULL) && (data->municipio[0] != '\0')) ? data->municipio : "?";
	std::string uf = ((data->uf != NULL) && (data->uf[0] != '\0')) ? data->uf : "?";
	std::string pais = ((data->pais != NULL) && (data->pais[0] != '\0')) ? data->pais : "?";

	// Create update query
	BSONObj querySet = BSON("$set" << BSON(
										"numero"		<< numero	<< "velocidadeVia"	<< velocidadeVia <<
										"endereco"		<< endereco	<< "distancia"		<< distancia <<
										"bairro"		<< bairro	<< "municipio"		<< municipio <<
										"estado"		<< uf		<< "pais"			<< pais
										)
							);

	Info(REVGEO_TAG "%s - Update querySet: %s", pConfiguration->GetTitle().c_str(), querySet.toString().c_str());

	// Updating based on vehicle id with multiple parameter
	cDBConnection.update(pConfiguration->GetMongoDBCollection(), query, querySet, false, true);
}
