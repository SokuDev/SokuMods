#ifndef DISCXXORD_EXCEPTION_HPP
#define DISCXXORD_EXCEPTION_HPP


#include <string>
#include "Network/Socket.hpp"

//! @brief Define the BaseException. All exceptions must be inherited from here.
class BaseException : public std::exception {
private:
	std::string _msg; //!< Message of the exception

public:
	//! @brief Constructor of the BaseException
	//! @param msg Exception message
	explicit BaseException(const std::string &&msg) : _msg(msg)	{};

	//! @brief what function
	//! @return char* The message of the exception
	const char *what() const noexcept override {
		return this->_msg.c_str();
	};

	~BaseException() override = default;
};

//! @brief Define a NetworkException.
class NetworkException : public BaseException {
public:
	//! @brief Create a NetworkException with a message.
	//! @param msg The error message.
	explicit NetworkException(const std::string &&msg) : BaseException(static_cast<const std::string &&>(msg)) {};
};

//! @brief Define a SocketCreationErrorException.
class SocketCreationErrorException : public NetworkException {
public:
	//! @brief Create a SocketCreationErrorException with a message.
	//! @param msg The error message.
	explicit SocketCreationErrorException(const std::string &&msg) : NetworkException("SocketCreationErrorException: " + static_cast<const std::string &&>(msg)) {};
};

//! @brief Define a HostNotFoundException.
class HostNotFoundException : public NetworkException {
public:
	//! @brief Create a HostNotFoundException with a message.
	//! @param msg The error message.
	explicit HostNotFoundException(const std::string &&msg) : NetworkException(static_cast<const std::string &&>(msg)) {};
};

//! @brief Define a ConnectException.
class ConnectException : public NetworkException {
public:
	//! @brief Create a ConnectException with a message.
	//! @param msg The error message.
	explicit ConnectException(const std::string &&msg) : NetworkException(static_cast<const std::string &&>(msg)) {};
};

//! @brief Define a NotConnectedException.
class NotConnectedException : public NetworkException {
public:
	//! @brief Create a NotConnectedException with a message.
	//! @param msg The error message.
	explicit NotConnectedException(const std::string &&msg) : NetworkException(static_cast<const std::string &&>(msg)) {};
};

//! @brief Define a AlreadyOpenedException.
class AlreadyOpenedException : public NetworkException {
public:
	//! @brief Create a AlreadyOpenedException with a message.
	//! @param msg The error message.
	explicit AlreadyOpenedException(const std::string &&msg) : NetworkException(static_cast<const std::string &&>(msg)) {};
};

//! @brief Define a EOFException.
class EOFException : public NetworkException {
public:
	//! @brief Create a EOFException with a message.
	//! @param msg The error message.
	explicit EOFException(const std::string &&msg) : NetworkException("EOFException: " + static_cast<const std::string &&>(msg)) {};
};

//! @brief Define a EOFException.
class BindFailedException : public NetworkException {
public:
	//! @brief Create a EOFException with a message.
	//! @param msg The error message.
	explicit BindFailedException(const std::string &&msg) : NetworkException("BindFailedException: " + static_cast<const std::string &&>(msg)) {};
};

//! @brief Define a EOFException.
class ListenFailedException : public NetworkException {
public:
	//! @brief Create a EOFException with a message.
	//! @param msg The error message.
	explicit ListenFailedException(const std::string &&msg) : NetworkException("ListenFailedException: " + static_cast<const std::string &&>(msg)) {};
};

//! @brief Define a EOFException.
class AcceptFailedException : public NetworkException {
public:
	//! @brief Create a EOFException with a message.
	//! @param msg The error message.
	explicit AcceptFailedException(const std::string &&msg) : NetworkException("AcceptFailedException: " + static_cast<const std::string &&>(msg)) {};
};

//! @brief Define a EOFException.
class WSAStartupFailedException : public NetworkException {
public:
	//! @brief Create a EOFException with a message.
	//! @param msg The error message.
	explicit WSAStartupFailedException(const std::string &&msg) : NetworkException("WSAStartupFailedException: " + static_cast<const std::string &&>(msg)) {};
};

//! @brief Define a EOFException.
class NotImplementedException : public NetworkException {
public:
	//! @brief Create a EOFException with a message.
	//! @param msg The error message.
	explicit NotImplementedException() : NetworkException("Not implemented") {};
};

//! @brief Define a EOFException.
class AbortConnectionException : public NetworkException {
private:
	unsigned short _code;

public:
	//! @brief Create a EOFException with a message.
	//! @param msg The error message.
	explicit AbortConnectionException(unsigned short code) : NetworkException(std::to_string(code)), _code(code) {};

	unsigned short getCode() const { return _code; };
};

//! @brief Define a InvalidHTTPAnswerException.
class InvalidHTTPAnswerException : public NetworkException {
public:
	//! @brief Create a InvalidHTTPAnswerException with a message.
	//! @param msg The error message.
	explicit InvalidHTTPAnswerException(const std::string &&msg) : NetworkException(static_cast<const std::string &&>(msg)) {};
};

class InvalidHandshakeException : public NetworkException {
public:
	//! @param msg The error message.
	explicit InvalidHandshakeException(const std::string &&msg) : NetworkException(static_cast<const std::string &&>(msg)) {};
};

class InvalidPongException : public NetworkException {
public:
	//! @param msg The error message.
	explicit InvalidPongException(const std::string &&msg) : NetworkException(static_cast<const std::string &&>(msg)) {};
};

class CryptFailedException : public NetworkException {
public:
	//! @param msg The error message.
	explicit CryptFailedException(const std::string &&msg) : NetworkException(msg + ": " + std::to_string(GetLastError())) {};
};

class ConnectionTerminatedException : public NetworkException {
private:
	unsigned _code;

public:
	//! @param msg The error message.
	ConnectionTerminatedException(const std::string &&msg, unsigned code) : NetworkException(static_cast<const std::string &&>(msg)), _code(code) {};
	unsigned getCode() const { return this->_code; };
};

//! @brief Define a HTTPErrorException.
class HTTPErrorException : public NetworkException {
private:
	Socket::HttpResponse _response;

public:
	//! @brief Create a HTTPErrorException with a message.
	//! @param response The response from a Socket.
	HTTPErrorException(const Socket::HttpResponse &response) :
		NetworkException(response.request.host + " responded with code " + std::to_string(response.returnCode) + " " + response.codeName),
		_response(response)
	{}

	//! @brief Return the response of the last Socket Exception.
	//! @return Socket::HttpResponse
	Socket::HttpResponse getResponse() const { return this->_response; }
};


#endif //DISCXXORD_EXCEPTION_HPP
