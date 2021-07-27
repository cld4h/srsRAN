#include "srslte/common/log.h"
#include "srslte/common/log_filter.h"
#include "srslte/interfaces/radio_interfaces.h"
namespace srsrelay{

class phy final : public srslte::phy_interface_radio
{
private:
  /* data */
  srslte::logger*                                   logger = nullptr;
public:
  phy(srslte::logger* logger_);
  ~phy();

  void radio_overflow() override;
  void radio_failure() override;
};
}