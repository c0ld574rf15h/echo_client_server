#pragma once

#include <arpa/inet.h>
#include <glog/logging.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#include <cstdlib>
#include <set>
#include <thread>

#define ZERO_PAD            0   
#define ERROR               -1
#define BACKLOG             32
#define MINUTES             60

#define SELECT_FAIL         -1
#define TIMEOUT_EXPIRED     0
#define MAX_MSGLEN          100

#define CLOSE_CONN          0