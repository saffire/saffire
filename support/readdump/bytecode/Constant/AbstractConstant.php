<?php

namespace bytecode\Constant;

abstract class AbstractConstant {
    protected $data;
    protected $type;

    function __construct($data) {
        $this->data = $data;
    }

    function getData() {
        return $this->data;
    }

    function setType($type) {
        $this->type = $type;
    }

    function getType() {
        return $this->type;
    }
}
