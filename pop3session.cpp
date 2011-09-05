
#include "pop3session.h"

#include <iostream>

#include "socket.h"

Pop3Session::Pop3Session()
    : socket(NULL)
{}

Pop3Session::Pop3Session(std::string const& server, int const& port)
{
    socket = new Socket(server, port);
}

Pop3Session::~Pop3Session()
{
    releaseSocket();
}

void Pop3Session::releaseSocket()
{
    if (socket != NULL)
    {
        delete socket;
    }
}

void Pop3Session::sendCommand(std::string const& command)
{
    socket->write(command);
}

void Pop3Session::getResponse(ServerResponse* response)
{
    std::string buffer;
    socket->readLine(&buffer);

    if (buffer[0] == '+')
    {
        response->status = true;
        buffer.erase(0, 3); // Remove the "+OK "
    }
    else
    {
        response->status = false;
        buffer.erase(0, 4); // Remove the "-ERR "
    }

    response->statusMessage = buffer;
    response->data.clear();
}

void Pop3Session::getMultilineResponse(ServerResponse* response)
{
    std::string buffer;
    getResponse(response);

    while (true)
    {
        buffer.clear();
        socket->readLine(&buffer);
        
        if (buffer == "." || buffer.length() == 0)
        {
            break;
        }
        else
        {
            if (buffer[0] == '.') /* Strip byte stuffed characters. */
            {
                buffer.erase(0, 1);
            }

            response->data.push_back(buffer);
        }
    }
}

void Pop3Session::authenticate(std::string const& username, std::string const& password)
{
    ServerResponse response;

    sendCommand("USER " + username);
    getResponse(&response);

    if (!response.status)
    {
        throw ServerError("Authentication failed", response.statusMessage);
    }

    sendCommand("PASS " + password);
    getResponse(&response);

    if (!response.status)
    {
        throw ServerError("Authentication failed", response.statusMessage);
    }
}

void Pop3Session::printMessageList()
{
    ServerResponse response;

    sendCommand("LIST");
    getMultilineResponse(&response);

    int spacePosition = 0;
    for (std::list<std::string>::iterator line = response.data.begin();
         line != response.data.end();
         line++)
    {
        spacePosition = line->find(' ');
        std::cout << line->substr(0, spacePosition);
    }
}
