target_sources(${LSE2_TARGET}
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/blockhandler.h
        ${CMAKE_CURRENT_LIST_DIR}/dataunit.h
        ${CMAKE_CURRENT_LIST_DIR}/protocol.h
        ${CMAKE_CURRENT_LIST_DIR}/protocolunit.h
        ${CMAKE_CURRENT_LIST_DIR}/satpext.h

        ${CMAKE_CURRENT_LIST_DIR}/tcp_protocol.h

        ${CMAKE_CURRENT_LIST_DIR}/tcp_protocol.cc
        )