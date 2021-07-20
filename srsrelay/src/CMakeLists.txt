add_executable(srsrelay main.cc relay.cc)
target_link_libraries(srsrelay)

set(SRSRELAY_SOURCES srslog)
#set(SRSRELAY_SOURCES srsrelay_phy srsrelay_stack srsrelay_upper srsrelay_mac srslog)
set(SRSLTE_SOURCES srslte_common srslog)
#set(SRSLTE_SOURCES srslte_common srslte_mac srslte_phy srslte_radio srslte_upper rrc_asn1 srslog)

target_link_libraries(srsrelay  ${SRSRELAY_SOURCES}
                                ${SRSLTE_SOURCES}
                                ${CMAKE_THREAD_LIBS_INIT}
                                ${Boost_LIBRARIES} 
                                )