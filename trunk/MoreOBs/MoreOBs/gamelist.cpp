#include "includes.h"
#include "gamelist.h"
#include "game.h"

CGameList::CGameList ( )
{
	m_id = 1000;
}

CGameList::~CGameList ( )
{
}

void CGameList::Update ( )
{
	foreach( CGame* game, m_list )
	{
		if( game->GetState( ) == STATUS_BROKEN )
		{
			RemoveGame( game );
			delete game;
		}
	}
}

uint32_t CGameList::NewGame ( CGame* game )
{
	m_list.push_back( game );
	return m_id++;
}

bool CGameList::RemoveGame ( CGame* game )
{
	m_list.remove( game );
	return true;
}

CGame* CGameList::FindGameById ( uint32_t id )
{
	foreach( CGame* game, m_list)
	{
		if(game->GetId( ) == id )
			return game;
	}

	return NULL;
}