<?php

namespace bytecode\Constant;

use bytecode\Factory;

class Code extends AbstractConstant {
    protected $stack_size = 0;
    protected $code_size = 0;
    protected $constants = array();
    protected $identifier = array();
    protected $lineno_offset = 0;
    protected $lineno_length = 0;
    protected $lineno = array();
    protected $opcodes = array();

    function __construct($data) {
        $ptr = 0;

        $tmp = unpack ("Vstacksize", substr($data, $ptr));
        $ptr += 4;
        $this->stack_size = $tmp['stacksize'];

        $tmp = unpack ("Vcodesize", substr($data, $ptr));
        $ptr += 4;
        $this->code_size = $tmp['codesize'];
        $this->opcodes = substr($data, $ptr, $this->code_size);
        $ptr += $this->code_size;

        $tmp = unpack ("Vconstcount", substr($data, $ptr));
        $ptr += 4;
        $constCount = $tmp['constcount'];
        for ($i=0; $i!=$constCount; $i++) {
            $tmp = unpack("Ctype/Vlen", substr($data, $ptr));
            $ptr += 5;
            $this->constants[] = Factory::createConstant($tmp['type'], substr($data, $ptr, $tmp['len']));
            $ptr += $tmp['len'];
        }

        $tmp = unpack("Vidcount", substr($data, $ptr));
        $ptr += 4;
        $idCount = $tmp['idcount'];
        for ($i=0; $i!=$idCount; $i++) {
            $tmp = unpack("Vlen", substr($data, $ptr));
            $ptr += 4;
            $ids[] = Factory::createId(substr($data, $ptr, $tmp['len']));
            $ptr += $tmp['len'];
        }

        $tmp = unpack ("Vlinostart/Vlinosize", substr($data, $ptr));
        $ptr += 8;
        $this->lineno_offset = $tmp['linostart'];
        $this->lineno_length = $tmp['linosize'];

        $this->lineno = substr($data, $ptr, $tmp['linosize']);
    }

    public function getCodeSize()
    {
        return $this->code_size;
    }

    public function getConstants()
    {
        return $this->constants;
    }

    public function getIdentifier()
    {
        return $this->identifier;
    }

    public function getLineno()
    {
        return $this->lineno;
    }

    public function getLinenoLength()
    {
        return $this->lineno_length;
    }

    public function getLinenoOffset()
    {
        return $this->lineno_offset;
    }

    public function getOpcodes()
    {
        return $this->opcodes;
    }

    public function getStackSize()
    {
        return $this->stack_size;
    }

}
