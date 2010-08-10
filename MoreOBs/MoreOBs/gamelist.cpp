#include "includes.h"
#include "gamelist.h"
#include "game.h"

CGameList::CGameList ( )
{
    m_id = 1;
}

CGameList::~CGameList ( )
{
    foreach( CGame* game, m_list)
    {
	delete game;
    }
    m_list.clear( );
}

void CGameList::Update ( )
{
    foreach( CGame* game, m_list )
    {
        if( game->GetDeleteReady( ) )
        {
            Remove( game );
            delete game;
        }
	else
	{
	    game->Update( );
	}
    }
}

uint32_t CGameList::New ( CGame* game )
{
    m_list.push_back( game );
    return m_id++;
}

bool CGameList::Remove ( CGame* game )
{
    m_list.remove( game );
    return true;
}

CGame* CGameList::FindGameById ( uint32_t id )
{
    foreach( CGame* game, m_list)
    {
        if(game->GetId( ) == id && !game->GetDelete( ) )
            return game;
    }

    return NULL;
}
