//
// Created by Gegel85 on 04/11/2020.
//

#include "getPublicIp.hpp"

#include "../Exceptions.hpp"
#include "../logger.hpp"
#include "Socket.hpp"

char *myIp = nullptr;

const char *getMyIp() {
	if (myIp)
		return myIp;
	logMessage("Fetching public IP\n");

	try {
		Socket sock;
		Socket::HttpRequest request{
			/*.body  */ "",
			/*.method*/ "GET",
			/*.host  */ "www.sfml-dev.org",
			/*.portno*/ 80,
			/*.header*/ {},
			/*.path  */ "/ip-provider.php",
		};
		auto response = sock.makeHttpRequest(request);

		if (response.returnCode != 200)
			throw HTTPErrorException(response);
		myIp = strdup(response.body.c_str());
		logMessagef("My ip is %s\n", myIp);
		return myIp;
	} catch (NetworkException &e) {
		logMessagef("Error: %s\n", e.what());
		throw;
	}
}