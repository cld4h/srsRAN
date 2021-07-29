add_executable(srsrelay main.cc relay.cc)
target_link_libraries(srsrelay)

set(SRSUE_SOURCES srsue_phy srsue_stack srsue_upper srsue_mac srsue_rrc srslog)
set(SRSENB_SOURCES srsenb_phy srsenb_stack srsenb_upper srsenb_mac srsenb_rrc srslog)
set(SRSLTE_SOURCES srslte_common srslte_mac srslte_phy srslte_radio srslte_upper rrc_asn1 srslog)

target_link_libraries(srsrelay  ${SRSRELAY_SOURCES}
                                ${SRSUE_SOURCES}
                                ${SRSENB_SOURCES}
                                ${SRSLTE_SOURCES}
                                ${CMAKE_THREAD_LIBS_INIT}
                                ${Boost_LIBRARIES} 
                                )