#include <iostream>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include "srslte/common/common_helper.h"
#include "srslte/common/config_file.h"
#include "srsrelay/hdr/relay.h"
#include "srslte/common/signal_handler.h"
#include "srslte/common/crash_handler.h" //srslte_debug_handle_crash
#include "srslte/common/logger_srslog_wrapper.h"
#include "srslte/common/logmap.h"
#include "srslte/srslog/srslog.h"
#include "srslte/config.h"

using namespace std;
using namespace srsrelay;
namespace bpo = boost::program_options;

/// Adjusts the input value in args from kbytes to bytes.
static size_t fixup_log_file_maxsize(int x)
{
  return (x < 0) ? 0 : size_t(x) * 1024u;
}

/**********************************************************************
 *  Program arguments processing
 ***********************************************************************/
string config_file;

static int parse_args(all_args_t* args, int argc, char* argv[])
{
  // Command line only options
  bpo::options_description general("General options");

  general.add_options()("help,h", "Produce help message")("version,v", "Print version information and exit");

  // Command line or config file options
  bpo::options_description common("Configuration options");
  // clang-format off
  common.add_options()
    //rf args:
    ("rf.srate",        bpo::value<double>(&args->rf.srate_hz)->default_value(0.0),      "Force Tx and Rx sampling rate in Hz")
    //-1 indicate using Automatic gain control in ue(AGC), enb uses fixed value though(50 for rx and 70 for tx).
    ("rf.rx_gain",      bpo::value<float>(&args->rf.rx_gain)->default_value(50),         "Front-end receiver gain")
    ("rf.tx_gain",      bpo::value<float>(&args->rf.tx_gain)->default_value(70),         "Front-end transmitter gain (all channels)")
    ("rf.tx_gain[0]",     bpo::value<float>(&args->rf.tx_gain_ch[0])->default_value(-1),  "Front-end transmitter gain CH0")
    ("rf.tx_gain[1]",     bpo::value<float>(&args->rf.tx_gain_ch[1])->default_value(-1),  "Front-end transmitter gain CH1")
    ("rf.tx_gain[2]",     bpo::value<float>(&args->rf.tx_gain_ch[2])->default_value(-1),  "Front-end transmitter gain CH2")
    ("rf.tx_gain[3]",     bpo::value<float>(&args->rf.tx_gain_ch[3])->default_value(-1),  "Front-end transmitter gain CH3")
    ("rf.tx_gain[4]",     bpo::value<float>(&args->rf.tx_gain_ch[4])->default_value(-1),  "Front-end transmitter gain CH4")
    ("rf.device_name",       bpo::value<string>(&args->rf.device_name)->default_value("auto"),       "Front-end device name")
    ("rf.device_args",       bpo::value<string>(&args->rf.device_args)->default_value("auto"),       "Front-end device arguments")
    ("rf.time_adv_nsamples", bpo::value<string>(&args->rf.time_adv_nsamples)->default_value("auto"), "Transmission time advance")
    ("log.rf_level",     bpo::value<string>(&args->rf.log_level)->default_value("debug"),         "RF log level")
    //from ue
    ("relay.radio", bpo::value<string>(&args->rf.type)->default_value("multi"), "Type of the radio [multi]")
    ("rf.freq_offset",  bpo::value<float>(&args->rf.freq_offset)->default_value(0),      "(optional) Frequency offset")
    ("rf.rx_gain[0]",   bpo::value<float>(&args->rf.rx_gain_ch[0])->default_value(-1),   "Front-end receiver gain CH0")
    ("rf.rx_gain[1]",   bpo::value<float>(&args->rf.rx_gain_ch[1])->default_value(-1),   "Front-end receiver gain CH1")
    ("rf.rx_gain[2]",   bpo::value<float>(&args->rf.rx_gain_ch[2])->default_value(-1),   "Front-end receiver gain CH2")
    ("rf.rx_gain[3]",   bpo::value<float>(&args->rf.rx_gain_ch[3])->default_value(-1),   "Front-end receiver gain CH3")
    ("rf.rx_gain[4]",   bpo::value<float>(&args->rf.rx_gain_ch[4])->default_value(-1),   "Front-end receiver gain CH4")
    ("rf.nof_carriers", bpo::value<uint32_t>(&args->rf.nof_carriers)->default_value(1),  "Number of carriers")
    ("rf.nof_antennas", bpo::value<uint32_t>(&args->rf.nof_antennas)->default_value(1),  "Number of antennas per carrier")
    ("rf.continuous_tx", bpo::value<string>(&args->rf.continuous_tx)->default_value("auto"), "Transmit samples continuously to the radio or on bursts (auto/yes/no). Default is auto (yes for UHD, no for rest)")

    ("rf.bands.rx[0].min", bpo::value<float>(&args->rf.ch_rx_bands[0].min)->default_value(0), "Lower frequency boundary for CH0-RX")
    ("rf.bands.rx[0].max", bpo::value<float>(&args->rf.ch_rx_bands[0].max)->default_value(0), "Higher frequency boundary for CH0-RX")
    ("rf.bands.rx[1].min", bpo::value<float>(&args->rf.ch_rx_bands[1].min)->default_value(0), "Lower frequency boundary for CH1-RX")
    ("rf.bands.rx[1].max", bpo::value<float>(&args->rf.ch_rx_bands[1].max)->default_value(0), "Higher frequency boundary for CH1-RX")
    ("rf.bands.rx[2].min", bpo::value<float>(&args->rf.ch_rx_bands[2].min)->default_value(0), "Lower frequency boundary for CH2-RX")
    ("rf.bands.rx[2].max", bpo::value<float>(&args->rf.ch_rx_bands[2].max)->default_value(0), "Higher frequency boundary for CH2-RX")
    ("rf.bands.rx[3].min", bpo::value<float>(&args->rf.ch_rx_bands[3].min)->default_value(0), "Lower frequency boundary for CH3-RX")
    ("rf.bands.rx[3].max", bpo::value<float>(&args->rf.ch_rx_bands[3].max)->default_value(0), "Higher frequency boundary for CH3-RX")
    ("rf.bands.rx[4].min", bpo::value<float>(&args->rf.ch_rx_bands[4].min)->default_value(0), "Lower frequency boundary for CH4-RX")
    ("rf.bands.rx[4].max", bpo::value<float>(&args->rf.ch_rx_bands[4].max)->default_value(0), "Higher frequency boundary for CH4-RX")

    ("rf.bands.tx[0].min", bpo::value<float>(&args->rf.ch_tx_bands[0].min)->default_value(0), "Lower frequency boundary for CH1-TX")
    ("rf.bands.tx[0].max", bpo::value<float>(&args->rf.ch_tx_bands[0].max)->default_value(0), "Higher frequency boundary for CH1-TX")
    ("rf.bands.tx[1].min", bpo::value<float>(&args->rf.ch_tx_bands[1].min)->default_value(0), "Lower frequency boundary for CH1-TX")
    ("rf.bands.tx[1].max", bpo::value<float>(&args->rf.ch_tx_bands[1].max)->default_value(0), "Higher frequency boundary for CH1-TX")
    ("rf.bands.tx[2].min", bpo::value<float>(&args->rf.ch_tx_bands[2].min)->default_value(0), "Lower frequency boundary for CH2-TX")
    ("rf.bands.tx[2].max", bpo::value<float>(&args->rf.ch_tx_bands[2].max)->default_value(0), "Higher frequency boundary for CH2-TX")
    ("rf.bands.tx[3].min", bpo::value<float>(&args->rf.ch_tx_bands[3].min)->default_value(0), "Lower frequency boundary for CH3-TX")
    ("rf.bands.tx[3].max", bpo::value<float>(&args->rf.ch_tx_bands[3].max)->default_value(0), "Higher frequency boundary for CH3-TX")
    ("rf.bands.tx[4].min", bpo::value<float>(&args->rf.ch_tx_bands[4].min)->default_value(0), "Lower frequency boundary for CH4-TX")
    ("rf.bands.tx[4].max", bpo::value<float>(&args->rf.ch_tx_bands[4].max)->default_value(0), "Higher frequency boundary for CH4-TX")
    //from enb
    ("rf.dl_freq",        bpo::value<float>(&args->rf.dl_freq)->default_value(-1),        "Downlink Frequency (if positive overrides EARFCN)")
    ("rf.ul_freq",        bpo::value<float>(&args->rf.ul_freq)->default_value(-1),        "Uplink Frequency (if positive overrides EARFCN)")



    ("log.all_level", bpo::value<string>(&args->log.all_level)->default_value("info"), "ALL log level")
    ("log.all_hex_limit", bpo::value<int>(&args->log.all_hex_limit)->default_value(32), "ALL log hex dump limit")

    ("log.filename", bpo::value<string>(&args->log.filename)->default_value("/tmp/relay.log"), "Log filename")
    ("log.file_max_size", bpo::value<int>(&args->log.file_max_size)->default_value(-1), "Maximum file size (in kilobytes). When passed, multiple files are created. Default -1 (single file)");
  
  // Positional options - config file location
  bpo::options_description position("Positional options");
  position.add_options()
    ("config_file", bpo::value<string>(&config_file), "RELAY configuration file");
  // clang-format on

  bpo::positional_options_description p;
  p.add("config_file", -1);

  // these options are allowed on the command line
  bpo::options_description cmdline_options;
  cmdline_options.add(common).add(position).add(general);

  // parse the command line and store result in vm
  bpo::variables_map vm;
  try {
    bpo::store(bpo::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
    bpo::notify(vm);
  } catch (bpo::error& e) {
    cerr << e.what() << endl;
    return SRSLTE_ERROR;
  }
  // help option was given - print usage and exit
  if (vm.count("help")) {
    cout << "Usage: " << argv[0] << " [OPTIONS] config_file" << endl << endl;
    cout << common << endl << general << endl;
    exit(SRSLTE_SUCCESS);
  }

  // print version number and exit
  if (vm.count("version")) {
    cout << "Version " << srslte_get_version_major() << "." << srslte_get_version_minor() << "."
         << srslte_get_version_patch() << endl;
    exit(SRSLTE_SUCCESS);
  }

  // if no config file given, check users home path
  if (!vm.count("config_file")) {

    if (!config_exists(config_file, "relay.conf")) {
      cout << "Failed to read RELAY configuration file " << config_file << " - exiting" << endl;
      return SRSLTE_ERROR;
    }
  }
  cout << "Reading configuration file " << config_file << "..." << endl;
  ifstream conf(config_file.c_str(), ios::in);
  if (conf.fail()) {
    cout << "Failed to read configuration file " << config_file << " - exiting" << endl;
    return SRSLTE_ERROR;
  }

  // parse config file and handle errors gracefully
  try {
    bpo::store(bpo::parse_config_file(conf, common), vm);
    bpo::notify(vm);
  } catch (const boost::program_options::error& e) {
    cerr << e.what() << endl;
    return SRSLTE_ERROR;
  }

  return SRSLTE_SUCCESS;
}

int main(int argc, char* argv[])
{
  srslte_register_signal_handler();
  srslte_debug_handle_crash(argc, argv);

  all_args_t args = {};

  cout << "Welcome to srsrelay!" << endl;

  if (int err = parse_args(&args, argc, argv)) {
    return err;
  }
  // Setup logging.
  log_sink = (args.log.filename == "stdout")
                 ? srslog::create_stdout_sink()
                 : srslog::create_file_sink(args.log.filename, fixup_log_file_maxsize(args.log.file_max_size));
  if (!log_sink) {
    return SRSLTE_ERROR;
  }
  srslog::log_channel* chan = srslog::create_log_channel("main_channel", *log_sink);
  if (!chan) {
    return SRSLTE_ERROR;
  }

  srslte::srslog_wrapper log_wrapper(*chan);

  // Start the log backend.
  srslog::init();

  srslte::logmap::set_default_logger(&log_wrapper);
  srslte::log_args(argc, argv, "RELAY");

  srsrelay::relay relay;
  relay.init(args,&log_wrapper);

  return 0;
}
