--TEST--
get_complex_array
--SKIPIF--
<?php if (!extension_loaded("extension_for_tests")) print "skip"; ?>
--FILEEOF--
<?php

/*
function get_complex_array()
{
    return array(
        "a" => 123,
        "b" => 456,
        "c" => array("nested value","example",7)
    );
}
var_export(get_complex_array());
*/
var_export(TestVariables\get_complex_array());


echo PHP_EOL;
--EXPECT--
array (
  'a' => 123,
  'b' => 456,
  'c' => 
  array (
    0 => 'nested value',
    1 => 'example',
    2 => 7,
  ),
)