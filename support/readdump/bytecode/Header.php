<?php

namespace bytecode;

class Header {
    const MAGIC = 0x43424653;

    protected $magic;
    protected $timestamp;
    protected $flags;
    protected $bytecode_length;
    protected $bytecode_length_uncompressed;
    protected $bytecode_offset;
    protected $signature_length;
    protected $signature_offset;


    function __construct($data) {
        $header = unpack ("Vmagic/".
                          "Vtimestamp/".
                          "Vflags/".
                          "Vbytecode_len/".
                          "Vbytecode_uncompressed_len/".
                          "Vbytecode_offset/".
                          "Vsignature_len/".
                          "Vsignature_offset", $data);
        $this->magic = $header['magic'];
        $this->timestamp = $header['timestamp'];
        $this->flags = $header['flags'];
        $this->bytecode_length = $header['bytecode_len'];
        $this->bytecode_length_uncompressed = $header['bytecode_uncompressed_len'];
        $this->bytecode_offset = $header['bytecode_offset'];
        $this->signature_length = $header['signature_len'];
        $this->signature_offset = $header['signature_offset'];
    }

    public function getBytecodeLength()
    {
        return $this->bytecode_length;
    }

    public function getBytecodeLengthUncompressed()
    {
        return $this->bytecode_length_uncompressed;
    }

    public function getBytecodeOffset()
    {
        return $this->bytecode_offset;
    }

    public function getFlags()
    {
        return $this->flags;
    }

    public function getMagic()
    {
        return $this->magic;
    }

    public function getSignatureLength()
    {
        return $this->signature_length;
    }

    public function getSignatureOffset()
    {
        return $this->signature_offset;
    }

    public function getTimestamp()
    {
        return $this->timestamp;
    }
}
