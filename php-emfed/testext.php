<head><meta http-equiv="refresh" content="2"></head>
<?php
    // define a user space function
    function some_function($param)
    {
        echo("userspace function called with $param\n");
    }

    function test1($param,$param2)
    {
        //echo("userspace function called with $param\n");
	//echo("param=$param,param2=$param2\n")
	echo("1-->$param,2-->$param2");
    }

    // example input
    $input = array(
        'x' =>  10,
        'y' =>  20,
        'z' =>  30
    );
    
  // test(function() {
        //echo("lambda function called with param $param1 $param2\n");
  //      echo("test success");
  //  });
   UnsolMsg("test1");
  //UnsolMsg(function() {
        //echo("lambda function called with param $param1 $param2\n");
	//	echo("test success");
    //});    
    //example_function($input, function($param1, $param2) {
    //    echo("lambda function called with param $param1 $param2\n");
    //});
?>
