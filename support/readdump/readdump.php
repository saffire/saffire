<?php

use bytecode\Dumper;

// PSR0 magic
spl_autoload_register('autoload');

$bc = new bytecode\Dumper($argv[1]);
if ($bc->isValid()) {
    $bc->dump();
} else {
    print("File ".$argv[1]." is not valid Saffire bytecode.\n");
}
exit;


function autoload($className)
{
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
