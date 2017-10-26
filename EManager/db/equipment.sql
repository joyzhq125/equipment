--
-- Current Database: `equipment`
--

-- CREATE DATABASE /*!32312 IF NOT EXISTS*/ `equipment`;

USE `equipment`;

--
-- Table structure for table `sb_ssdy`
--

DROP TABLE IF EXISTS `sb_ssdy`;
CREATE TABLE `sb_ssdy` (
  `ssid` int unsigned NOT NULL auto_increment,
  `sbbm` varchar(20),
  `mc` varchar(64),
  `jd` dec(10,6),
  `wd` dec(10,6),
  `sslx` smallint,
  `sszlx` smallint,
  `ssfz` smallint,
  `sszl` smallint,
  `ssqy` smallint,
  `ssbm` smallint,
  `bmdj` smallint,
  `wltgs` tinyint,
   `bz`     text,
   PRIMARY KEY (`ssid`)
) ;

--
-- Table structure for table `sb_jgzj`
--

DROP TABLE IF EXISTS `sb_jgzj`;
CREATE TABLE `sb_jgzj` (
  `sbid` int unsigned NOT NULL auto_increment,
  `sbbm` varchar(20),
  `ssssid` int,
   `jqm` varchar(20),
   `ip`  varchar(20),
   `dk`  tinyint, 
  `yhm` varchar(20),
   `mm` varchar(20),
   `dzm` smallint,
   `mc`  varchar(64), 
  `jd` dec(10,6),
  `wd` dec(10,6),
  `sblx` smallint,
  `whdw` smallint,
  `ssfz` smallint,
  `sszl` smallint,
  `jcfs` tinyint,
  `sbzt` tinyint,
  `bjsz` char(1),
  `gjcs` smallint,
  `wlzt` tinyint,
  `gzzt` tinyint,
  `czzt` tinyint,
  `jcgxsj` datetime,
  `xtjg`  smallint,
   `bz`     text,
   PRIMARY KEY (`sbid`)
) ;

--
-- Table structure for table `sb_sbdy`
--

DROP TABLE IF EXISTS `sb_sbdy`;
CREATE TABLE `sb_sbdy` (
  `sbid` int unsigned NOT NULL auto_increment,
  `ssid` int,
  `ssssid` int,
  `dzm`   smallint,
  `tdh`  tinyint,
   `jd` dec(10,6),
   `wd` dec(10,6),
   `mc`  varchar(64), 
   `sblx` tinyint,  
   `sbzlx` smallint,
   `sscl`  smallint,
   `ssclh` tinyint,
   `ssfz`  smallint,
    `sszl` smallint,
    `sbjb` smallint,
    `sbzt` tinyint,
    `ssqy` smallint,
    `ssbm` smallint,
    `bmdj` smallint,
    `whdw` smallint,
    `wltgs` smallint,
    `sbtgs` smallint,
    `jcfs`  tinyint,
    `bjsz`  char(1),
    `gjcs`  smallint,
    `wlzt`  tinyint,
    `gzzt`  tinyint,
    `czzt`  tinyint,
    `jcgxsj` datetime,
    `xtjg`   smallint,
   `bz`     text,
   PRIMARY KEY (`sbid`)
) ;


--
-- Table structure for table `yw_xm`
--

DROP TABLE IF EXISTS `yw_xm`;
CREATE TABLE `yw_xm` (
  `xmid` int unsigned NOT NULL auto_increment,
  `xmbm` varchar(20),
   `cd`  tinyint,
  `xmmc` varchar(64),
   `lb`   tinyint,
   `lx`   tinyint,
   `sx`    int,
   `xx`    int,
   `jqlb`  smallint,
   `jqjb`  tinyint,
   `sxjs`  text,
   `xxjs`  text,
   `hfqr` char(1),
   `bjsz`  char(1),
   `zt`    char(1),
   `bz`    text,
   PRIMARY KEY (`xmid`)
);


--
-- Table structure for table `yw_ml`
--

DROP TABLE IF EXISTS `yw_ml`;
CREATE TABLE `yw_ml` (
  `xmid` int unsigned NOT NULL auto_increment,
  `xmbm` varchar(20),
  `cd`   tinyint,
  `xmmc` varchar(64),
  `lx`   tinyint,
   `fhjg` char(1),
   `zt`   char(1),
   `bz`    text,
   PRIMARY KEY (`xmid`)
) ;


--
-- Table structure for table `yw_txsj`
--

DROP TABLE IF EXISTS `yw_txsj`;
CREATE TABLE `yw_txsj` (
  `jlid` int unsigned NOT NULL auto_increment,
  `sbid` int,
   `sbmc` varchar(64),
   `ssid` int,
   `ssmc`  varchar(64),
   `ssfz`  varchar(32),
   `sszl`  varchar(32),
   `sblx`  varchar(32),
   `ip`    varchar(20),
   `tdh`   tinyint,
   `txlx`  varchar(20),
   `xmid`  int,
   `xmmc`  varchar(64),
   `zh`    int,
   `sx`    int,
   `xx`    int,
   `sjz`   int,
   `txsj`  datetime,
   PRIMARY KEY (`jlid`)
) ;

--
-- Table structure for table `yw_czsj`
--
DROP TABLE IF EXISTS `yw_czsj`;
CREATE TABLE `yw_czsj` (
  `jlid` int unsigned NOT NULL auto_increment,
  `sbid` int,
   `sbmc` varchar(64),
   `ssid` int,
   `ssmc`  varchar(64),
   `ssfz`  varchar(32),
   `sszl`  varchar(32),
   `sblx`  varchar(32),
   `ip`    varchar(20),
   `tdh`   tinyint,
   `czxmid`  int,
   `czmc`   varchar(64),
   `czml`   varchar(20),
   `sjz`    int,
   `czsj`   datetime,
   PRIMARY KEY (`jlid`)
) ;


--
-- Table structure for table `yw_jqgl`
--

DROP TABLE IF EXISTS `yw_jqgl`;
CREATE TABLE `yw_jqgl` (
  `jlid` int unsigned NOT NULL auto_increment,
  `sbid` int,
   `sbmc` varchar(64),
   `ssid` int,
   `ssmc`  varchar(64),
   `ssfz`  varchar(32),
   `sszl`  varchar(32),
   `sblx`  varchar(32),
   `ip`    varchar(20),
   `tdh`   tinyint,
   `jqlb`  varchar(32),
   `jqjb`  varchar(32),
   `jslr`  text,
   `czzt`  varchar(32),
   `jqsj`  datetime,
   PRIMARY KEY (`jlid`)
) ;


--
-- Table structure for table `yw_jqcl`
--

DROP TABLE IF EXISTS `yw_jqcl`;
CREATE TABLE `yw_jqcl` (
  `clid` int unsigned NOT NULL auto_increment,
  `sbid` int,
  `sbmc` varchar(64),
  `ssid` int,
   `ssmc`  varchar(64),
   `ssfz`  varchar(32),
   `sszl`  varchar(32),
   `sblx`  varchar(32),
   `ip`    varchar(20),
   `tdh`   tinyint,
   `gjcs`  smallint,
   `wlzt`  tinyint,
   `gzzt`  tinyint,
   `czfs`  tinyint,
   `clsj`  datetime,
   PRIMARY KEY (`clid`)
) ;