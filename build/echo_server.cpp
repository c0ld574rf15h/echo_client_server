#include <glog/logging.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <set>
#include "utility.h"
using namespace std;

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    google::LogToStderr();

    if(argc < 2 || argc > 3 || (argc == 3 && memcmp(argv[2], "-b", 2))) {
        LOG(ERROR) << "Usage : " << argv[0] << " Port Number [-b]";
        return ERROR;
    }

    LOG(INFO) << "Initializing Server";

    // 1. Create Master Socket ( Listening Socket )
    int master_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (master_sd == -1) {
		LOG(ERROR) << "socket() returned an error";
		return ERROR;
	}

	int optval = 1;
	setsockopt(master_sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

    // 2. Bind Address Information
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(addr.sin_zero, ZERO_PAD, sizeof(addr.sin_zero));
    
    if(bind(master_sd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr)) == -1) {
        LOG(ERROR) << "bind() returned an error";
        close(master_sd); return ERROR;
    }

    // 3. Set Socket to Listening Mode
    if(listen(master_sd, BACKLOG) == -1) {
        LOG(ERROR) << "listen() returned an error";
        close(master_sd); return ERROR;
    }

    // Initialize Master fd Set
    fd_set master_set, working_set;
    FD_ZERO(&master_set);
    int max_sd = master_sd;
    FD_SET(master_sd, &master_set);

    // Set Timetout
    struct timeval timeout;
    timeout.tv_sec = 3*MINUTES;
    timeout.tv_usec = 0;

    // 4. Receive clients or messages
    set<int> fd_set;
    fd_set.insert(master_sd);

    while(true) {
        memcpy(&working_set, &master_set, sizeof(master_set));
        switch(select(max_sd+1, &working_set, NULL, NULL, &timeout)) {
            case SELECT_FAIL:
                LOG(ERROR) << "select() returned an error";
                return ERROR;
            case TIMEOUT_EXPIRED:
                LOG(ERROR) << "Timeout Expired";
                return ERROR;
            default:
                for(auto it=fd_set.begin();it!=fd_set.end();++it) {
                    if(FD_ISSET(*it, &working_set)) {
                        int c_sd = 0;
                        if(*it == master_sd) {
                            //do {
                            struct sockaddr_in c_addr;
                            socklen_t c_len = sizeof(sockaddr);
                            c_sd = accept(master_sd, reinterpret_cast<struct sockaddr*>(&c_addr), &c_len);
                            if(c_sd == -1) {
                                LOG(INFO) << "End Reaping Clients";
                                break;
                            }
                            fd_set.insert(c_sd);
                            LOG(INFO) << "New Incoming Connection - " << c_sd;
                            FD_SET(c_sd, &master_set);
                            max_sd = max(max_sd, c_sd);
                            //} while(c_sd != -1);
                        } else {
                            //do {
                            char buf[MAX_MSGLEN];
                            int rcv = recv(*it, buf, MAX_MSGLEN, 0);
                            buf[rcv] = '\0';
                            switch(rcv) {
                                case -1:
                                    LOG(INFO) << "Something got wrong while receiving message";
                                    break;
                                case CLOSE_CONN:
                                    LOG(WARNING) << "Connection has been closed by client - " << *it;
                                    fd_set.erase(*it);
                                default:
                                    LOG(INFO) << rcv << " Bytes Received [Client #" << *it << "] : " << buf;
                                    if(send(*it, buf, rcv, 0) < 0) {
                                        LOG(ERROR) << "send() returned an error";
                                        return ERROR;
                                    }
                            }
                            //} while(true);
                        }
                    }
                }
        }
    }
    for(auto it = fd_set.begin(); it != fd_set.end(); ++it)
        if(FD_ISSET(*it, &master_set))
            close(*it);

    return 0;
}