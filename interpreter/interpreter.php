<?php

$source = <<< 'EOB'
	push	10
	pop	~"$a"
	push	~"$a"
	print
	push	"10"
	pop	~"$b"
	push	~"$b"
	print
	push	5
	push	1
	add
	pop	~"$a"
	push	~"$a"
	print
	push	~"$a"
	push	1
	add
	pop	~"$a"
	push	~"$a"
	print
	push	~"$a"
	push	1
	add
	pop	~"$a"
	push	~"$a"
	print
	push	1
	push	~"$a"
	sub
	pop	~"$a"
	push	~"$a"
	print
EOB;

/**
 This is the byte code for the following source:
 $a = 10;
 print $a;

 $b = "10";
 print $b;


 $a = 5 + 1;
 print $a;

 $a = $a + 1;
 print $a;

 $a = $a + 1;
 print $a;

 $a = $a - 1;
 print $a;
**/


$stack = array();
$vars = array();

foreach (explode("\n", $source) as $line) {
    $tmp = explode("\t", $line);
    array_shift($tmp);

    $operator = array_shift($tmp);

    switch ($operator) {
        case "push" :
            $operand = array_shift($tmp);
            if ($operand[0] == "\"") {
                array_push($stack, array("s", (string)$operand));
            } elseif ($operand[0] == "~") {
                $var = substr($operand, 2, strlen($operand)-3);
                array_push($stack, $vars[$var]);
            } else {
                array_push($stack, array("i", (int)$operand));
            }
            break;
        case "pop" :
            $v1 = array_pop($stack);
            $operand = array_shift($tmp);
            if ($operand[0] == "~") {
                $var = substr($operand, 2, strlen($operand)-3);
                $vars[$var] = $v1;
            }
            break;
        case "add" :
            $v1 = array_pop($stack);
            $v2 = array_pop($stack);
            array_push($stack, array("i", (int)($v1[1] + $v2[1])));
            break;
        case "sub" :
            $v1 = array_pop($stack);
            $v2 = array_pop($stack);
            array_push($stack, array("i", (int)($v1[1] - $v2[1])));
            break;
        case "print" :
            $v1 = array_pop($stack);
            print "PRINT '".$v1[1]."'\n";
            break;
    }
}

