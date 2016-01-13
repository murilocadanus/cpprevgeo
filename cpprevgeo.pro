SOURCES +=	source/main.cpp \
			source/RevGeoApp.cpp

HEADERS +=	include/RevGeoApp.hpp \
			include/Defines.hpp \
			include/revgeo.hpp

OTHER_FILES += resources/app.config

include("app.pri")
include("compiler.pri")

