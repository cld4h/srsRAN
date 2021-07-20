#include <iostream>
#include "srsrelay/hdr/relay.h"
#include "srslte/common/buffer_pool.h"
#include "srslte/common/logger.h"
#include "srslte/build_info.h"

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

  // Init RELAY log
  log.init("RELAY  ", logger);
  log.set_level(srslte::LOG_LEVEL_INFO);
  log.info("%s", get_build_string().c_str());

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