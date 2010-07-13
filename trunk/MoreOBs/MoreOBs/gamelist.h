#ifndef GAMELIST_H
#define GAMELIST_H

class CGame;

class CGameList
{
private:
	list<CGame*> m_list;
	uint32_t m_id;
public:
	CGameList ( );
	~CGameList ( );
	void Update ( );
	uint32_t NewGame ( CGame* game );
	bool RemoveGame ( CGame* game );
	CGame* FindGameById ( uint32_t id );
	list<CGame*>& GetGameList ( )		{ return m_list; }
	uint32_t GetSize ( )				{ return m_list.size( ); }
};

#endif