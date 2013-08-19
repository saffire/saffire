<?php

namespace bytecode\Constant;

use bytecode\Constant\AbstractConstant;

class Numerical extends AbstractConstant {

    function __construct($data) {
        $data = unpack("Vval", $data);
        $this->data = $data['val'];
    }

}
