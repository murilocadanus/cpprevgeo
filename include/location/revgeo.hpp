#ifndef __REVGEO_H__
#define __REVGEO_H__

#define NESTADOS 27
#define TAMHASH 2000

#define DBREVGEO "/hugetlbfs/enderecos"

struct endereco_posicao {

		char municipio[65];
		char rua[90];
		char uf;

		unsigned int prox;
};

struct endereco_posicao_geo {

		char municipio[60];
		char bairro[60];
		char rua[68];
		char uf;

		unsigned int prox;
};


struct endereco_posicao_mapa {

		char municipio[60];
		char bairro[60];
		char rua[68];
		char numero[6];
		char cep[12];
		char uf[8];
		char pais[20];
		char acesso_carro[2];
};


struct cache_endereco {
		unsigned int nposicao;
		unsigned long int hash[28][TAMHASH];
		struct endereco_posicao end[2000000];
};

struct cache_endereco_mapa {
		unsigned int nposicao;
		unsigned long int hash[28][TAMHASH];
		struct endereco_posicao_mapa end[2000000];
};

struct cache_endereco_geo {
		unsigned int nposicao;
		unsigned long int hash[28][TAMHASH];
		struct endereco_posicao_geo end[2000000];
};

// #ifndef SOUSO

static char *estados [] = {
	(char*)  "ACRE",
	(char*)  "ALAGOAS",
	(char*)  "AMAPA",
	(char*)  "AMAZONAS",
	(char*)  "BAHIA",
	(char*)  "CEARA",
	(char*)  "DISTRITO FEDERAL",
	(char*)  "ESPIRITO SANTO",
	(char*)  "GOIAS",
	(char*)  "MARANHAO",
	(char*)  "MATO GROSSO",
	(char*)  "MATO GROSSO DO SUL",
	(char*)  "MINAS GERAIS",
	(char*)  "PARA",
	(char*)  "PARAIBA",
	(char*)  "PARANA",
	(char*)  "PERNAMBUCO",
	(char*)  "PIAUI",
	(char*)  "RIO DE JANEIRO",
	(char*)  "RIO GRANDE DO NORTE",
	(char*)  "RIO GRANDE DO SUL",
	(char*)  "RONDONIA",
	(char*)  "RORAIMA",
	(char*)  "SANTA CATARINA",
	(char*)  "SAO PAULO",
	(char*)  "SERGIPE",
	(char*)  "TOCANTINS",
	(char*)  ""
};

static char *ufs [] = {
	(char*)  "AC",
	(char*)  "AL",
	(char*)  "AP",
	(char*)  "AM",
	(char*)  "BA",
	(char*)  "CE",
	(char*)  "DF",
	(char*)  "ES",
	(char*)  "GO",
	(char*)  "MA",
	(char*)  "MT",
	(char*)  "MS",
	(char*)  "MG",
	(char*)  "PA",
	(char*)  "PB",
	(char*)  "PR",
	(char*)  "PE",
	(char*)  "PI",
	(char*)  "RJ",
	(char*)  "RN",
	(char*)  "RS",
	(char*)  "RO",
	(char*)  "RR",
	(char*)  "SC",
	(char*)  "SP",
	(char*)  "SE",
	(char*)  "TO",
	(char*)  ""
};

// #endif
struct enderecowebraska {

		char placa[20];
		int clioid;
		char uf[20];
		char municipio[60];
		char cep[10];
		char rua[60];
		int numero;

};

struct cache_endereco * mem_endereco_init();
struct cache_endereco * mem_endereco_attach();

struct cache_endereco_geo * mem_endereco2_init();
struct cache_endereco_geo * mem_endereco2_attach();

struct cache_endereco_mapa * mem_endereco_mapa_init();
struct cache_endereco_mapa * mem_endereco_mapa_attach();

struct cache_endereco_mapa * mem_endereco2_mapa_init();
struct cache_endereco_mapa * mem_endereco2_mapa_attach();

int revGeo (struct posicao *, struct enderecowebraska *);
int revGeoEnd (double lat2, double long2, struct endereco_posicao *end,unsigned short int *, unsigned int*);
int revGeoGeo (double lat2, double long2, struct endereco_posicao_geo *end,unsigned short int *, unsigned int*);
int revGeoSas (double lat2, double long2, struct endereco_posicao_geo *end,unsigned short int *, unsigned int*);
int revGeoMax (double lat2, double long2, struct endereco_posicao_geo *end,unsigned short int *, unsigned int*);

int endhash (struct endereco_posicao *endp);
int end2hash (struct endereco_posicao_geo *endp);

struct arco {
		double long1;
		double lat1;
		double long2;
		double lat2;

		int nini;
		int nfim;
		//  int ruaoid;
};

struct cidade {
		char nome[42];
		char uf[3];
		char pais[3];
};

struct rua {
		char nome[67];
		char cidade[40];
		char bairro[49];
		char tipo;          // 0 - Rua, 1 - Travessa, 2 - Alameda, 3 - Avenida, 4 - Rodovia
		char uf[3];
};

struct seguradora {
		int regiaosulamerica;
		int pad;
};

struct segmento {
		struct arco arc;
		struct rua ru;
		struct seguradora seg;
};

#endif

