SET(LIB_OUTNAME lightmap)

SET(HEADER_PATH ${OCTOON_PATH_HEADER}/${LIB_OUTNAME})
SET(SOURCE_PATH ${OCTOON_PATH_SOURCE}/octoon-core/${LIB_OUTNAME})

SET(LIGHTMAP_LIST
	${HEADER_PATH}/lightmap.h
	${SOURCE_PATH}/lightmap.cpp
	${HEADER_PATH}/lightmap_pack.h
)
SOURCE_GROUP(lightmap  FILES ${LIGHTMAP_LIST})

LIST(APPEND LIGHTMAP_LIST ${LIGHTMAP_LIST})