/*
 * ur_communication.cpp
 *
 * Copyright 2015 Thomas Timm Andersen
 *
 * Licensed under the Apache License, Version 2.0 (the "License";
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

#include "ur_communication.h"
#include <ofMain.h>

#define MODULE_NAME "URComm"
using namespace std;

//--------------------------------------------------------------------------------------
void CloseSocket(const int socket)
{
#ifdef TARGET_WIN32
	closesocket(socket);
#else
	close(socket);
#endif
}

void SetNonBlocking(const int socket, const bool nonBlocking)
{
#ifdef TARGET_WIN32
	unsigned long nonBlock = nonBlocking;
	int retVal = ioctlsocket(socket, FIONBIO, &nonBlock);
#else
	int flags = fcntl(m_hSocket, F_GETFL, 0);
	int retVal;
	if (nonBlocking) {
		retVal = fcntl(socket, F_SETFL, flags | O_NONBLOCK);
	}
	else {
		retVal = fcntl(socket, F_SETFL, flags & ~O_NONBLOCK);
	}
#endif
}

//--------------------------------------------------------------------------------------

UrCommunication::UrCommunication(const std::condition_variable& msg_cond, const std::string& _host, 
	const int _port /*=30001*/) 
 : host(_host)
 , port (_port)
 , keepalive_(false)
 {
	robot_state_ = new RobotState(msg_cond);
	firstClient.setup(host, port);
	//secClient.setup(host, 30002);
}

bool UrCommunication::start() {
	if (!firstClient.isConnected()) {
		ofLogError(MODULE_NAME) << "Error connecting to get firmware version" << endl;
		return false;
	}
	ofLogVerbose(MODULE_NAME) << "Connected to get Firmware version";
	keepalive_ = true;
	uint8_t buf[512];
	unsigned int bytes_read;
	std::string cmd;
	memset(buf, 0, 512);

	bytes_read = firstClient.receiveRawBytes ((char*) buf, 512);

	robot_state_->unpack(buf, bytes_read);
	//wait for some traffic so the UR socket doesn't die in version 3.1.
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ofLog()<<buf;
	char tmp[64];
	sprintf(tmp, "Firmware version detected: %.7f", robot_state_->getVersion());
	ofLog()<<tmp;
// 	firstClient.close();
// 
// 	ofLogNotice(MODULE_NAME)<< "Switching to secondary interface for masterboard data: Connecting...";
// 	if (!secClient.isConnected()) {
// 		ofLogError(MODULE_NAME) << "Error connecting to get masterboard data" << endl;
// 		return false;
// 	}
// 
// 	ofLogNotice(MODULE_NAME) << "Secondary interface: Got connection";
	comThread_ = std::thread(&UrCommunication::run, this);
	return true;
}

void UrCommunication::halt() {
	keepalive_ = false;
	comThread_.join();
}

void UrCommunication::run() {
	uint8_t buf[2048];
	int bytes_read;
	memset(buf, 0, 2048);
	while (keepalive_) {
		while (firstClient.isConnected() && keepalive_) {
			bytes_read = firstClient.receiveRawBytes((char*) buf, 2048); // usually only up to 1295 bytes
			if (bytes_read > 0) {
// 				setsockopt(sec_sockfd_, IPPROTO_TCP, TCP_NODELAY,
// 						(char *) &flag_, sizeof(int));
				robot_state_->unpack(buf, bytes_read);
//                ofLog()<<robot_state_->getTime();
			} else {
                ofLog(OF_LOG_ERROR)<<"disconnect";
				robot_state_->setDisconnected();
				firstClient.close();
			}
		}
		if (keepalive_) {
			//reconnect
			ofLogError(MODULE_NAME)<<"Secondary port: No connection. Is controller crashed? Will try to reconnect in 10 seconds...";
			if (!firstClient.setup(host, port)) {
				ofLogError(MODULE_NAME)<<"Failed opening secondary socket";
			}
			else {
				if (!firstClient.isConnected()) {
					ofLogError(MODULE_NAME, "Error re-connecting to port %i. Is controller started? Will try to reconnect in 10 seconds...", port);
				} else {
					ofLogNotice(MODULE_NAME)<<"Secondary port: Reconnected";
				}
			}
		}
	}

	//wait for some traffic so the UR socket doesn't die in version 3.1.
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	firstClient.close();
}
