/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2021 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#ifndef SRSRAN_DUMMY_GNB_STACK_H
#define SRSRAN_DUMMY_GNB_STACK_H

#include <srsran/adt/circular_array.h>
#include <srsran/common/phy_cfg_nr.h>
#include <srsran/interfaces/gnb_interfaces.h>

class gnb_dummy_stack : public srsenb::stack_interface_phy_nr
{
private:
  srslog::basic_logger&                                                logger = srslog::fetch_basic_logger("GNB STK");
  const uint16_t                                                       rnti   = 0x1234;
  const uint32_t                                                       mcs    = 1;
  srsran::circular_array<srsran_dci_location_t, SRSRAN_NOF_SF_X_FRAME> dci_dl_location;
  srsran::circular_array<srsran_dci_location_t, SRSRAN_NOF_SF_X_FRAME> dci_ul_location;
  srsran::circular_array<uint32_t, SRSRAN_NOF_SF_X_FRAME>              dl_data_to_ul_ack;
  uint32_t                                                             ss_id         = 0;
  srsran_dci_format_nr_t                                               dci_format_ul = SRSRAN_DCI_FORMAT_NR_COUNT;
  srsran_dci_format_nr_t                                               dci_format_dl = SRSRAN_DCI_FORMAT_NR_COUNT;
  uint32_t                                                             dl_freq_res   = 0;
  uint32_t                                                             dl_time_res   = 0;
  srsran_random_t                                                      random_gen    = nullptr;
  srsran::phy_cfg_nr_t                                                 phy_cfg       = {};
  bool                                                                 valid         = false;

  struct dummy_harq_proc {
    static const uint32_t  MAX_TB_SZ = SRSRAN_LDPC_MAX_LEN_CB * SRSRAN_SCH_NR_MAX_NOF_CB_LDPC;
    std::vector<uint8_t>   data;
    srsran_softbuffer_tx_t softbuffer = {};

    dummy_harq_proc()
    {
      // Allocate data
      data.resize(MAX_TB_SZ);

      // Initialise softbuffer
      if (srsran_softbuffer_tx_init_guru(&softbuffer, SRSRAN_SCH_NR_MAX_NOF_CB_LDPC, SRSRAN_LDPC_MAX_LEN_ENCODED_CB) <
          SRSRAN_SUCCESS) {
        ERROR("Error Tx buffer");
      }
    }

    ~dummy_harq_proc() { srsran_softbuffer_tx_free(&softbuffer); }
  };
  srsran::circular_array<dummy_harq_proc, SRSRAN_MAX_HARQ_PROC_DL_NR> tx_harq_proc;

public:
  struct args_t {
    srsran::phy_cfg_nr_t phy_cfg;                           ///< Physical layer configuration
    uint16_t             rnti                     = 0x1234; ///< C-RNTI
    uint32_t             mcs                      = 10;     ///< Modulation code scheme
    uint32_t             ss_id                    = 1;      ///< Search Space identifier
    uint32_t             pdcch_aggregation_level  = 0;      ///< PDCCH aggregation level
    uint32_t             pdcch_dl_candidate_index = 0;      ///< PDCCH DL DCI candidate index
    uint32_t             pdcch_ul_candidate_index = 0;      ///< PDCCH UL DCI candidate index
    uint32_t             dl_start_rb              = 0;      ///< Start resource block
    uint32_t             dl_length_rb             = 0l;     ///< Number of resource blocks
    uint32_t             dl_time_res              = 0;      ///< PDSCH time resource
  };

  gnb_dummy_stack(args_t args) :
    mcs(args.mcs), rnti(args.rnti), dl_time_res(args.dl_time_res), phy_cfg(args.phy_cfg), ss_id(args.ss_id)
  {
    random_gen = srsran_random_init(0x1234);

    // Select DCI locations
    for (uint32_t slot = 0; slot < SRSRAN_NOF_SF_X_FRAME; slot++) {
      srsran::bounded_vector<srsran_dci_location_t, SRSRAN_SEARCH_SPACE_MAX_NOF_CANDIDATES_NR> locations;

      if (not phy_cfg.get_dci_locations(slot, rnti, args.ss_id, args.pdcch_aggregation_level, locations)) {
        logger.error(
            "Error generating locations for slot %d and aggregation level %d", slot, args.pdcch_aggregation_level);
        return;
      }

      // DCI DL
      if (args.pdcch_dl_candidate_index >= locations.size()) {
        logger.error("Candidate index %d exceeds the number of candidates %d for aggregation level %d",
                     args.pdcch_dl_candidate_index,
                     (uint32_t)locations.size(),
                     args.pdcch_aggregation_level);
        return;
      }
      dci_dl_location[slot] = locations[args.pdcch_dl_candidate_index];

      // DCI UL
      if (args.pdcch_ul_candidate_index >= locations.size()) {
        logger.error("Candidate index %d exceeds the number of candidates %d for aggregation level %d",
                     args.pdcch_ul_candidate_index,
                     (uint32_t)locations.size(),
                     args.pdcch_aggregation_level);
        return;
      }
      dci_ul_location[slot] = locations[args.pdcch_ul_candidate_index];
    }

    // Select DCI formats
    dci_format_dl = phy_cfg.get_dci_format_pdsch(args.ss_id);
    dci_format_ul = phy_cfg.get_dci_format_pusch(args.ss_id);
    if (dci_format_dl == SRSRAN_DCI_FORMAT_NR_COUNT or dci_format_ul == SRSRAN_DCI_FORMAT_NR_COUNT) {
      logger.error("Missing valid DL or UL DCI format in search space");
      return;
    }

    // Select DL frequency domain resources
    dl_freq_res = srsran_ra_nr_type1_riv(args.phy_cfg.carrier.nof_prb, args.dl_start_rb, args.dl_length_rb);

    // Setup DL Data to ACK timing
    for (uint32_t i = 0; i < SRSRAN_NOF_SF_X_FRAME; i++) {
      dl_data_to_ul_ack[i] = args.phy_cfg.harq_ack.dl_data_to_ul_ack[i % SRSRAN_MAX_NOF_DL_DATA_TO_UL];
    }

    // If reached this point the configuration is valid
    valid = true;
  }

  ~gnb_dummy_stack() { srsran_random_free(random_gen); }
  bool is_valid() const { return valid; }

  int rx_data_indication(rx_data_ind_t& grant) override { return 0; }

  int slot_indication(const srsran_slot_cfg_t& slot_cfg) override { return 0; }

  int get_dl_sched(const srsran_slot_cfg_t& slot_cfg, dl_sched_t& dl_sched) override
  {
    // Check if it is TDD DL slot and PDSCH mask, if no PDSCH shall be scheduled, do not set any grant and skip
    if (not srsran_tdd_nr_is_dl(&phy_cfg.tdd, phy_cfg.carrier.scs, slot_cfg.idx)) {
      return SRSRAN_SUCCESS;
    }

    // Instantiate PDCCH and PDSCH
    pdcch_dl_t pdcch = {};
    pdsch_t    pdsch = {};

    // Select grant and set data
    pdsch.data[0] = tx_harq_proc[slot_cfg.idx].data.data();

    // Second TB is not used
    pdsch.data[1] = nullptr;

    // Fill DCI configuration
    pdcch.dci_cfg = phy_cfg.get_dci_cfg();

    // Fill DCI context
    if (not phy_cfg.get_dci_ctx_pdsch_rnti_c(ss_id, dci_dl_location[slot_cfg.idx], rnti, pdcch.dci.ctx)) {
      logger.error("Error filling PDSCH DCI context");
      return SRSRAN_ERROR;
    }

    // Fill DCI fields
    srsran_dci_dl_nr_t& dci   = pdcch.dci;
    dci.freq_domain_assigment = dl_freq_res;
    dci.time_domain_assigment = dl_time_res;
    dci.mcs                   = mcs;
    dci.rv                    = 0;
    dci.ndi                   = (slot_cfg.idx / SRSRAN_NOF_SF_X_FRAME) % 2;
    dci.pid                   = slot_cfg.idx % SRSRAN_NOF_SF_X_FRAME;
    dci.dai                   = slot_cfg.idx % SRSRAN_NOF_SF_X_FRAME;
    dci.tpc                   = 1;
    dci.pucch_resource        = 0;
    dci.harq_feedback         = dl_data_to_ul_ack[TTI_TX(slot_cfg.idx)];

    // Create PDSCH configuration
    if (not phy_cfg.get_pdsch_cfg(slot_cfg, dci, pdsch.sch)) {
      logger.error("Error converting DCI to grant");
      return SRSRAN_ERROR;
    }

    // Generate random data
    srsran_random_byte_vector(random_gen, pdsch.data[0], pdsch.sch.grant.tb[0].tbs / 8);

    // Set softbuffer
    pdsch.sch.grant.tb[0].softbuffer.tx = &tx_harq_proc[slot_cfg.idx].softbuffer;

    // Reset Tx softbuffer always
    srsran_softbuffer_tx_reset(pdsch.sch.grant.tb[0].softbuffer.tx);

    // Push scheduling results
    dl_sched.pdcch_dl.push_back(pdcch);
    dl_sched.pdsch.push_back(pdsch);

    return SRSRAN_SUCCESS;
  }

  int get_ul_sched(const srsran_slot_cfg_t& slot_cfg, ul_sched_t& ul_sched) override { return 0; }
};

#endif // SRSRAN_DUMMY_GNB_STACK_H
