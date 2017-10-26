

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cfg.h"
#include "logger.h"


EMConfig emConfig;
void LoadConfig()
{
	FILE *cfg;
	char line[512];
	char *val;
	if ( (cfg = fopen( EM_CONFIG, "r")) == NULL )
	{
		Fatal( "Can't open %s: %s", EM_CONFIG, strerror(errno) );
	}
	while ( fgets( line, sizeof(line), cfg ) != NULL )
	{
		char *line_ptr = line;

		// Trim off any cr/lf line endings
		int chomp_len = strcspn( line_ptr, "\r\n" );
		line_ptr[chomp_len] = '\0';

		// Remove leading white space
		int white_len = strspn( line_ptr, " \t" );
		line_ptr += white_len;

		// Check for comment or empty line
		if ( *line_ptr == '\0' || *line_ptr == '#' )
			continue;

		// Remove trailing white space
		char *temp_ptr = line_ptr+strlen(line_ptr)-1;
		while ( *temp_ptr == ' ' || *temp_ptr == '\t' )
		{
			*temp_ptr-- = '\0';
			temp_ptr--;
		}

		// Now look for the '=' in the middle of the line
		temp_ptr = strchr( line_ptr, '=' );
		if ( !temp_ptr )
		{
			Warning( "Invalid data in %s: '%s'", EM_CONFIG, line );
			continue;
		}

		// Assign the name and value parts
		char *name_ptr = line_ptr;
		char *val_ptr = temp_ptr+1;

		// Trim trailing space from the name part
		do
		{
			*temp_ptr = '\0';
			temp_ptr--;
		}
		while ( *temp_ptr == ' ' || *temp_ptr == '\t' );

		// Remove leading white space from the value part
		white_len = strspn( val_ptr, " \t" );
		val_ptr += white_len;

		val = (char *)malloc( strlen(val_ptr)+1 );
		strncpy( val, val_ptr, strlen(val_ptr)+1 );

		if ( strcasecmp( name_ptr, "EM_VERSION" ) == 0 )
              	emConfig.VER = val;
		if ( strcasecmp( name_ptr, "EM_DB_HOST" ) == 0 )
              	emConfig.DB_HOST = val;
		else if ( strcasecmp( name_ptr, "EM_DB_NAME" ) == 0 )
              	emConfig.DB_NAME = val;
		else if ( strcasecmp( name_ptr, "EM_DB_USER" ) == 0 )
              	emConfig.DB_USER = val;
		else if ( strcasecmp( name_ptr, "EM_DB_PASS" ) == 0 )
              	emConfig.DB_PASS = val;
		else if ( strcasecmp( name_ptr, "CHECK_INTERVAL" ) == 0 )
          		emConfig.CHK_INTERVAL = atoi(val);
		else if ( strcasecmp( name_ptr, "MISC_INTERVAL" ) == 0 )
          		emConfig.MISC_INTERVAL = atoi(val);		
      		else if ( strcasecmp( name_ptr, "TCP_SRV_NUM" ) == 0 )
      			emConfig.TCP_SRV_NUM = atoi(val);
      		else if ( strcasecmp( name_ptr, "TCP_SRV_PORT" ) == 0 )
			emConfig.TCP_SRV_PORT = atoi(val);	
		else if ( strcasecmp( name_ptr, "UDP_SRV_PORT" ) == 0 )
			emConfig.UDP_SRV_PORT = atoi(val);
		else if ( strcasecmp( name_ptr, "LOG_LVL" ) == 0 )
			emConfig.LOG_LVL = atoi(val);
		else if ( strcasecmp( name_ptr, "TERM_LV" ) == 0 )
			emConfig.TERM_LV = atoi(val);
		else if ( strcasecmp( name_ptr, "FILE_LV" ) == 0 )
			emConfig.FILE_LV = atoi(val);
		else if ( strcasecmp( name_ptr, "DB_LV" ) == 0 )
			emConfig.DB_LV = atoi(val);			
		else if ( strcasecmp( name_ptr, "UDP_SRV_PORT_PHP" ) == 0 )
			emConfig.UDP_SRV_PORT_PHP = atoi(val);		
		else if ( strcasecmp( name_ptr, "EM_HOST" ) == 0 )
			emConfig.EM_HOST = val;
		else
		{
			// We ignore this now as there may be more parameters than the
			// c/c++ binaries are bothered about
			//Warning( "Invalid parameter '%s' in %s", name_ptr, ZM_CONFIG );
		}

	}
	fclose( cfg);
	Debug(1,"TCP_SRV_NUM %d,TCP_SRV_PORT %d,UDP_SRV_PORT %d,LOG_LVL %d,TERM_LV %d,FILE_LV %d,DB_LV %d",
		emConfig.TCP_SRV_NUM,emConfig.TCP_SRV_PORT,emConfig.UDP_SRV_PORT,emConfig.LOG_LVL,emConfig.TERM_LV,emConfig.FILE_LV,emConfig.DB_LV);	
	//dbConnect();
	//config.Load();
	//config.Assign();
}


/*
ConfigItem::ConfigItem( const char *p_name, const char *p_value, const char *const p_type )
{
	name = new char[strlen(p_name)+1];
	strcpy( name, p_name );
	value = new char[strlen(p_value)+1];
	strcpy( value, p_value );
	type = new char[strlen(p_type)+1];
	strcpy( type, p_type );

	//Info( "Created new config item %s = %s (%s)\n", name, value, type );

	accessed = false;
}

ConfigItem::~ConfigItem()
{
	delete[] name;
	delete[] value;
	delete[] type;
}

void ConfigItem::ConvertValue() const
{
	if ( !strcmp( type, "boolean" ) )
	{
		cfg_type = CFG_BOOLEAN;
		cfg_value.boolean_value = (bool)strtol( value, 0, 0 );
	}
	else if ( !strcmp( type, "integer" ) )
	{
		cfg_type = CFG_INTEGER;
		cfg_value.integer_value = strtol( value, 0, 10 );
	}
	else if ( !strcmp( type, "hexadecimal" ) )
	{
		cfg_type = CFG_INTEGER;
		cfg_value.integer_value = strtol( value, 0, 16 );
	}
	else if ( !strcmp( type, "decimal" ) )
	{
		cfg_type = CFG_DECIMAL;
		cfg_value.decimal_value = strtod( value, 0 );
	}
	else
	{
		cfg_type = CFG_STRING;
		cfg_value.string_value = value;
	}
	accessed = true;
}

bool ConfigItem::BooleanValue() const
{
	if ( !accessed )
		ConvertValue();

	if ( cfg_type != CFG_BOOLEAN )
	{
		Error( "Attempt to fetch boolean value for %s, actual type is %s. Try running 'zmupdate.pl -f' to reload config.", name, type );
		exit( -1 );
	}

	return( cfg_value.boolean_value );
}

int ConfigItem::IntegerValue() const
{
	if ( !accessed )
		ConvertValue();

	if ( cfg_type != CFG_INTEGER )
	{
		Error( "Attempt to fetch integer value for %s, actual type is %s. Try running 'zmupdate.pl -f' to reload config.", name, type );
		exit( -1 );
	}

	return( cfg_value.integer_value );
}

double ConfigItem::DecimalValue() const
{
	if ( !accessed )
		ConvertValue();

	if ( cfg_type != CFG_DECIMAL )
	{
		Error( "Attempt to fetch decimal value for %s, actual type is %s. Try running 'zmupdate.pl -f' to reload config.", name, type );
		exit( -1 );
	}

	return( cfg_value.decimal_value );
}

const char *ConfigItem::StringValue() const
{
	if ( !accessed )
		ConvertValue();

	if ( cfg_type != CFG_STRING )
	{
		Error( "Attempt to fetch string value for %s, actual type is %s. Try running 'zmupdate.pl -f' to reload config.", name, type );
		exit( -1 );
	}

	return( cfg_value.string_value );
}

Config::Config()
{
	n_items = 0;
	items = 0;
}

Config::~Config()
{
	if ( items )
	{
		for ( int i = 0; i < n_items; i++ )
		{
			delete items[i];
		}
		delete[] items;
	}
}

void Config::Load()
{
    static char sql[ZM_SQL_SML_BUFSIZ];
   
    strncpy( sql, "select Name, Value, Type from Config order by Id", sizeof(sql) );
	if ( mysql_query( &dbconn, sql ) )
	{
		Error( "Can't run query: %s", mysql_error( &dbconn ) );
		exit( mysql_errno( &dbconn ) );
	}

	MYSQL_RES *result = mysql_store_result( &dbconn );
	if ( !result )
	{
		Error( "Can't use query result: %s", mysql_error( &dbconn ) );
		exit( mysql_errno( &dbconn ) );
	}
	n_items = mysql_num_rows( result );

	if ( n_items <= ZM_MAX_CFG_ID )
	{
		Error( "Config mismatch, expected %d items, read %d. Try running 'zmupdate.pl -f' to reload config.", ZM_MAX_CFG_ID+1, n_items );
		exit( -1 );
	}

	items = new ConfigItem *[n_items];
	for( int i = 0; MYSQL_ROW dbrow = mysql_fetch_row( result ); i++ )
	{
		items[i] = new ConfigItem( dbrow[0], dbrow[1], dbrow[2] );
	}
	mysql_free_result( result );
}

void Config::Assign()
{
ZM_CFG_ASSIGN_LIST
}

const ConfigItem &Config::Item( int id )
{
	if ( !n_items )
	{
		Load();
		Assign();
	}

	if ( id < 0 || id > ZM_MAX_CFG_ID )
	{
		Error( "Attempt to access invalid config, id = %d. Try running 'zmupdate.pl -f' to reload config.", id );
		exit( -1 );
	}

	ConfigItem *item = items[id];
	
	if ( !item )
	{
		Error( "Can't find config item %d", id );
		exit( -1 );
	}
		
	return( *item );
}

Config config;
*/
