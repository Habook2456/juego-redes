#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>
#include <map>
#include <sstream>
#include <thread>
#include <chrono>
#include <iomanip>

#define STR_LENGTH 4096

using namespace std;

int posMap = 0;

struct Client
{
    int socket;
    std::string nickname;
    int avatar;
};

// clients vector
vector<Client> clients;

struct playerInfo
{
    int posX, posY, posMap;
};

void printAllClients()
{
    std::cout << "Lista de clientes:\n";
    for (const auto &client : clients)
    {
        std::cout << "SocketID: " << client.socket << ", Nickname: " << client.nickname << "\n";
    }
}

void handleClient(int clientSocket)
{
    char nickname[100];
    memset(nickname, 0, sizeof(nickname));
    // recibir identificador de cliente (nickname)
    size_t n = recv(clientSocket, nickname, sizeof(nickname), 0);
    nickname[n] = '\0';

    // obtener numero de cliente
    int clientNum = clients.size();
    clients.push_back({clientSocket, std::string(nickname), clientNum++});
    std::cout << "Cliente '" << nickname << "' ha iniciado sesión." << std::endl;

    // obtener avatar de cliente (numero de cliente - ASCII)
    char avatar;
    avatar = '0' + clientNum;
    int valorAscii = static_cast<int>(avatar);
    send(clientSocket, &valorAscii, sizeof(valorAscii), 0);

    // send avatar to all clients
    string blockAVATAR = 'N' + to_string(valorAscii);
    for (const auto &otherClient : clients)
    {
        if (otherClient.socket != clientSocket)
        {
            send(otherClient.socket, blockAVATAR.c_str(), blockAVATAR.size(), 0);
        }
    }

    std::cout << "GAME STARTED" << std::endl;
    while (1)
    {
        playerInfo receivedPlayer;
        recv(clientSocket, reinterpret_cast<char *>(&receivedPlayer), sizeof(receivedPlayer), 0);
        //std::cout << "Player (" << nickname << "): " << receivedPlayer.posX << ", " << receivedPlayer.posY << ", " << receivedPlayer.posMap << std::endl;

        // send to all clients posMap (map position)
        string blockPOSMAP = 'M' + to_string(posMap);
        for (const auto &otherClient : clients)
        {
            send(otherClient.socket, blockPOSMAP.c_str(), blockPOSMAP.size(), 0);
        }
        posMap++;
        if (posMap > 112)
        {
            posMap = 0;
        }

        // send to all clients posPlayer (player position)
        string blockPOSPLAYER = 'P' + to_string(valorAscii) + ',' + to_string(receivedPlayer.posX) + ',' + to_string(receivedPlayer.posY);
        for (const auto &otherClient : clients)
        {
            if (otherClient.socket != clientSocket)
            {
                std::cout << "Sending from " << nickname << " to " << otherClient.nickname << ": " << blockPOSPLAYER << std::endl;
                send(otherClient.socket, blockPOSPLAYER.c_str(), blockPOSPLAYER.size(), 0);
            }
        }
    }

    printAllClients();
}

int main(void)
{
    system("clear");
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1)
    {
        perror("Socket");
        exit(1);
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(45000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cerr << "Error al vincular el socket a la dirección" << std::endl;
        close(serverSocket);
        return -1;
    }

    if (listen(serverSocket, 10) == -1)
    {
        std::cerr << "Error al intentar escuchar conexiones entrantes" << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Esperando conexiones entrantes..." << std::endl;

    while (1)
    {
        int clientSocket;
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket == -1)
        {
            std::cerr << "Error al aceptar la conexión entrante" << std::endl;
            continue;
        }

        std::thread(handleClient, clientSocket).detach();
    }

    close(serverSocket);
    return 0;
}