#include "ClientConnection.h"
#include "HostThread.h"
#include <stdio.h>

int     connectionCounter;

ClientConnection *connections[MAXCONN];

HostLock    pendingLock;
ClientConnection *pendingConnections[MAXCONN];
int     nPendingConnections;


extern  int     mainLoopIteration;
extern  bool    quitting;

ClientConnection *AddConnection()
{
        int     i;
        
        for(i = 0; i < MAXCONN; ++i) {
            if(connections[i] == 0) {
                ClientConnection *conn = new ClientConnection();
                connections[i] = conn;
                return conn;
            }
        }
        return 0;
}

void    RemoveConnection(long id)
{
        int     i;

        for(i = 0; i < MAXCONN; ++i) {
            ClientConnection *conn = connections[i];
            if(conn && conn->_id == id) {
                connections[i] = 0;
                break;
            }
        }
}


ClientConnection *BlockConnection(long id)
{
        int     i;
        int     j;

        if(quitting)
            return 0;
        for(i = 0; i < MAXCONN; ++i) {
            ClientConnection *conn = connections[i];
            if(conn && conn->_id == id) {
                pendingLock.Lock();
                for(j = 0; j < MAXCONN; ++j) {
                    if(pendingConnections[j] == 0) {
                        pendingConnections[j] = conn;
                        conn->_waiting = true;
                        ++nPendingConnections;
                        pendingLock.Unlock();
                        conn->Wait();
                        return conn;
                    }
                }
                pendingLock.Unlock();
                return 0;
            }
        }
        return 0;
}

void        ReleaseConnections()
{
        int     i;

        pendingLock.Lock();
        for(i = 0; i < MAXCONN; ++i) {
            ClientConnection *conn = pendingConnections[i];
            if(!conn)
                continue;
            conn->Release();
            pendingConnections[i] = 0;
            --nPendingConnections;
        }
        pendingLock.Unlock();
}


int     CountPendingConnections()
{
        return nPendingConnections;
}

ClientConnection::ClientConnection()
{
        _id = ++connectionCounter;
}


ClientConnection::~ClientConnection()
{
        _waiting = false;
}


void    ClientConnection::Release()
{
        sprintf(_buff, "%d\n", mainLoopIteration);
        printf("Releasing %d\n", _id);
        _waiting = false;
}

void    ClientConnection::Wait()
{
        printf("Blocking %d\n", _id);
        while(_waiting && !quitting) {
            HostThread::Sleep(200);
        }
}
