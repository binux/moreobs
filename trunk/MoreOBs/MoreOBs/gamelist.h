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
	uint32_t New ( CGame* game );
	bool Remove ( CGame* game );
	CGame* FindGameById ( uint32_t id );
	list<CGame*>& GetList ( )		{ return m_list; }
	uint32_t GetSize ( )				{ return m_list.size( ); }
};

#endif
