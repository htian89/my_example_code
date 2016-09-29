/*
 * main.cpp
 *
 *  Created on: 2014年12月11日
 *      Author: root
 */

#include "tcp_server.h"

#include <iostream>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/settings.hpp>
#include <boost/log/utility/setup/from_stream.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <fstream>

namespace logging = boost::log;
using namespace std;

static tcp_server *ipc_server = NULL;

static int log_init()
{
#if  __cplusplus >= 201103L
    std::ifstream file(DEBUG_CONFIG_FILE);
#else
    std::ifstream file(DEBUG_CONFIG_FILE.c_str());
#endif
    if(file.is_open())
    {
        logging::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");
        logging::expressions::format_named_scope("Scopes", logging::keywords::format = "%n (%f : %l)");
        logging::core::get()->add_global_attribute("Scope", logging::attributes::named_scope());
        logging::init_from_stream(file);
        logging::add_common_attributes();
    }
#if 0
    {
        logging::add_file_log("websocket_client.log");
        boost::log::add_file_log(
                boost::log::keywords::file_name = "websocket_client.log",
                boost::log::keywords::open_mode = std::ios_base::in,
                boost::log::keywords::max_size = 16*1024*1024,
                boost::log::keywords::format =
                        "[%TimeStamp%][%ThreadID%]: %Message%",
                boost::log::keywords::auto_flush = true
                );
    }
#endif
    LOG_DEBUG("log start");
}

#include <sys/wait.h>
static void sigchld_handler(int signo)
{
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    LOG_DEBUG("pid = "  << pid << "; status = " << status);
    if(pid > 0 && ipc_server != NULL)
    {
        ipc_server->get_websocket_interface()->clear_process_id(pid);
    }
}

static void signal_init()
{
    signal(SIGCHLD,  sigchld_handler);
}

int main(void)
{
    log_init();
    signal_init();
    for(;;)
    {
        VERIFY_THROW(ipc_server = new tcp_server(IPC_SERVER_PORT, 10),  {delete ipc_server; sleep(5); continue;});
        break;
    }
    ipc_server->start();
    ipc_server->run();
    delete ipc_server;
    return 0;
}
