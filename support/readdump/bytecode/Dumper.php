<?php

namespace bytecode;

use bytecode\Header;
use bytecode\Factory;
use bytecode\Constant\Code;

class Dumper {
    const PADDING = "      ";

    protected $filename;
    protected $ansi;

    /**
     * @param $filename
     */
    function __construct($filename) {
        $this->ansi = new Ansi();

        $this->filename = $filename;
        $this->data = file_get_contents($filename);

        $this->header = new Header(substr($this->data, 0, 32));
    }

    /**
     * @return bool
     */
    public function isValid() {
        if (! $this->data ||
            $this->header->getMagic() != Header::MAGIC) {
            return false;
        }
        return true;
    }

    /**
     *
     */
    public function dump() {
        $this->dumpHeader($this->header);

        // Fetch bytecode and decompress before
        $data = substr($this->data, $this->header->getBytecodeOffset(), $this->header->getBytecodeLength());
        $data = bzdecompress($data);

        // First object in the bytecode data is always a code object
        $codeObject = Factory::createConstant(Factory::CONSTANT_TYPE_CODE, $data);
        $this->dumpCode($codeObject);
    }

    /**
     * @param Header $header
     */
    protected function dumpHeader(Header $header) {
        $this->ansi->fg(Ansi::YELLOW);
        $this->ansi->bold();

        printf("-- Header -----------------------------------------------------------------\n");
        printf("  Magic                : 0x%08X\n", $header->getMagic());
        printf("  Sourcefile timestamp : ". date("d-m-Y h:i:s", $header->getTimestamp())."\n");
        printf("  Flags                : 0x%08X\n", $header->getFlags());
        echo "\n";
        printf("  Bytecode len         : %5d  (0x%08X)\n", $header->getBytecodeLength(), $header->getBytecodeLength());
        printf("  Bytecode uncomp len  : %5d  (0x%08X)\n", $header->getBytecodeLengthUncompressed(), $header->getBytecodeLengthUncompressed());
        printf("  Bytecode off         : %5d  (0x%08X)\n", $header->getBytecodeOffset(), $header->getBytecodeOffset());
        printf("  Signature len        : %5d  (0x%08X)\n", $header->getSignatureLength(), $header->getSignatureLength());
        printf("  Signature off        : %5d  (0x%08X)\n", $header->getSignatureOffset(), $header->getSignatureOffset());
        printf("---------------------------------------------------------------------------\n");
        echo "\n";

        $this->ansi->reset();
    }

    protected function dumpCode(Code $code, $level = 0) {
        // Just make sure we start with a white color at level 0
        $col_idx = (($level + 6) % 7) + 1;
        $this->ansi->fg($col_idx);
        $this->ansi->bold();

        printf(str_repeat(self::PADDING, $level) . "-- Bytecode info --------------------------------------------------------------\n");
        printf(str_repeat(self::PADDING, $level) . "  Stack size          : %5d (0x%08X)\n", $code->getStackSize(), $code->getStackSize());
        printf(str_repeat(self::PADDING, $level) . "  Code size           : %5d (0x%08X)\n", $code->getCodeSize(), $code->getCodeSize());
        printf(str_repeat(self::PADDING, $level) . "  Constant count      : %5d\n", count($code->getConstants()));
        printf(str_repeat(self::PADDING, $level) . "  Identifier count    : %5d\n", count($code->getIdentifier()));
        printf(str_repeat(self::PADDING, $level) . "  Lineno start        : %5d (0x%08X)\n", $code->getLinenoOffset(), $code->getLinenoOffset());
        printf(str_repeat(self::PADDING, $level) . "  Lineno size         : %5d (0x%08X)\n", $code->getLinenoLength(), $code->getLinenoLength());
        printf(str_repeat(self::PADDING, $level) . "-------------------------------------------------------------------------------\n");
        echo "\n";

        printf(str_repeat(self::PADDING, $level) . "-- Uncompressed bytecode ------------------------------------------------------\n");
        $this->hexdump($code->getOpcodes(), str_repeat(self::PADDING, $level)."  ");
        printf(str_repeat(self::PADDING, $level) . "-------------------------------------------------------------------------------\n");
        echo "\n";

        printf(str_repeat(self::PADDING, $level) . "-- Constants ------------------------------------------------------------------\n");
        foreach ($code->getConstants() as $k => $c) {
            switch ($c->getType()) {
                case Factory::CONSTANT_TYPE_STRING : // String
                    printf(str_repeat(self::PADDING, $level) . "  %d: String: %s\n", $k+1, $c->getData());
                    break;
                case Factory::CONSTANT_TYPE_NUMERICAL: // Numerical
                    printf(str_repeat(self::PADDING, $level) . "  %d: Numerical: %d (0x%08X)\n", $k+1, $c->getData(), $c->getData());
                    break;
                case Factory::CONSTANT_TYPE_CODE : // Code
                    printf(str_repeat(self::PADDING, $level) . "  %d: Code: \n", $k+1);
                    $this->dumpCode($c, $level + 1);

                    // Colr has been changed. Reset it back again
                    $this->ansi->fg($col_idx);
                    $this->ansi->bold();
                    break;
            }
        }
        printf(str_repeat(self::PADDING, $level) . "-------------------------------------------------------------------------------\n");
        echo "\n";

        printf(str_repeat(self::PADDING, $level) . "-- Identifiers ----------------------------------------------------------------\n");
        foreach ($code->getIdentifier() as $k => $i) {
            printf(str_repeat(self::PADDING, $level) . "  %d: %s\n", $k+1, $i->getData());
        }
        printf(str_repeat(self::PADDING, $level) . "-------------------------------------------------------------------------------\n");
        echo "\n";

        printf(str_repeat(self::PADDING, $level) . "-- Linenos --------------------------------------------------------------------\n");
        $this->hexdump($code->getLineno(), str_repeat(self::PADDING, $level)."  ");
        printf(str_repeat(self::PADDING, $level) . "-------------------------------------------------------------------------------\n");
        echo "\n";

        $this->ansi->reset();
    }

    protected function hexdump($data, $prefix = "") {
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
}
