#!/bin/bash

phpext="php-emfed.so"
maxconn_prefix="net.core.somaxconn"
msgmnb_prefix="kernel.msgmnb"
maxconn_val=1024
msgmnb_val=10485760
phpini="/etc/php5/apache2/php.ini"
sysctlconf="/etc/sysctl.conf"

echo "start EM install ..."
echo "change tcp conn and ipc msgqueue size limit ..."
sed -i '/net.core.somaxconn/d' $sysctlconf
sed -i '/kernel.msgmnb/d' $sysctlconf
echo "$maxconn_prefix=$maxconn_val"|tee -a $sysctlconf
echo "$msgmnb_prefix=$msgmnb_val"|tee -a $sysctlconf
sysctl -p


echo "chmod files permission ..."
echo "chmod EManager`chmod a+x EManager`"
echo "chmod monitor`chmod a+x monitor`"
echo "chmod em`chmod a+x em`"
echo "chmod root`chmod 600 root`"
echo "mkdir /var/log/EManager `mkdir -p /var/log/EManager`"
echo "chmod /var/log/EManager `chmod 777 -R /var/log/EManager`"

echo "copy files ..."
echo "cp em.conf`cp -p -f em.conf /usr/local/etc`"
echo "cp libphpcpp.so`cp -p -f libphpcpp.so /usr/lib`"
echo "cp php-emfed.so`cp -p -f php-emfed.so /usr/lib/php5/20121212`"
echo "cp EManager`cp -p -f EManager /usr/local/bin`"
echo "cp em`cp -p -f em /etc/init.d/`"
echo "cp monitor `cp -p -f monitor /usr/local/bin`"
echo "cp cron `cp -p -f root /var/spool/cron/crontabs/`"

echo "cron restart `service cron restart >/dev/null 2>&1`"

echo "config php extension..."
#number=`grep $phpext $phpini|wc -l`
#if [ $number -eq 0 ]
#then
# echo "add php.ini extension = $phpext"
# `echo "extension = $phpext"|tee -a $phpini`
# echo "apache restart..."
# service apache2 restart
#else
# echo "nothing"
#fi

sed -i '/php-emfed.so/d' $phpini
echo "add php.ini extension=$phpext"
echo "extension=$phpext"|tee -a $phpini
echo "apache restart... `/etc/init.d/apache2 restart >/dev/null 2>&1`"
#echo "pls run EManager with 'service em restart' or wait a moment..."
echo "Emanager restart..."
service em restart >/dev/null 2>&1
echo "complete EM install ..."
