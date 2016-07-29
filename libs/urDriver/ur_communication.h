/*
 * ur_communication.h
 *
 * Copyright 2015 Thomas Timm Andersen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UR_COMMUNICATION_H_
#define UR_COMMUNICATION_H_

#include "robot_state.h"
#include "do_output.h"
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <mutex>
//#include <condition_variable>
#include <sys/types.h>
#include <iostream>
#include <chrono>
#include <fcntl.h>
#include <sys/types.h>

#include <ofxTCPClient.h>

#ifndef TARGET_WIN32
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#else
#include <Ws2tcpip.h>
#include <WinSock2.h>
#endif

void CloseSocket(const int socket);
void SetNonBlocking(const int socket, bool nonBlocking);

class ofxTCPClient;

class UrCommunication {
	public:
	RobotState* robot_state_;

	UrCommunication(const std::condition_variable& msg_cond, const std::string& host);
	bool start();
	void halt();
private:
	std::string host;
	ofxTCPClient firstClient;// , secClient;
	//int pri_sockfd_, sec_sockfd_;
	//struct sockaddr_in pri_serv_addr_, sec_serv_addr_;
	//struct hostent *server_;
	bool keepalive_;
	std::thread comThread_;
	void run();



};

#endif /* UR_COMMUNICATION_H_ */
