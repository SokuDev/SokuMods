//
// Created by Gegel85 on 05/04/2019.
//

#ifndef DISC_ORD_SOCKET_HPP
#define DISC_ORD_SOCKET_HPP


#if defined _WIN32
#	include <winsock.h>
#else
#	include <sys/socket.h>
#	define INVALID_SOCKET -1
	typedef int SOCKET;
#endif
#include <string>
#include <map>

//! @brief Define a Socket
class Socket {
public:
	//! @brief Define a http request payload.
	struct HttpRequest {
		std::string httpVer; //!< The http version
		std::string body; //!< The body of the request
		std::string method; //!< The method of the request (put, get, etc.)
		std::string host; //!< The host to contact
		unsigned ip;
		int portno; //!< The port number to contact the host
		std::map<std::string, std::string> header; //!< The header of the request (entry: value)
		std::string path; //!< The url to fetch
	};

	//! @brief Define a http response payload.
	struct HttpResponse {
		HttpRequest request; //!< The request payload
		std::map<std::string, std::string> header; //!< The header of the request (entry: value)
		int returnCode; //!< The http-return code
		std::string codeName; //!< The name of the return code
		std::string httpVer; //!< The http version
		std::string body; //!< The body of the response
	};

	//! @brief Construct a Socket.
	Socket(SOCKET sockfd, struct sockaddr_in addr);

	//! @brief Construct a Socket.
	Socket();
	Socket(const Socket &socket);
	Socket &operator=(const Socket &socket);

	//! @brief Socket Destructor.
	virtual ~Socket();

	//! @brief Get the status of the Socket.
	//! @return The status of the Socket.
	bool isOpen() const;

	//! @brief Connect the Socket to a host.
	//! @param host The host to connect to.
	//! @param portno The port number used to connect to the host.
	virtual void connect(const std::string &host, unsigned short portno);

	//! @brief Connect the Socket to an ip.
	//! @param ip The ip to connect to.
	//! @param portno The port number used to connect to the host.
	virtual void connect(unsigned int ip, unsigned short portno);

	//! @brief Disconnect the Socket.
	virtual void disconnect();

	//! @brief Send a message
	//! @param msg The message to send.
	virtual void send(const std::string &msg);

	//! @brief Read the Socket buffer.
	//! @param size How much must be read.
	//! @return std::string
	virtual std::string read(int size);

	//! @brief Read the Socket buffer until it is empty.
	//! @return std::string
	virtual std::string readUntilEOF();

	//! @brief Generate a http payload from a HttpRequest
	//! @param request The request to generate
	//! @return std::string
	static std::string generateHttpRequest(const HttpRequest &request);

	//! @brief Generate a http payload from a HttpRequest
	//! @param request The request to generate
	//! @return std::string
	static std::string generateHttpResponse(const HttpResponse &response);

	//! @brief Create a http response from a HttpRequest
	//! @param request The request to generate
	//! @return HttpResponse
	HttpResponse makeHttpRequest(const HttpRequest &request);

	//! @brief Create a basic http request from scratch.
	//! @param host Host to connect to.
	//! @param portno Port number used to connect to the host.
	//! @param content The content of the request.
	//! @return std::string
	std::string makeRawRequest(const std::string &host, unsigned short portno, const std::string &content);

	//! @brief Parse the response.
	//! @param respon The string to parse
	//! @return HttpResponse
	static HttpResponse parseHttpResponse(const std::string &respon);

	//! @brief Parse the request.
	//! @param respon The string to parse
	//! @return HttpRequest
	static HttpRequest parseHttpRequest(const std::string &requ);

	void bind(unsigned short port);

	Socket accept();

	//! @brief Return the socket value.
	//! @return SOCKET
	SOCKET getSockFd() const { return this->_sockfd; };

	const sockaddr_in &getRemote() const;

	void setNoDestroy(bool noDestroy) const;

	bool isDisconnected() const;

protected:
	mutable bool _noDestroy = false;
	SOCKET _sockfd = INVALID_SOCKET; //!< The socket
	mutable bool _opened = false; //!< The status of the socket.
	struct sockaddr_in _remote;
};

#endif //DISC_ORD_SOCKET_HPP
