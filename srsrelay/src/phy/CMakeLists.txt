#set(SOURCES cc_worker.cc phy.cc phy_common.cc phy_ue_db.cc prach_worker.cc sf_worker.cc txrx.cc)
set(SOURCES phy.cc )
add_library(srsrelay_phy STATIC ${SOURCES})