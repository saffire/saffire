<?php

namespace bytecode;

use bytecode\Id;

class Factory {
    // Make sure these constants match the type in the constants of the bytecode!
    const CONSTANT_TYPE_STRING = 0;
    const CONSTANT_TYPE_NUMERICAL = 1;
    const CONSTANT_TYPE_CODE = 2;

    static function createConstant($type, $data) {
        switch ($type) {
            case self::CONSTANT_TYPE_CODE :
                $ret = new Constant\Code($data);
                break;
            case self::CONSTANT_TYPE_NUMERICAL :
                $ret = new Constant\Numerical($data);
                break;
            case self::CONSTANT_TYPE_STRING :
                $ret = new Constant\String($data);
                break;
            default :
                throw new \LogicException("Unknown constant type encountered: ".$type);
        }

        $ret->setType($type);
        return $ret;
    }

    static function createId($data) {
        return new Id\String($data);
    }

}
