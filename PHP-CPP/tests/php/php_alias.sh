#!/bin/bash
#!/bin/bash
#
# this only alias to comand "/usr/bin/php -d enable_dl=On -d extension_dir=../ext_dir -d extension=extfortest.so"
# Designed to simplify debugging of tests
# You can get the same thing using normal aliases: `alias php_alias="php -d enable_dl=On -d extension_dir=../ext_dir -d extension=extfortest.so"`
# Exemlple toese: $ ./php_alias.sh dbg.php
#

if [ ! $1 ]
then
	echo "Required the path to php file"
	exit;
fi

LD_LIBRARY_PATH="$(cd $PWD/../.. && echo $PWD):${LD_LIBRARY_PATH}"
export LD_LIBRARY_PATH

/usr/bin/php -d extension_dir=../ext_dir -d extension=extfortest.so -d ini7=47 -d ini1=ReValIni1 -d ini2=ReValIni2 $1 $2 $3 $4 $5
