#include "srsran/common/log.h"
#include "srsran/common/log_filter.h"
#include "srsran/interfaces/radio_interfaces.h"
namespace srsrelay{

class phy final : public srsran::phy_interface_radio
{
private:
  /* data */
  srsran::logger*                                   logger = nullptr;
public:
  phy(srsran::logger* logger_);
  ~phy();

  void radio_overflow() override;
  void radio_failure() override;
};
}