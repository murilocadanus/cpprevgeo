SOURCES +=	source/main.cpp \
			source/RevGeoApp.cpp

HEADERS +=	include/RevGeoApp.hpp \
			include/Defines.hpp

OTHER_FILES += resources/configs/sascloud.config

include("app.pri")
include("compiler.pri")

