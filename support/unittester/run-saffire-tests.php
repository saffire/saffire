#!/usr/bin/php
<?php

// PSR0 autoloader
function autoload($className) {
    $className = ltrim($className, '\\');
    $fileName  = '';
    $namespace = '';
    if ($lastNsPos = strripos($className, '\\')) {
        $namespace = substr($className, 0, $lastNsPos);
        $className = substr($className, $lastNsPos + 1);
        $fileName  = str_replace('\\', DIRECTORY_SEPARATOR, $namespace) . DIRECTORY_SEPARATOR;
    }
    $fileName .= str_replace('_', DIRECTORY_SEPARATOR, $className) . '.php';

    require $fileName;
}
spl_autoload_register("autoload");


// Use current dir if nothing has been entered
if (! isset($argv[1]) || $argv[1] == "") {
    print "Usage: ".$argv[0]." <testdir>\n";
    print "\n";
    exit;
}

if (is_dir($argv[1])) {
    $it = new RecursiveDirectoryIterator($argv[1]);
    $it = new RecursiveIteratorIterator($it);
    $it = new RegexIterator($it, '/^.+\.sfu$/i', RecursiveRegexIterator::GET_MATCH);
} else if (is_file($argv[1])) {
    $it = new ArrayIterator(array($argv[1]));
} else {
    print "I don't know what $argv[1] is.\n";
    exit;
}

$suite = new \Saffire\UnitTester($it);
$suite->addOutput(new \Saffire\Output\Console());
$exitcode = $suite->run();
exit($exitcode);
