#ifndef CLIENTLIST_H
#define CLIENTLIST_H

class CClient;

class CClientList
{
private:
    list<CClient*> m_list;
public:
    CClientList ( );
    ~CClientList ( );
    void Update ( );
    void New ( CClient* client );
    void Remove ( CClient* client );
    list<CClient*>& GetList( )	    { return m_list; }
    //CClient* FindClientById ( uint32_t id );
    uint32_t GetSize ( )                 { return m_list.size( ); }
};

#endif
