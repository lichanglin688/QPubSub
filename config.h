#pragma once

//#define BROKER_TCP 0 // Define this to use TCP, otherwise it will use local sockets

#ifdef BROKER_TCP
#include <QTcpServer>
#include <QTcpSocket>
using SocketServer = QTcpServer;
using SocketDescriptor = qintptr;
using Socket = QTcpSocket;
using SocketErrorType = QAbstractSocket::SocketError;
#else
#include <QLocalServer>
#include <QLocalSocket>
using SocketServer = QLocalServer;
using SocketDescriptor = quintptr;
using Socket = QLocalSocket;
using SocketErrorType = QLocalSocket::LocalSocketError;
#endif // BROKER_TCP
