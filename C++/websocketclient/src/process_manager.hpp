/*
 * process_manager.hpp
 *
 *  Created on: 2015年1月14日
 *      Author: root
 */

#ifndef PROCESS_MANAGER_HPP_
#define PROCESS_MANAGER_HPP_

#include <sstream>
#include <cstdlib>
#include <unistd.h>

#include "common/utils.h"
#include "common/vclient_common.h"

class process_manager {
public:
    static int create_process(const std::string &ip, const ushort_t port)
    {
        pid_t pid;
        pid = vfork();
        switch(pid)
        {
        case -1:
            LOG_ERR("fork faild");
            return 0;
        case 0:
        {
            if(port == 5004) {
                std::stringstream vlc_param;
                vlc_param << "file:///" << ip << ":" << port << "/1.jpg";
                LOG_DEBUG(vlc_param.str());
                execlp("vlc", "vlc", vlc_param.str().c_str(), "--no-video-title-show", "--no-mouse-events", "-f", "--loop", NULL);
            } else {
                std::stringstream vlc_param;
                vlc_param << "rtp://" << ip << ":" << port;
                LOG_DEBUG(vlc_param.str());
                execlp("vlc", "vlc", "-f", vlc_param.str().c_str(), "--no-video-title-show", "--no-mouse-events", "-f", NULL);
            }
        }
        }
        return pid;
    }

    static int create_process(const std::string &cmd)
    {
        pid_t pid;
        pid = fork();
        switch(pid)
        {
        case -1:
            LOG_ERR("fork faild");
            return 0;
        case 0:
        {
            setenv("LANG", "en_US.UTF-8",1);
            system(cmd.c_str());
            LOG_DEBUG("update finished!");
            exit(0);
        }
        }
        return pid;
    }

    static void stop_process(const int pid)
    {
        kill(pid, SIGKILL);
    }
};

#endif /* PROCESS_MANAGER_HPP_ */
