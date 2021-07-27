#include <iostream>
#include "srsrelay/hdr/relay.h"
#include "srslte/common/buffer_pool.h"
#include "srslte/common/logger.h"
#include "srslte/build_info.h"
#include "srsrelay/hdr/phy.h"

using namespace srslte;

namespace srsrelay{

relay::relay() : logger(nullptr)
{
  // print build info
  std::cout << std::endl << get_build_string() << std::endl;
  pool = byte_buffer_pool::get_instance();
}

int relay::init(const all_args_t& args_, srslte::logger* logger_)
{
  int ret = SRSLTE_SUCCESS;
  logger  = logger_;
  args    = args_;

  // Init RELAY log
  log.init("RELAY  ", logger);
  log.set_level(srslte::LOG_LEVEL_DEBUG);
  log.info("%s", get_build_string().c_str());

  std::unique_ptr<srsrelay::phy> lte_phy = std::unique_ptr<srsrelay::phy>(new srsrelay::phy(logger));
  if (!lte_phy) {
    srslte::console("Error creating LTE PHY instance.\n");
    return SRSLTE_ERROR;
  }

  log.debug("LTE PHY instance created!");

  std::unique_ptr<srslte::radio> lte_radio = std::unique_ptr<srslte::radio>(new srslte::radio(logger));
  if (!lte_radio) {
    srslte::console("Error creating radio multi instance.\n");
    return SRSLTE_ERROR;
  }

  // init layers
  if (lte_radio->init(args.rf, lte_phy.get())) {
    srslte::console("Error initializing radio.\n");
    return SRSLTE_ERROR;
  }

  return ret;
}

bool relay::get_metrics(relay_metrics_t* m)
{
  m->running = started;
  return true;
}

std::string relay::get_build_info()
{
  if (std::string(srslte_get_build_info()).find("  ") != std::string::npos) {
    return std::string(srslte_get_version());
  }
  return std::string(srslte_get_build_info());
}

std::string relay::get_build_mode()
{
  return std::string(srslte_get_build_mode());
}

std::string relay::get_build_string()
{
  std::stringstream ss;
  ss << "Built in " << get_build_mode() << " mode using " << get_build_info() << "." << std::endl;
  return ss.str();
}

} //srsrelay