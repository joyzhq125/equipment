
#ifndef CFG_H
#define CFG_H
#include <mysql/mysql.h>
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
    short CHK_INTERVAL;
    short MISC_INTERVAL;
    short TCP_SRV_NUM;
    short TCP_SRV_PORT;
    short  UDP_SRV_PORT;
    short  LOG_LVL;
    short  TERM_LV;
    short  FILE_LV;
    short  DB_LV;

}EMConfig;

extern EMConfig emConfig;

#define CFG_SIZE   100

typedef enum
{
	E_CFG_D_DEVCODE,  //down
	E_CFG_D_HB,
		
	E_CFG_U_HB,            //up
	E_CFG_U_ZMDBZ,
	E_CFG_LOGLVL_TERM,
	E_CFG_LOGLVL_FILE,
	E_CFG_RESERVE
}E_CFG_ITEM;



class ConfigItem
{
private:
	char *name;
	char *value;
	char *type;

	mutable enum { CFG_BOOLEAN, CFG_INTEGER, CFG_DECIMAL, CFG_STRING,CFG_UNKNOWN } cfg_type;
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

private:
	int n_items;
	//ConfigItem **items;
	ConfigItem * m_item;

public:
	Config();
	~Config();

	//void Load();
	//void Assign();
	const ConfigItem& Item(MYSQL& dbconn,int id );
	string findItemName(int id);
};

extern Config config;

#endif // CONFIG_H
