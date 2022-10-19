#ifndef _CLIENTCONNECTION_H
#define _CLIENTCONNECTION_H

#include "HostLock.h"

#define MAXCONN 20

class ClientConnection {
public:
        ClientConnection();
        ~ClientConnection();

        void    Release();
        void    Wait();

        long        _id;
        HostLock    _lock;
        char        _buff[256];
        volatile    bool        _waiting;
};


extern  int     connectionCounter;

extern  ClientConnection *connections[MAXCONN];

extern  HostLock    pendingLock;
extern  ClientConnection *pendingConnections[MAXCONN];

extern  ClientConnection *AddConnection();
extern  void    RemoveConnection(long id);
extern  ClientConnection *BlockConnection(long id);
extern  void        ReleaseConnections();
extern  int     CountPendingConnections();

#endif // _CLIENTCONNECTION_H
