#include "includes.h"
#include "clientlist.h"
#include "client.h"

CClientList::CClientList ( )
{
}

CClientList::~CClientList ( )
{
    foreach( CClient* client, m_list )
    {
	delete client;
    }
}

void CClientList::Update ( )
{
    foreach(CClient* client, m_list)
    {
	client->Update( );
    }
}

void CClientList::New ( CClient* client )
{
    if(client)
	m_list.push_back ( client );
}

void CClientList::Remove ( CClient* client )
{
    if(client)
    	m_list.remove ( client );
}
