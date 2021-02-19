set(NVIDIA_HEADERS
    types.h    
    manager.h
    event_registry.h
    thread_pool.h
    synchronized_queue.h    
    )

install(FILES ${NVIDIA_HEADERS} DESTINATION ${INCLUDE_DIR}/nvidia)
