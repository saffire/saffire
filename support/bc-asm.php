<?php

$asm = new Assembler($argv[1]);
if ($asm->assemble()) $asm->c_print();
exit;

/**
 *
 */
class Frame {
    /** @var \Assembler */
    protected $asm;                 // Assembler object

    protected $stack_size = 25;     // Stack size for this frame
    public $name;                   // name of the frame
    protected $constants;           // Constant array
    protected $identifiers;         // Identifier array
    protected $linenos;             // ?
    protected $labels;              // Labelname => Offsets
    protected $bc;                  // Bytecode

    protected $compares = array ("OP_EQ" => 1,
                                 "OP_NE" => 2
                                );
    protected $opcodes = array("STOP" => 0x00,
             "POP_TOP" => 0x01,
             "ROT_TWO" => 0x02,
             "ROT_THREE" => 0x03,
             "DUP_TOP" => 0x04,
             "ROT_FOUR" => 0x05,

             "NOP" => 0x09,

             "BINARY_ADD" => 0x17,
             "BINARY_SUB" => 0x18,
             "BINARY_MUL" => 0x19,
             "BINARY_DIV" => 0x1A,
             "BINARY_SHL" => 0x1B,
             "BINARY_SHR" => 0x1C,
             "BINARY_AND" => 0x1D,
             "BINARY_OR" => 0x1E,
             "BINARY_XOR" => 0x1F,

             "INPLACE_ADD" => 0x20,
             "INPLACE_SUB" => 0x21,
             "INPLACE_MUL" => 0x22,
             "INPLACE_DIV" => 0x23,
             "INPLACE_SHL" => 0x24,
             "INPLACE_SHR" => 0x25,
             "INPLACE_AND" => 0x26,
             "INPLACE_OR" => 0x27,
             "INPLACE_XOR" => 0x28,

             "MAKE_METHOD" => 0x71,

             "POP_BLOCK" => 0x72,
             "RETURN" => 0x73,
             "BREAK_LOOP" => 0x74,
             "BREAKELSE_LOOP" => 0x75,


             "USE" => 0x7E,
             "IMPORT" => 0x7F,

             "STORE_ID" => 0x80,
             "LOAD_CONST" => 0x81,
             "LOAD_ID" => 0x82,

             "JUMP_FORWARD" => 0x83,
             "JUMP_IF_TRUE" => 0x84,
             "JUMP_IF_FALSE" => 0x85,
             "JUMP_ABSOLUTE" => 0x86,

             "DUP_TOPX" => 0x87,

             "LOAD_GLOBAL" => 0x88,
             "STORE_GLOBAL" => 0x89,
             "DELETE_GLOBAL" => 0x8A,

             "SETUP_LOOP" => 0x90,
             "CONTINUE_LOOP" => 0x92,

        "BUILD_CLASS" => 0x93,

             "COMPARE_OP" => 0x95,
             "SETUP_FINALLY" => 0x96,
             "SETUP_EXCEPT" => 0x97,
             "END_FINALLY" => 0x98,

             "CALL_METHOD" => 0xC0,
             "RESERVED" => 0xFF,
        );

    /**
     * @param $frame_name
     */
    public function __construct(Assembler $asm, $frame_name) {
        $this->asm = $asm;
        $this->name = $frame_name;

        $this->constants = array();
        $this->identifiers = array();
        $this->labels = array();

        $this->bc = array();
    }

    /**
     *
     */
    public function c_print() {
        echo "/**\n";
        echo " *\n";
        echo " */\n";
        echo "t_bytecode *generate_dummy_bytecode_".$this->asm->basename."_".$this->name."(void) {\n";
        echo "    char byte_code[] = \n";

        for ($i=0; $i!=count($this->bc); $i++) {
            $ln = $this->linenos[$i];

            // If this offset has a label, print it as well..
            if (in_array($i, $this->labels)) {
                $k = array_search($i, $this->labels);
                echo "    // #".$k.":\n";
            }

            // Construct opcode hex codes
            $s = "";
            $opr = $this->bc[$i];
            $s .= sprintf("        \"\x%02X", $opr);

            if (($opr & 0xC0) == 0xC0) {
                $s .= sprintf("\x%02X\x%02X", $this->bc[++$i], $this->bc[++$i]);
                $s .= sprintf("\x%02X\x%02X", $this->bc[++$i], $this->bc[++$i]);
            }
            if (($opr & 0xC0) == 0x80) {
                $s .= sprintf("\x%02X\x%02X", $this->bc[++$i], $this->bc[++$i]);
            }
            $s .= sprintf("\"");

            // Print codes plus the line in comments
            printf("%-30s // %s\n", $s, trim($this->asm->getLine($ln)));
        }
        echo "    ;\n";


        echo "    t_bytecode *bc = (t_bytecode *)smm_malloc(sizeof(t_bytecode));\n";
        echo "    bzero(bc, sizeof(t_bytecode));\n";
        echo "    bc->stack_size = ".$this->stack_size.";\n";
        echo "    bc->code_len = sizeof(byte_code);\n";
        echo "    bc->code = smm_malloc(bc->code_len);\n";
        echo "    memcpy(bc->code, byte_code, bc->code_len);\n";
        echo "\n";
        echo "    bc->constants = NULL;   // Important to start constants and identifiers on NULL\n";
        echo "    bc->identifiers = NULL;\n";
        echo "\n";
        echo "    // constants\n";

        for ($i=0; $i!=count($this->constants); $i++) {
            if ($this->constants[$i]['type'] == "numerical") {
                echo "    _new_constant_long(bc, ".$this->constants[$i]['constant'].");\n";
            }
            if ($this->constants[$i]['type'] == "string") {
                echo "    _new_constant_string(bc, \"".$this->constants[$i]['constant']."\");\n";
            }
            if ($this->constants[$i]['type'] == "code") {
                echo "    _new_constant_code(bc, ".$this->constants[$i]['constant']."());\n";
            }
        }

        echo "\n";
        echo "    // identifier names\n";
        for ($i=0; $i!=count($this->identifiers); $i++) {
            echo "    _new_name(bc, \"".$this->identifiers[$i]."\");\n";
        }
        echo "    \n";
        echo "    return bc;\n";
        echo "}\n";
        echo "\n";
        echo "\n";
        echo "\n";
    }


    /**
     *
     */
    public function assemble() {
        // All the bytecodeoffsets => label-names we need to fill later
        $label_pass = array();

        while (($line = $this->asm->nextline()) !== false) {

            $line = trim($line);
            if ($line == "") continue;         // Skip empty lines
            if ($line[0] == ";") continue;     // Skip comments

            // Matches a frame?
            if (preg_match_all("/^[ \t]*\@frame[ \t]+([a-z0-9_]+)/i", $line, $matches)) {
                $this->asm->setLineNo($this->asm->getLineNo() - 1);
                break;
            }

            // Matches a label?
            if (preg_match("/^[ \t]*\#([a-z0-9_]+)\:/i", $line, $matches)) {
                $label = $matches[1];
                if (isset ($this->labels[$label])) {
                    print("Label '$label' defined twice!");
                    exit(1);
                }
                $this->labels[$label] = count($this->bc);
                continue;
            }

            # Match opcodes
            if (preg_match_all('/^[ \t]*([a-z_]+)(.*)$/i', $line, $matches)) {

                // Parse opcode and operands from source line
                $opcode = $matches[1][0];
                $operands = trim($matches[2][0]);

                $operand = array();
                preg_match_all('/("[^"]*"|\$?\d+|[@#]?[a-z0-9_]+)(?:,\t*)*/i', $operands, $matches);
                foreach ($matches[1] as $match) $operand[] = $match;

                // Convert opcode to hex
                $opcode_hex = $this->_fetch_hex_opcode($opcode);
                if ($opcode_hex == -1) {
                    die("Unknown hex conversion found for $opcode at line ".$this->asm->getLineNo());
                }


                $this->linenos[count($this->bc)] = $this->asm->getLineNo();

                // Calculate number of operands
                $operand_count = 0;
                if (($opcode_hex & 0x80) == 0x80) $operand_count = 1;
                if (($opcode_hex & 0xC0) == 0xC0) $operand_count = 2;

                // Sanity check on operand counts
                if (count($operand) != $operand_count) {
                    printf("Opcode ".$opcode." on line ".$this->asm->getLineNo()." requires ".$operand_count." operands, but ".count($operand)." given!\n");
                    exit(1);
                }


                // Convert operands to constant / identifiers / bytecode offsets
                for ($i=0; $i!=$operand_count; $i++) {
                    if ($operand[$i][0] == '#') {
                        // Loop
                        // Note: We will fill offsets later on, when we actually have parsed ALL the codes
                        $label_pass[count($this->bc)] = substr($operand[$i], 1);
                        $operand[$i] = 0xFFFF;

                    } else if ($operand[$i][0] == '"') {
                        // String
                        $operand[$i] = $this->_convert_constant("string", substr($operand[$i], 1, -1));

                    } else if ($operand[$i][0] == '@') {
                        // Frame
                        $operand[$i] = $this->_convert_constant("code", "generate_dummy_bytecode_".$this->asm->basename."_".substr($operand[$i], 1));

                    } else if ($operand[$i][0] == '$') {
                        // Real numerical
                        $operand[$i] = substr($operand[$i], 1);

                    } else if (is_numeric($operand[$i])) {
                        // Numerical
                        $operand[$i] = $this->_convert_constant("numerical", $operand[$i]);

                    } elseif (isset($this->compares[$operand[$i]])) {
                        $operand[$i] = $this->compares[$operand[$i]];

                    } else {
                        // Identifier name
                        $operand[$i] = $this->_convert_identifier($operand[$i]);
                    }
                }

                // Save to our bytecode array
                $this->bc[] = $opcode_hex;
                for ($i=0; $i!=$operand_count; $i++) {
                    $this->bc[] = $this->lo($operand[$i]);
                    $this->bc[] = $this->hi($operand[$i]);
                }

                continue;
            }

            # Matches nothing...
        }

        // Fill the missing labels
        foreach ($label_pass as $off => $label) {
            if (! isset($this->labels[$label])) {
                printf("Cannot find label $label\n");
                exit(1);
            }

            $label_off = $this->labels[$label];

            if (in_array($this->bc[$off], array(0x86, 0x92, 0x93))) {  // These are absolute jumps
                $new_offset = $label_off;
            } else {
                $new_offset = $label_off - $off - 2 - 1;
            }
            $this->bc[$off+1] = $new_offset;

        }
    }


    /**
     * @param $type
     * @param $constant
     */
    protected function _convert_constant($type, $constant) {
        // Find constant
        foreach ($this->constants as $k => $v) {
            if ($v['type'] == $type && $v['constant'] == $constant) {
                return $k;
            }
        }

        // Add
        $tmp = array();
        $tmp['type'] = $type;
        $tmp['constant'] = $constant;
        $this->constants[] = $tmp;
        $k = count($this->constants)-1;

        return $k;
    }


    /**
     * @param $identifier
     * @return int
     */
    protected function _convert_identifier($identifier) {
        // Find constant
        foreach ($this->identifiers as $k => $v) {
            if ($v == $identifier) {
                return (int)$k;
            }
        }

        // Add
        $this->identifiers[] = $identifier;
        $k = count($this->identifiers)-1;

        return $k;
    }


    /**
     * @param $opcode
     * @return int
     */
    protected function _fetch_hex_opcode($opcode) {
        $opcode = strtoupper($opcode);

        if (! isset($this->opcodes[$opcode])) {
            printf("Incorrect opcode ($opcode) found at line ".$this->lineno);
            return -1;
        }

        return $this->opcodes[$opcode];
    }


    protected function lo($w) {
        return ($w & 0x00FF);
    }

    protected function hi($w) {
        return (($w & 0xFF00) >> 16);
    }

}

/**
 *
 */
class Assembler {
    protected $lineno;
    protected $f;
    protected $filename;
    protected $lines;
    public $basename;

    /**
     * @param $filename
     */
    public function __construct($filename) {
        $this->filename = $filename;
        $this->lines = file($filename);
        $this->lineno = 0;
        $this->basename = basename(str_replace(".", "_", $this->filename));;

        $this->frames = array();
    }


    public function getline($idx) {
        return $this->lines[$idx - 1];
    }
    /**
     *
     */
    public function nextline() {
        if ($this->lineno >= count($this->lines)) {
            return false;
        }

        $this->lineno++;

        $s = trim($this->lines[$this->lineno - 1]);
        return $this->lines[$this->lineno - 1];
    }

    /**
     * @return mixed
     */
    public function getLineNo() {
        return $this->lineno;
    }

    public function setLineNo($lineno) {
        $this->lineno = $lineno;
    }



    /**
     *
     */
    public function assemble() {
        // Assemble "main"-frame
        $frame = new Frame($this, "main");
        $frame->assemble();
        $this->frames[] = $frame;

        // When we're not done (ie: we're stopped at @frame), do next frame
        while (($line = $this->nextline()) !== false) {
            if (preg_match("/^[ \t]*\@frame[ \t]+([a-z0-9_]+)/i", $line, $matches)) {
                $frame = new Frame($this, $matches[1]);
                $frame->assemble();
                $this->frames[] = $frame;
            }
        }

        return true;
    }

    /**
     *
     */
    public function c_print() {
        echo "/**\n";
        echo " * Generated bytecode from ".$this->filename."\n";
        echo " */\n";
        echo "\n";
        echo "\n";

        // Setup forward defines
        foreach ($this->frames as $frame) {
            echo "t_bytecode *generate_dummy_bytecode_".$this->basename."_".$frame->name."(void);\n";
        }

        // Parse frames
        foreach ($this->frames as $frame) {
            $frame->c_print();
        }
        echo "\n";
    }

}