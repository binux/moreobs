#ifndef INCLUDES_H
#define INCLUDES_H

//boost

#include <boost/foreach.hpp>
#include <boost/cstdint.hpp>

typedef boost::int8_t          int8_t          ;
typedef boost::int_least8_t    int_least8_t    ;
typedef boost::int_fast8_t     int_fast8_t     ;

typedef boost::uint8_t         uint8_t         ;
typedef boost::uint_least8_t   uint_least8_t   ;
typedef boost::uint_fast8_t    uint_fast8_t    ;

typedef boost::int16_t         int16_t         ;
typedef boost::int_least16_t   int_least16_t   ;
typedef boost::int_fast16_t    int_fast16_t    ;

typedef boost::uint16_t        uint16_t         ;
typedef boost::uint_least16_t  uint_least16_t   ;
typedef boost::uint_fast16_t   uint_fast16_t    ;

typedef boost::int32_t         int32_t         ;
typedef boost::int_least32_t   int_least32_t   ;
typedef boost::int_fast32_t    int_fast32_t    ;

typedef boost::uint32_t        uint32_t        ;
typedef boost::uint_least32_t  uint_least32_t  ;
typedef boost::uint_fast32_t   uint_fast32_t   ;


#define foreach BOOST_FOREACH

// STL

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>
//#include <set>
#include <string>
#include <vector>
#include <list>

using namespace std;

typedef vector<unsigned char> BYTEARRAY;

// time

uint32_t GetCTime( );		// ctime
uint32_t GetTime( );		// seconds
uint32_t GetTicks( );		// milliseconds

// output
#define DEBUG_LEVEL_ERROR       1
#define DEBUG_LEVEL_WARN        2
#define DEBUG_LEVEL_MESSAGE     3
#define DEBUG_LEVEL_PACKET      4
#define DEBUG_LEVEL_PACKET_RECV 5
#define DEBUG_LEVEL_PACKET_SEND 6

void CONSOLE_Print( string message , uint32_t level = DEBUG_LEVEL_MESSAGE );
void DEBUG_Print( string message , uint32_t level = DEBUG_LEVEL_MESSAGE );
void DEBUG_Print( BYTEARRAY b , uint32_t level = DEBUG_LEVEL_MESSAGE );

#endif