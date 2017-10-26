

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "em.h"
#include "cfg.h"
#include "db.h"
#include "logger.h"

struct st_cfg_map
{
	E_CFG_ITEM item;
	string value;

}cfg_map[CFG_SIZE]=
{
	{E_CFG_D_DEVCODE,"d_devcode"},
	{E_CFG_D_HB,"d_hb"},
	{E_CFG_U_HB,"u_hb"},
	{E_CFG_U_ZMDBZ,"u_zmdbz"},
	{E_CFG_LOGLVL_TERM,"log_lvl_term"},
	{E_CFG_LOGLVL_FILE,"log_lvl_file"}
};



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
	dbConnect();
	//config.Load();
	//config.Assign();
}



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
		//cfg_type = CFG_UNKNOWN;
		//cfg_value.decimal_value = 0;
	}
	accessed = true;
}

bool ConfigItem::BooleanValue() const
{
	if ( !accessed )
		ConvertValue();

	if ( cfg_type != CFG_BOOLEAN )
	{
		printf( "Attempt to fetch boolean value for %s, actual type is %s.\n", name, type );
		return false;
	}

	return( cfg_value.boolean_value );
}

int ConfigItem::IntegerValue() const
{
	if ( !accessed )
		ConvertValue();

	if ( cfg_type != CFG_INTEGER )
	{
		printf( "Attempt to fetch integer value for %s, actual type is %s.\n", name, type );
		return 0;
	}

	return( cfg_value.integer_value );
}

double ConfigItem::DecimalValue() const
{
	if ( !accessed )
		ConvertValue();

	if ( cfg_type != CFG_DECIMAL )
	{
		printf( "Attempt to fetch decimal value for %s, actual type is %s.\n", name, type );
		return 0;
	}

	return( cfg_value.decimal_value );
}

const char *ConfigItem::StringValue() const
{
	if ( !accessed )
		ConvertValue();

	if ( cfg_type != CFG_STRING )
	{
		printf( "Attempt to fetch string value for %s, actual type is %s.\n", name, type );
		return "";
	}

	return( cfg_value.string_value );
}

Config::Config()
{
	//n_items = CFG_SIZE;
	//items = 0;
	//items = new ConfigItem *[n_items];
	m_item = NULL;
}

Config::~Config()
{
/*
	if ( items )
	{
		for ( int i = 0; i < n_items; i++ )
		{
			if(items[i])
			{
				delete items[i];
			}
		}
		delete[] items;
	}
*/
	if(m_item)
	{
		delete m_item;
		m_item = NULL;
	}
}
string Config::findItemName(int id)
{
	for(int idx = 0;idx < sizeof(cfg_map)/sizeof(struct st_cfg_map);idx ++)
	{
		if (id == cfg_map[idx].item)
		{
			return cfg_map[idx].value;
		}
	}
	return "";
}
/*
void Config::Load()
{

      static char sql[SQLBUFSIZE];
   
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
{¡Z
}
*/
const ConfigItem& Config::Item(MYSQL& dbconn,int id )
{
	string itemName = findItemName(id);
	do
	{
		if(!itemName.empty())
		{
			char sql[SQLBUFSIZE];
			snprintf( sql,  sizeof(sql),"select ifnull(Name,''), ifnull(Value,''), ifnull(Type,'') from EM_Config where Name = '%s'",itemName.c_str());
			Info("%s",sql);
			//printf("%s\n",sql);
			if ( mysql_query( &dbconn, sql ) )
			{
				printf( "Can't run query: %s\n", mysql_error( &dbconn ) );
				break ;
			}

			MYSQL_RES *result = mysql_store_result( &dbconn );
			if ( !result )
			{
				printf( "Can't use query result: %s\n", mysql_error( &dbconn ) );
				break ;
			}
			n_items = mysql_num_rows( result );
			if ( n_items  != 1 )
			{
				mysql_free_result( result );
				printf( "get rows num is %d\n", n_items );
				break ;
			}
			MYSQL_ROW dbrow = mysql_fetch_row( result );
			if (mysql_errno( &dbconn ))
			{
				mysql_free_result( result );
				printf( "Can't fetch row: %s\n", mysql_error( &dbconn ) );
				break ;
			}
			if (dbrow )
			{
				if(m_item) 
				{
					delete m_item;
					m_item = NULL;
				}
				m_item = new ConfigItem( dbrow[0], dbrow[1], dbrow[2] );
				mysql_free_result( result );	
				return *m_item;
			}
			mysql_free_result( result );		
			
		}
	}while(0);
	if(m_item) 
	{
		delete m_item;
		m_item = NULL;
	}
	m_item = new ConfigItem("err","err","err");
	return *m_item;
	
}

Config config;

