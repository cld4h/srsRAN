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
