<?php

$source = file("php://stdin");
$max_lines = count($source);

$stack = array();
$vars = array();
$labels = array();

// Find and cache labels
foreach ($source as $idx => $line) {
    if ($line[0] != "\t") {
        $labels[substr($line, 0, -2)] = $idx;
    }
    //print rtrim($line)."\n";
}
//print "\n\n";

//print_r($labels);

$linenum = 0;
while ($linenum < $max_lines) {
    $line = $source[$linenum];

    // Skip labels
    if ($line[0] != "\t") {
        $linenum++;
        continue;
    }

    // Extract line
    $line = trim($line);
    $tmp = explode("\t", $line);

    $operator = array_shift($tmp);
    switch ($operator) {
        case "compLT" :
            $v1 = array_pop($stack);
            $v2 = array_pop($stack);
            if ($v1[1] < $v2[1]) {
                array_push($stack, array("i", 0));
            } else {
                array_push($stack, array("i", 1));
            }
            break;

        case "compEQ" :
            $v1 = array_pop($stack);
            $v2 = array_pop($stack);
            if ($v1[1] == $v2[1]) {
                array_push($stack, array("i", 1));
            } else {
                array_push($stack, array("i", 0));
            }
            break;

        case "compGT" :
            $v1 = array_pop($stack);
            $v2 = array_pop($stack);
            //print "Comparing: ".$v1[1]." > ".$v2[1].": ";
            if ((int)$v1[1] < (int)$v2[1]) {
                array_push($stack, array("i", 1));
                //print "NGT\n";
            } else {
                array_push($stack, array("i", 0));
                //print "YGT\n";
            }
            break;

        case "jmp" :
            $label = array_shift($tmp);
            $linenum = $labels[$label];
            continue;
            break;

        case "jz" :
            $v1 = array_pop($stack);
            if ($v1[1] == 0) {
                $label = array_shift($tmp);
                $linenum = $labels[$label];
                continue;
            }
            //print "not zero";
            break;

        case "push" :
            $operand = array_shift($tmp);
            if ($operand[0] == "\"") {
                $var = substr($operand, 1, strlen($operand)-2);
                array_push($stack, array("s", (string)$var));
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
        case "inc" :
            $v1 = array_pop($stack);
            array_push($stack, array("i", (int)($v1[1]+1)));
            break;
        case "dec" :
            $v1 = array_pop($stack);
            array_push($stack, array("i", (int)($v1[1]-1)));
            break;

        case "add" :
            $v1 = array_pop($stack);
            $v2 = array_pop($stack);
            array_push($stack, array("i", (int)($v2[1] + $v1[1])));
            break;
        case "sub" :
            $v1 = array_pop($stack);
            $v2 = array_pop($stack);
            array_push($stack, array("i", (int)($v2[1] - $v1[1])));
            break;
        case "print" :
            $v1 = array_pop($stack);
            print $v1[1];
            break;
    }

    $linenum++;
}
print "\n";
