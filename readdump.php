<?php

$fh = fopen($argv[1], "rb");
$data = fread($fh, 7*4);

$header = unpack ("Vmagic/".
                  "Vtimestamp/".
                  "Vversion/".
                  "Vconst_count/".
                  "Vconst_off/".
                  "Vclass_count/".
                  "Vclass_off", $data);

print_r($header);

if ($header['magic'] != 0x43424653) {
    printf("incorrect header found\n");
    exit;
}

printf("------------------------------\n");
printf("Magic                : 0x%08X\n", $header['magic']);
printf("Sourcefile timestamp : ". date("d-m-Y h:i:s", $header['timestamp'])."\n");
printf("Constant count       : ". $header['const_count']." (0x%08X)\n", $header['const_count']);
printf("Constant offset      : ". $header['const_off']." (0x%08X)\n", $header['const_off']);
printf("Class count          : ". $header['class_count']." (0x%08X)\n", $header['class_count']);
printf("Class offset         : ". $header['class_off']." (0x%08X)\n", $header['class_off']);

printf("------------------------------\n");
for ($i=0; $i!=$header['const_count']; $i++) {
    $data = fread($fh, 5);
    $const_header = unpack ("Vlength/Ctype", $data);

    printf("Type   : ". getConstType($const_header['type'])." (".$const_header['length'].") ");

    switch ($const_header['type']) {
        case 0 :
            break;
        case 1 :    // String
        case 4 :    // Regex
            $data = fread($fh, $const_header['length']);
            printf(": '".$data."'");
            break;
        case 2 :
            $data = fread($fh, $const_header['length']);
            $tmp = unpack ("Vvalue", $data);
            printf(": %d (0x%08X)", $tmp['value'], $tmp['value']);
            break;
        case 3 :
            $data = fread($fh, $const_header['length']);
            $tmp = unpack ("Vvalue", $data);
            printf(": %s", $tmp['value'] == 0 ? "false" : "true");
            break;
    }

    //fseek($fh, $const_header['length'], SEEK_CUR);

    printf("\n");
}

fclose($fh);
exit;



function getConstType($type) {
    switch ($type) {
        case 0 : return "null";
        case 1 : return "string";
        case 2 : return "numerical";
        case 3 : return "boolean";
        case 4 : return "regex";
        default : return "unknown ???";
    }
}


