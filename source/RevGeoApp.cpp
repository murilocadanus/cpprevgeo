#include "RevGeoApp.hpp"
#include "Configuration.hpp"
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/reader.h>

#define REVGEO_TAG "[RevGeo] "

using namespace rapidjson;

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

	// Set service header
	struct curl_slist *headers;
	headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "charset: utf-8");

	// Set service parameters
	cService.SetOption(CURLOPT_HTTPHEADER, headers);
	cService.SetOption(CURLOPT_HTTPGET, 1);
	cService.SetOption(CURLOPT_WRITEFUNCTION, CurlWrapper::Writer);
	cService.SetOption(CURLOPT_WRITEDATA, &CurlWrapper::sBuffer);
	cService.SetOption(CURLOPT_TIMEOUT, pConfiguration->GetServiceTimeOut());

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

			if(query_res.getIntField("veiculo") > 0)
			{
				// Get mongodb attributes to use as entry data
				int veiculo = query_res.getIntField("veiculo");

				double lon = query_res.getFieldDotted("coordenadas.coordinates.0").Double();
				double lat = query_res.getFieldDotted("coordenadas.coordinates.1").Double();

				// Init the struct with empty values to storage callback values
				endereco_posicao_mapa data = (const struct endereco_posicao_mapa){ NULL };

				Info(REVGEO_TAG "%s - --------------------", pConfiguration->GetTitle().c_str());
				Info(REVGEO_TAG "%s - OID: %d", pConfiguration->GetTitle().c_str(), query_res.getField("_id"));
				Info(REVGEO_TAG "%s - Veiculo: %d", pConfiguration->GetTitle().c_str(), veiculo);

				// Create the URL
				cService.UrlComposer(pConfiguration->GetServiceURL().c_str(), lat, lon, pConfiguration->GetServiceKey().c_str());

				// Set the url with parameters into the service
				cService.SetOption(CURLOPT_URL, cService.GetUrlWithParams());

				// Call a WS for reverse geolocation
				cService.Perform();

				if(cService.IsOK())
				{
					Document document;

					// Parse response as JSON
					if (document.Parse<0>(CurlWrapper::sBuffer.c_str()).HasParseError())
					{
						// Clear buffer for new iteraction
						CurlWrapper::sBuffer.clear();

						Info(REVGEO_TAG "%s - Can't parse JSON data", pConfiguration->GetTitle().c_str());
						continue;
					}
					else
					{
						// Clear buffer for new iteraction
						CurlWrapper::sBuffer.clear();

						// Retrieve results
						const Value& results = document["results"];

						// Verify data
						if (results.Size() <= 0)
						{
							Info(REVGEO_TAG "%s - Zero results", pConfiguration->GetTitle().c_str());

							// Update mongodb position with revgeo data
							this->UpdatePosition(&data, veiculo);

							continue;
						}
						else if(!results[0]["address_components"].IsArray())
						{
							Info(REVGEO_TAG "%s - Can't get JSON address components data", pConfiguration->GetTitle().c_str());
							continue;
						}
						else
						{
							const Value& addresses = results[0]["address_components"];

							for (SizeType i = 0; i < addresses.Size(); i++)
							{
								std::string type = addresses[i]["types"][0].GetString();

								if (type == "country")
									strcpy(data.pais, addresses[i]["short_name"].GetString());

								if (type == "administrative_area_level_1")
									strcpy(data.uf, addresses[i]["short_name"].GetString());

								if (type == "locality")
									strcpy(data.municipio, addresses[i]["long_name"].GetString());

								if (type == "sublocality_level_1")
									strcpy(data.bairro, addresses[i]["long_name"].GetString());

								if (type == "route")
									strcpy(data.rua, addresses[i]["long_name"].GetString());

								if (type == "street_number")
									strcpy(data.numero, addresses[i]["long_name"].GetString());
							}

							Info(REVGEO_TAG "%s - Rua: %s", pConfiguration->GetTitle().c_str(), data.rua);
							Info(REVGEO_TAG "%s - Bairro: %s", pConfiguration->GetTitle().c_str(), data.bairro);
							Info(REVGEO_TAG "%s - Municipio: %s", pConfiguration->GetTitle().c_str(), data.municipio);
							Info(REVGEO_TAG "%s - Uf: %s", pConfiguration->GetTitle().c_str(), data.uf);
							Info(REVGEO_TAG "%s - Numero: %s", pConfiguration->GetTitle().c_str(), data.numero);
							Info(REVGEO_TAG "%s - Pais: %s", pConfiguration->GetTitle().c_str(), data.pais);

							// Update mongodb position with revgeo data
							this->UpdatePosition(&data, veiculo);
						}
					}
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
		return EXIT_SUCCESS;
	}
	catch(const mongo::DBException &e)
	{
		Error(REVGEO_TAG "%s - Failed to connect to mongodb: %s", pConfiguration->GetTitle().c_str(), e.what());
		return EXIT_FAILURE;
	}
}

bool RevGeoApp::Shutdown()
{
	Info(REVGEO_TAG "%s - Shutting down...", pConfiguration->GetTitle().c_str());

	// Disconnect from mongodb
	BSONObj info;
	cDBConnection.logout(pConfiguration->GetMongoDBHost(), info);
	Info(REVGEO_TAG "%s - Disconnected from mongodb...", pConfiguration->GetTitle().c_str());

	// Config a time to wait until the next process
	sleep(pConfiguration->GetSleepProcessInterval());

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

