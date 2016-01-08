SOURCES +=	source/location/libGeoWeb.cpp \
			source/main.cpp \
			source/RevGeoApp.cpp

HEADERS +=	include/location/libGeoWeb.hpp \
			include/location/revgeo.hpp \
			include/RevGeoApp.hpp \
			include/Defines.hpp \
			include/revgeo.hpp

OTHER_FILES += resources/configs/sascloud.config

include("app.pri")
include("compiler.pri")

