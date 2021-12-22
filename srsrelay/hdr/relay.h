/******************************************************************************
 * File:        relay.h
 * Description: Top-level relay class. Creates and links all
 *              layers and helpers.
 *****************************************************************************/

#ifndef SRSRELAY_RELAY_H
#define SRSRELAY_RELAY_H

#include "relay_metrics_interface.h"
#include "srsran/common/buffer_pool.h"
#include "srsran/common/log_filter.h"
#include "srsran/radio/radio.h"
#include "srsenb/hdr/enb.h"
#include "srsue/hdr/ue.h"

namespace srsrelay{
    
/*******************************************************************************
  RELAY Parameters
*******************************************************************************/
typedef struct {
  std::string all_level;
  int         all_hex_limit;
  int         file_max_size;
  std::string filename;
} log_args_t;

typedef struct {
  srsran::rf_args_t ue_rf;
  srsran::rf_args_t enb_rf;
  srsue::phy_args_t ue_phy;
  srsenb::phy_args_t enb_phy;
  srsue::stack_args_t ue_stack;
  srsenb::stack_args_t enb_stack;

  log_args_t        log;
} all_args_t;

/*******************************************************************************
  Main RELAY class
*******************************************************************************/
class relay : public relay_metrics_interface
{
public:

  relay();
  int init(const all_args_t& args_, srsran::logger* logger_);

  // RELAY metrics interface
  bool get_metrics(relay_metrics_t* m);
private:

  bool started = false;

  // Generic logger members
  srsran::logger*    logger = nullptr;
  srsran::log_filter log; // Own logger for RELAY

  all_args_t                args;
  srsran::byte_buffer_pool* pool = nullptr;

  std::string get_build_mode();
  std::string get_build_info();
  std::string get_build_string();
};

} //namespace srsrelay

#endif // SRSRELAY_RELAY_H