#include "srsrelay/hdr/phy.h"

namespace srsrelay{

phy::phy(srslte::logger* logger_) :
  logger(logger_)
{
}

phy::~phy() = default;

void phy::radio_overflow()
{
  // TODO: handle radio overflow
}

void phy::radio_failure()
{
  // TODO: handle failure
}

}