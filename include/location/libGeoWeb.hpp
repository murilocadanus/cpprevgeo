#ifndef __LIBGEOWEB_H_
#define __LIBGEOWEB_H_

#include <curl/curl.h>
#include "location/revgeo.hpp"
#include <string>

// URL para chamar o web service de resolucao de endereco
//#define REV_GEO_WEB_URL "http://revgeowebdev1.sascar.com.br/revgeoWeb-webapp/resources/geolocalizacao/recuperaGeoLocalizacao"
//#define REV_GEO_WEB_URL "http://revgeowebtst1.sascar.com.br/revgeoWeb-webapp/resources/geolocalizacao/recuperaGeoLocalizacao"
//#define REV_GEO_WEB_URL "http://revgeowebhom1.sascar.com.br/revgeoWeb-webapp/resources/geolocalizacao/recuperaGeoLocalizacao"
//#define REV_GEO_WEB_URL "http://revgeo.sascar-dc.br/revgeoWeb-webapp/resources/geolocalizacao/recuperaGeoLocalizacao"

#define REV_GEO_WEB_URL "https://maps.google.com/maps/api/geocode/json?latlng=-25.271193,-50.141602&sensor=false&key=AIzaSyCgIsGUoox-_-bD2o2d2bzrWru4SXFDZRs"

// Timeout em segundos
#define REV_GEO_TIMEOUT 5L

class LibGeoWeb
{
	public:
		CURL *curlPostRevgeo;
		CURLcode resultURLCode;
		struct curl_slist *headers;
		std::string readBufferRevgeo;
		char webParams[128];
		char urlWithParams[255];

	public:
		void converteEncode( const char* encodeOriginal, const char* encodeNovo, char* bufferEntrada, char* bufferSaida, size_t maximo );
		int32_t revGeoWeb( double latit, double longit, struct endereco_posicao_mapa *end);
		void finalizaGeoWeb();
};
#endif // _LIBGEOWEB_H_
