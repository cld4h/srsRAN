#ifndef SRSRELAY_RELAY_METRICS_INTERFACE_H
#define SRSRELAY_RELAY_METRICS_INTERFACE_H

#include "srslte/common/metrics_hub.h"

namespace srsrelay {

typedef struct {
  bool                 running;
} relay_metrics_t;

// RELAY interface
class relay_metrics_interface : public srslte::metrics_interface<relay_metrics_t>

{
public:
  virtual bool get_metrics(relay_metrics_t* m) = 0;
};
}

#endif //SRSRELAY_RELAY_METRICS_INTERFACE_H