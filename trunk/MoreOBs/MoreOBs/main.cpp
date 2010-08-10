#include "includes.h"
#include "config.h"
#include "moreobs.h"

#include <boost/program_options.hpp>

#ifdef WIN32
 //#include <windows.h>
 #include <MMSystem.h>
#endif

#include <time.h>

#ifndef WIN32
 #include <sys/time.h>
#endif

#ifdef __APPLE__
 #include <mach/mach_time.h>
#endif

//===================================================================================================================
uint32_t debug_level;

string gLogFile;
uint32_t gLogMethod;
ofstream *gLog = NULL;
CMoreObs *gMoreObs = NULL;

uint32_t GetCTime( )
{
	return time( NULL );
}

uint32_t GetTime( )
{
	return GetTicks( ) / 1000;
}

uint32_t GetTicks( )
{
	return clock( );
}

void CONSOLE_Print( string message , uint32_t level )
{
	if(level>debug_level)
		return ;

	cout << level << ">";
	cout << message << endl;

	// logging

	if( !gLogFile.empty( ) )
	{
		if( gLogMethod == 1 )
		{
			ofstream Log;
			Log.open( gLogFile.c_str( ), ios :: app );

			if( !Log.fail( ) )
			{
				time_t Now = time( NULL );
				string Time = asctime( localtime( &Now ) );

				// erase the newline
				Time.erase( Time.size( ) - 1 );
				Log << level << ">";
				Log << "[" << Time << "]";
				Log << message << endl;
				Log.close( );
			}
		}
		else if( gLogMethod == 2 )
		{
			if( gLog && !gLog->fail( ) )
			{
				time_t Now = time( NULL );
				string Time = asctime( localtime( &Now ) );

				// erase the newline
				Time.erase( Time.size( ) - 1 );
				*gLog << level << ">";
				*gLog << "[" << Time << "]" ;
				*gLog << message << endl;
				gLog->flush( );
			}
		}
	}
}

void DEBUG_Print( string message , uint32_t level )
{
	if(level>debug_level)
		return ;

	cout << level << ">";
	cout << message << endl;
}

void DEBUG_Print( BYTEARRAY b , uint32_t level )
{
	if(level>debug_level)
		return ;

	switch(level)
	{
		case DEBUG_LEVEL_PACKET_RECV:
			cout << "<<<IN\n" ;
			break;
		case DEBUG_LEVEL_PACKET_SEND:
			cout << "   OUT>>>\n" ;
			break;
		default :
			cout << level << "\n" ;
			break;
	}

	unsigned int i = 0;
	while( i < b.size( ) )
	{
		switch(level)
		{
			case DEBUG_LEVEL_PACKET_RECV:
				cout << "" ;
				break;
			case DEBUG_LEVEL_PACKET_SEND:
				cout << "   " ;
				break;
			default :
				cout << level << ">" ;
				break;
		}

		unsigned int j;
		for( j = 0; j < 8; j++ )
		{
			if(j+i < b.size( ))
				cout << setfill('0') << setw(2) << hex << (int)b[i+j] << " ";
			else
				cout << "   ";
		}
		cout << " ";
		for( j = 8; j < 16; j++ )
		{
			if(j+i < b.size( ))
				cout << setfill('0') << setw(2) << hex << (int)b[i+j] << " ";
			else
				cout << "   ";
		}
		cout << "  ";

		for( j = 0; j < 16 && i < b.size( ); j++,i++ )
		{
			if(j % 8 == 0)
				cout << " ";
			if((int)b[i] >= 32 && (int)b[i] < 127)
				cout << b[i];
			else
				cout << ".";
		}
		cout << endl;
	}
}

//=============================================================================================================

int main(int argc, char* argv[])
{
	string cfgFile;

	try{
        namespace po = boost::program_options;

        po::options_description generic("Generic options");
        generic.add_options()
            ("version,v", "print version string")
            ("help,h", "help message")    
            ("about,a", "about me")
            ;

        po::options_description config("Configuration");
        config.add_options()
			("config,c",po::value<string>(&cfgFile)->default_value("moreobs.cfg"),"config file")
            ;

        po::options_description cmdline_options;
        cmdline_options.add(generic).add(config);

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, cmdline_options),vm);
		po::notify(vm);

        if (vm.count("help")) {
            cout << cmdline_options << "\n";
            return 1;
        }
        if (vm.count("version")) {
            cout << "Version: " << "0.2.1079" << "\n";
            return 1;
        }
        if (vm.count("about")) {
            cout<< "Waaagh!TV Client Simulation Server (personal)"<<endl;
            cout<< "Author: binux@CN" <<endl <<"Version: " <<"0.2.1079" <<endl
                << "Build: 2010-7-9"<<endl <<"This program is based on Waaagh!TV. All information about the protocol is guessing by looking at the network traffic. Thanks to guys for your useful programs."<<endl;
            return 1;
        }
        if (cfgFile!="moreobs.cfg" && vm.count("config")) {
            cout << "Using config file :" << cfgFile << endl;
        }
    } catch(boost::system::system_error& e) {
		DEBUG_Print(string("[MAIN]") + e.what() , DEBUG_LEVEL_ERROR);
        return 1;
    }

//============================================================================================================

	CConfig cfg;
	cfg.Read(cfgFile);
	debug_level = cfg.GetInt( "debug_level" , 99 );
	gLogFile = cfg.GetString( "log", string( ) );
	gLogMethod = cfg.GetInt( "logmethod", 1 );

	if( !gLogFile.empty( ) )
	{
		if( gLogMethod == 1 )
		{
			// log method 1: open, append, and close the log for every message
			// this works well on Linux but poorly on Windows, particularly as the log file grows in size
			// the log file can be edited/moved/deleted while GHost++ is running
		}
		else if( gLogMethod == 2 )
		{
			// log method 2: open the log on startup, flush the log for every message, close the log on shutdown
			// the log file CANNOT be edited/moved/deleted while GHost++ is running

			gLog = new ofstream( );
			gLog->open( gLogFile.c_str( ), ios :: app );
		}
	}

	CONSOLE_Print( "[MAIN] starting up" );

	if( !gLogFile.empty( ) )
	{
		if( gLogMethod == 1 )
			CONSOLE_Print( "[MAIN] using log method 1, logging is enabled and [" + gLogFile + "] will not be locked" , DEBUG_LEVEL_MESSAGE );
		else if( gLogMethod == 2 )
		{
			if( gLog->fail( ) )
				CONSOLE_Print( "[MAIN] using log method 2 but unable to open [" + gLogFile + "] for appending, logging is disabled" , DEBUG_LEVEL_WARN );
			else
				CONSOLE_Print( "[MAIN] using log method 2, logging is enabled and [" + gLogFile + "] is now locked" , DEBUG_LEVEL_MESSAGE );
		}
	}
	else
		CONSOLE_Print( "[MAIN] no log file specified, logging is disabled" , DEBUG_LEVEL_WARN);

	gMoreObs = new CMoreObs( &cfg );

	//runing
	gMoreObs->Run();

	//shutdown
	CONSOLE_Print( "[MAIN] shutting down", DEBUG_LEVEL_MESSAGE );
	delete gMoreObs;
	gMoreObs = NULL;

	if( gLog )
	{
		if( !gLog->fail( ) )
			gLog->close( );

		delete gLog;
	}

	return 0;
}
