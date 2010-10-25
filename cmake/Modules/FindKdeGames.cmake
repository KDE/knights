# - Try to find the KdeGames library
# Once done this will define
#  KDEGAMES_FOUND        - System has the KdeGames include directory and library.
#  KDEGAMES_INCLUDE_DIR  - The KdeGames include directory.
#  KDEGAMES_LIBRARY      - Link this to use the KdeGames library.

if(KDEGAMES_INCLUDE_DIR AND KDEGAMES_LIBRARY)
	#in cache already
	set(KdeGames_FOUND TRUE)
else(KDEGAMES_INCLUDE_DIR AND KDEGAMES_LIBRARY)
	#reset variables
	set(KDEGAMES_INCLUDE_DIRS)
	set(KDEGAMES_INCLUDE_DIR)
	set(KDEGAMES_LIBRARY)
	# locate includes
	find_path(KDEGAMES_INCLUDE_DIR NAMES kgametheme.h HINTS
		${INCLUDE_INSTALL_DIR} ${KDE4_INCLUDE_DIR}
		${GNUWIN32_DIR}/include
	)
	set(KDEGAMES_INCLUDE_DIR ${KDEGAMES_INCLUDE_DIR})
	# locate libraries
	find_library(KDEGAMES_LIBRARY NAMES kdegames PATHS
		${LIB_INSTALL_DIR} ${KDE4_LIB_DIR}
		${GNUWIN32_DIR}/lib
	)
	set(KDEGAMES_LIBRARY ${KDEGAMES_LIBRARY})
	# handle find_package() arguments
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(KdeGames DEFAULT_MSG KDEGAMES_INCLUDE_DIR KDEGAMES_LIBRARY)
	mark_as_advanced(KDEGAMES_INCLUDE_DIR KDEGAMES_LIBRARY)
endif(KDEGAMES_INCLUDE_DIR AND KDEGAMES_LIBRARY)
