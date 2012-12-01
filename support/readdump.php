<?php

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
echo "\n";

printf("-- Uncompressed bytecode (minus code/stack lengths)-----------------------\n");
fseek($fh, $header['bytecode_offset'], SEEK_SET);
$bincode = fread($fh, $header['bytecode_len']);
$bincode = bzdecompress($bincode);
$bincode = substr($bincode, 8);
hexdump($bincode, false, true, false);
printf("---------------------------------------------------------------------------\n");
exit;

function hexdump ($data) {
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

            if ($i !== $len - 1) $dump .= "\n";
        }
    }

    $dump .= "\n";
    echo $dump;
}
