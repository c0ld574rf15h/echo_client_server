#include "utility.h"
using namespace std;

void recv_msg(int sd) {
    char buf[MAX_MSGLEN];
    while(true) {
        int len = recv(sd, buf, MAX_MSGLEN-1, 0);
        buf[min(len, MAX_MSGLEN)] = '\0';
        if(len == -1 || len == 0) {
            LOG(WARNING) << "Something has gone woopy";
            break;
        }
        LOG(INFO) << "Echo message from server : " << buf;
    }
}

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    google::LogToStderr();

    if(argc != 3) {
        LOG(ERROR) << "Usage : " << argv[0] << " <Server Address> <Port Number>";
        return ERROR;
    }

    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1) {
        LOG(ERROR) << "socket() returned an error";
        return ERROR;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    memset(addr.sin_zero, ZERO_PAD, sizeof(addr.sin_zero));

    if(connect(sd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr)) == -1) {
        LOG(ERROR) << "connect() returned an error";
        return ERROR;
    }

    LOG(INFO) << "Connected to Server";

    thread recv_thread(recv_msg, sd);
    recv_thread.detach();

    while(true) {
        char buf[MAX_MSGLEN];
        int len = read(0, buf, MAX_MSGLEN-1);
        buf[min(MAX_MSGLEN-1, len)] = '\0';

        if(!strncmp(buf, "exit", 4)) {
            LOG(WARNING) << "Exiting program";
            break;
        }
        if(send(sd, buf, strlen(buf), 0) == -1) {
            LOG(ERROR) << "send() returned an error";
            return ERROR;
        }
    }

    close(sd);
    return 0;
}