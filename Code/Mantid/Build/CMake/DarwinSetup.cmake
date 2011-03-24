set ( CMAKE_INCLUDE_PATH "${THIRD_PARTY}/include" )
set ( BOOST_INCLUDEDIR "${THIRD_PARTY}/include" )

set ( CMAKE_LIBRARY_PATH "${THIRD_PARTY}/lib/mac64" )
set ( BOOST_LIBRARYDIR  "${THIRD_PARTY}/lib/mac64" )

# Force 64 bit compile
set ( CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -m64 )
set ( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -m64 )

set ( CMAKE_INSTALL_NAME_DIR ${CMAKE_LIBRARY_PATH} )
