<?php

define("PADDING", "        ");

/*
 * Reads and displays saffire bytecode file
 */

$fh = fopen($argv[1], "rb");
$data = fread($fh, 8*4);

$header = unpack ("Vmagic/".
                  "Vtimestamp/".
                  "Vflags/".
                  "Vbytecode_len/".
                  "Vbytecode_uncompressed_len/".
                  "Vbytecode_offset/".
                  "Vsignature_len/".
                  "Vsignature_offset", $data);

if ($header['magic'] != 0x43424653) {
    printf("incorrect header found\n");
    exit;
}

printf("-- Header -----------------------------------------------------------------\n");
printf("  Magic                : 0x%08X\n", $header['magic']);
printf("  Sourcefile timestamp : ". date("d-m-Y h:i:s", $header['timestamp'])."\n");
printf("  Flags                : 0x%08X\n", $header['flags']);
echo "\n";
printf("  Bytecode len         : %5d  (0x%08X)\n", $header['bytecode_len'], $header['bytecode_len']);
printf("  Bytecode uncomp len  : %5d  (0x%08X)\n", $header['bytecode_uncompressed_len'], $header['bytecode_uncompressed_len']);
printf("  Bytecode off         : %5d  (0x%08X)\n", $header['bytecode_offset'], $header['bytecode_offset']);
printf("  Signature len        : %5d  (0x%08X)\n", $header['signature_len'], $header['signature_len']);
printf("  Signature off        : %5d  (0x%08X)\n", $header['signature_offset'], $header['signature_offset']);
printf("---------------------------------------------------------------------------\n");
echo "\n";

fseek($fh, $header['bytecode_offset'], SEEK_SET);
$bincode = fread($fh, $header['bytecode_len']);
$bincode = bzdecompress($bincode);

parse($bincode, 0);
exit;


function parse($bincode, $level) {
    printf(str_repeat(PADDING, $level) . "-- Bytecode info ----------------------------------------------------------\n");
    $tmp = unpack ("Vstacksize", $bincode);
    $bincode = substr($bincode, 4);
    printf(str_repeat(PADDING, $level) . "  Stack size          : %5d (0x%08X)\n", $tmp['stacksize'], $tmp['stacksize']);

    $tmp = unpack ("Vcodesize", $bincode);
    $bincode = substr($bincode, 4);
    $code = substr($bincode, 0, $tmp['codesize']);
    $bincode = substr($bincode, $tmp['codesize']);
    printf(str_repeat(PADDING, $level) . "  Code size           : %5d (0x%08X)\n", $tmp['codesize'], $tmp['codesize']);

    $tmp = unpack ("Vconstcount", $bincode);
    $bincode = substr($bincode, 4);
    $cc = $tmp['constcount'];
    printf(str_repeat(PADDING, $level) . "  Constant count     : %5d (0x%08X)\n", $cc, $cc);

    for ($i=0; $i!=$cc; $i++) {
        $tmp = unpack("Ctype/Vlen", $bincode);
        $bincode = substr($bincode, 1 + 4);
        $c = new StdClass;
        $c->type = $tmp['type'];
        $c->len = $tmp['len'];
        $c->data = substr($bincode, 0, $tmp['len']);
        $bincode = substr($bincode, $tmp['len']);
        $const[] = $c;
    }


    $tmp = unpack ("Vidcount", $bincode);
    $bincode = substr($bincode, 4);
    $idcount = $tmp['idcount'];
    printf(str_repeat(PADDING, $level) . "  Identifier size     : %5d (0x%08X)\n", $tmp['idcount'], $tmp['idcount']);
    for ($i=0; $i!=$idcount; $i++) {
        $tmp = unpack("Vlen", $bincode);
        $bincode = substr($bincode, 4);
        $c = new StdClass;
        $c->len = $tmp['len'];
        $c->data = substr($bincode, 0, $tmp['len']);
        $bincode = substr($bincode, $tmp['len']);
        $id[] = $c;
    }

    $tmp = unpack ("Vlinosize", $bincode);
    $bincode = substr($bincode, 4);
    $linos = substr($bincode, 0, $tmp['linosize']);
    $bincode = substr($bincode, $tmp['linosize']);
    printf(str_repeat(PADDING, $level) . "  Lineno size         : %5d (0x%08X)\n", $tmp['linosize'], $tmp['linosize']);
    printf(str_repeat(PADDING, $level) . "---------------------------------------------------------------------------\n");
    echo "\n";

    printf(str_repeat(PADDING, $level) . "-- Uncompressed bytecode --------------------------------------------------\n");
    hexdump($code, str_repeat(PADDING, $level));
    printf(str_repeat(PADDING, $level) . "---------------------------------------------------------------------------\n");
    echo "\n";

    printf(str_repeat(PADDING, $level) . "-- Constants --------------------------------------------------------------\n");
    foreach ($const as $k => $c) {
        switch ($c->type) {
            case 0 : // String
                printf(str_repeat(PADDING, $level) . "#%d String: %s\n", $k+1, $c->data);
                break;
            case 1 : // Numerical
                printf(str_repeat(PADDING, $level) . "#%d Numerical: %d (0x%08X)\n", $k+1, $c->data, $c->data);
                break;
            case 2 : // Code
                printf(str_repeat(PADDING, $level) . "#%d Code: \n", $k+1);
                parse($c->data, $level + 1);
                break;
        }
    }
    printf(str_repeat(PADDING, $level) . "---------------------------------------------------------------------------\n");
    echo "\n";

    printf(str_repeat(PADDING, $level) . "-- Identifiers ------------------------------------------------------------\n");
    foreach ($id as $i) {
        hexdump($i->data, str_repeat(PADDING, $level));
    }
    printf(str_repeat(PADDING, $level) . "---------------------------------------------------------------------------\n");
    echo "\n";

    printf(str_repeat(PADDING, $level) . "-- Linenos ----------------------------------------------------------------\n");
    hexdump($linos, str_repeat(PADDING, $level));
    printf(str_repeat(PADDING, $level) . "---------------------------------------------------------------------------\n");
    echo "\n";

}




exit;





function hexdump ($data, $prefix = "") {
    $hexi   = '';
    $ascii  = '';
    $dump   = '';
    $offset = 0;
    $len    = strlen($data);

    for ($i = $j = 0; $i < $len; $i++) {
        $hexi .= sprintf("%02X ", ord($data[$i]));

        $ascii .= (ord($data[$i]) >= 32) ? $data[$i] : '.';

        if ($j === 3 || $j === 7 || $j === 11) {
            $hexi  .= ' ';
        }

        if (++$j === 16 || $i === $len - 1) {
            $dump .= sprintf("%04X  %-51s  %s", $offset, $hexi, $ascii);

            $hexi   = $ascii = '';
            $offset += 16;
            $j      = 0;

            if ($i !== $len - 1) $dump .= "\n" . $prefix;
        }
    }

    $dump .= "\n";
    echo $prefix . $dump;
}
