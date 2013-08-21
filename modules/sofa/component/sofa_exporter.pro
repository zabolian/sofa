load(sofa/pre)

TEMPLATE = lib
TARGET = sofa_exporter

DEFINES += SOFA_BUILD_EXPORTER

HEADERS += initExporter.h \
           misc/WriteState.h \
           misc/WriteState.inl \
           misc/WriteTopology.h \
           misc/WriteTopology.inl \
           misc/VTKExporter.h \
           misc/OBJExporter.h \
           misc/STEPExporter.h \
           misc/MDDExporter.h \
           misc/INPExporter.h \
           misc/INPExporterMaster.h \
           misc/STLExporter.h \
           misc/MeshExporter.h

SOURCES += initExporter.cpp \
           misc/WriteState.cpp \
           misc/WriteTopology.cpp \
           misc/VTKExporter.cpp \
           misc/OBJExporter.cpp \
           misc/STEPExporter.cpp \
           misc/MDDExporter.cpp \
           misc/INPExporter.cpp \
           misc/INPExporterMaster.cpp \
           misc/STLExporter.cpp \
           misc/MeshExporter.cpp

# Make sure there are no cross-dependencies
INCLUDEPATH -= $$SOFA_INSTALL_INC_DIR/applications
DEPENDPATH -= $$SOFA_INSTALL_INC_DIR/applications

#exists(component-local.cfg): include(component-local.cfg)

load(sofa/post)
 
