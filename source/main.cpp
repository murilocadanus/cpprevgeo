#include "RevGeoApp.hpp"
#include <thread>

int main(int argc, char **argv)
{
	//std::thread t1(I9Run<RevGeoApp>, argc, argv, "plataforma_sasweb.config");
	//t1.join();

	std::thread t2(I9Run<RevGeoApp>, argc, argv, "sasintegra.config");
	t2.join();
}
