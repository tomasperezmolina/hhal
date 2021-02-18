set(GN_HEADERS
    types.h    
    manager.h    
    tlb.h    
    )

install(FILES ${GN_HEADERS} DESTINATION ${INCLUDE_DIR}/gn)

#TODO@v2 temp installing whole gn/gn dir
install(DIRECTORY gn/ DESTINATION ${INCLUDE_DIR}/gn/gn)


