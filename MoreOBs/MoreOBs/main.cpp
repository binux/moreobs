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

uint32_t GetTime( )
{
	return time (NULL);
}

uint32_t GetTicks( )
{
#ifdef WIN32
	// don't use GetTickCount anymore because it's not accurate enough (~16ms resolution)
	// don't use QueryPerformanceCounter anymore because it isn't guaranteed to be strictly increasing on some systems and thus requires "smoothing" code
	// use timeGetTime instead, which typically has a high resolution (5ms or more) but we request a lower resolution on startup

	return timeGetTime( );
#elif __APPLE__
	uint64_t current = mach_absolute_time( );
	static mach_timebase_info_data_t info = { 0, 0 };
	// get timebase info
	if( info.denom == 0 )
		mach_timebase_info( &info );
	uint64_t elapsednano = current * ( info.numer / info.denom );
	// convert ns to ms
	return elapsednano / 1e6;
#else
	uint32_t ticks;
	struct timespec t;
	clock_gettime( CLOCK_MONOTONIC, &t );
	ticks = t.tv_sec * 1000;
	ticks += t.tv_nsec / 1000000;
	return ticks;
#endif
}

void CONSOLE_Print( string message , uint32_t level )
{
	if(level>debug_level)
		return ;

	switch(level)
	{
		case DEBUG_LEVEL_ERROR:
			cout << "<ERROR>  " ;
			break;
		case DEBUG_LEVEL_WARN:
			cout << "<WARN>   " ;
			break;
		case DEBUG_LEVEL_MESSAGE:
			cout << "<MESSAGE>" ;
			break;
		case DEBUG_LEVEL_PACKET:
			cout << "<PACKET> " ;
			break;
	}
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
				Log << "[" << Time << "]";
				switch(level)
				{
					case DEBUG_LEVEL_ERROR:
						Log << "<ERROR>  " ;
						break;
					case DEBUG_LEVEL_WARN:
						Log << "<WARN>   " ;
						break;
					case DEBUG_LEVEL_MESSAGE:
						Log << "<MESSAGE>" ;
						break;
					case DEBUG_LEVEL_PACKET:
						Log << "<PACKET> " ;
						break;
				}
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
				*gLog << "[" << Time << "]" ;
				switch(level)
				{
					case DEBUG_LEVEL_ERROR:
						*gLog << "<ERROR>  " ;
						break;
					case DEBUG_LEVEL_WARN:
						*gLog << "<WARN>   " ;
						break;
					case DEBUG_LEVEL_MESSAGE:
						*gLog << "<MESSAGE>" ;
						break;
					case DEBUG_LEVEL_PACKET:
						*gLog << "<PACKET> " ;
						break;
				}
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

	switch(level)
	{
		case DEBUG_LEVEL_ERROR:
			cout << "<ERROR>  " ;
			break;
		case DEBUG_LEVEL_WARN:
			cout << "<WARN>   " ;
			break;
		case DEBUG_LEVEL_MESSAGE:
			cout << "<MESSAGE>" ;
			break;
		case DEBUG_LEVEL_PACKET:
			cout << "<PACKET> " ;
			break;
		default :
			return ;
	}
	cout << message << endl;
}

void DEBUG_Print( BYTEARRAY b , uint32_t level )
{
	if(level>debug_level)
		return ;

	switch(level)
	{
		case DEBUG_LEVEL_ERROR:
			cout << "<ERROR>\n" ;
			break;
		case DEBUG_LEVEL_WARN:
			cout << "<WARN>\n" ;
			break;
		case DEBUG_LEVEL_MESSAGE:
			cout << "<MESSAGE>\n" ;
			break;
		case DEBUG_LEVEL_PACKET:
			cout << "<PACKET>\n" ;
			break;
		default :
			return ;
	}
	cout << "{ ";

	for( unsigned int i = 0; i < b.size( ); i++ )
		cout << hex << (int)b[i] << " ";

	cout << "}" << endl;
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
	debug_level = cfg.GetInt( "debug_level" , 4 );
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
	CONSOLE_Print( "[GHOST] shutting down" );
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