
#ifndef CFG_H
#define CFG_H

#include <string>
using namespace std;

#define EM_CONFIG	"/usr/local/etc/em.conf"	// Path to config file


extern void LoadConfig();

typedef struct 
{
    string VER;
    string DB_HOST;
    string DB_NAME;
    string DB_USER;
    string DB_PASS;
    unsigned short CHK_INTERVAL;
    unsigned short MISC_INTERVAL;
    unsigned short TCP_SRV_NUM;
    unsigned short TCP_SRV_PORT;
    unsigned short  UDP_SRV_PORT;
    unsigned short  UDP_SRV_PORT_PHP;
    unsigned short  LOG_LVL;
    unsigned short  TERM_LV;
    unsigned short  FILE_LV;
    unsigned short  DB_LV;
    string  EM_HOST;

}EMConfig;

extern EMConfig emConfig;

/*
class ConfigItem
{
private:
	char *name;
	char *value;
	char *type;

	mutable enum { CFG_BOOLEAN, CFG_INTEGER, CFG_DECIMAL, CFG_STRING } cfg_type;
	mutable union
	{
		bool boolean_value;
		int integer_value;
		double decimal_value;
		char *string_value;
	} cfg_value;
	mutable bool accessed;

public:
	ConfigItem( const char *p_name, const char *p_value, const char *const p_type );
	~ConfigItem();
	void ConvertValue() const;
	bool BooleanValue() const;
	int IntegerValue() const;
	double DecimalValue() const;
	const char *StringValue() const;

	inline operator bool() const
	{
		return( BooleanValue() );
	}
	inline operator int() const
	{
		return( IntegerValue() );
	}
	inline operator double() const
	{
		return( DecimalValue() );
	}
	inline operator const char *() const
	{
		return( StringValue() );
	}
};

class Config
{
public:
	ZM_CFG_DECLARE_LIST

private:
	int n_items;
	ConfigItem **items;

public:
	Config();
	~Config();

	void Load();
	void Assign();
	const ConfigItem &Item( int id );
};

extern Config config;
*/
#endif // ZM_CONFIG_H
