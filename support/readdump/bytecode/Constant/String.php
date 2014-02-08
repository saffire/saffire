<?php

namespace bytecode\Constant;

use bytecode\Constant\AbstractConstant;

class String extends AbstractConstant {

    function __construct($data) {
        $data = unpack("Vlen/A*", $data);
        print_r($data);
        $this->data = $data['len'];
    }
}
