<?php

$asm = new Assembler($argv[1]);
$bc = $asm->assemble();
if (! $bc) exit;
$asm->c_print($bc);
exit;

class Assembler {
    protected $lineno;
    protected $f;
    protected $filename;

    protected $constants;
    protected $variables;
    protected $labels;

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

             "BUILD_CLASS" => 0x70,
             "MAKE_METHOD" => 0x71,

             "POP_BLOCK" => 0x72,
             "RETURN_VALUE" => 0x73,
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
             "COMPARE_OP" => 0x95,
             "SETUP_FINALLY" => 0x96,
             "SETUP_EXCEPT" => 0x97,
             "END_FINALLY" => 0x98,

             "CALL_METHOD" => 0xC0,
             "RESERVED" => 0xFF,
        );


    /**
     * @param $filename
     */
    public function __construct($filename) {
        $this->filename = $filename;
        $this->lines = file($filename);
    }


    /**
     *
     */
    protected function _init_assemble() {
         $this->bc_offset = 0;
         $this->lineno = 0;
         $this->constants = array();
         $this->identifiers = array();
         $this->labels = array();
     }


    /**
     *
     */
    public function assemble() {
        $this->_init_assemble();
        $bc = array();

        $this->linenos = array();

        // All the bytecodeoffsets => label-names we need to fill later
        $label_pass = array();

        foreach ($this->lines as $ln => $line) {
            $this->lineno++;

            $line = trim($line);
            if ($line == "") continue;         // Skip empty lines
            if ($line[0] == ";") continue;     // Skip comments

            // Match label?
            if (preg_match_all("/^[ \t]*\#([a-z0-9_]+)\:/i", $line, $matches)) {
                $label = $matches[1][0];
                if (isset ($this->labels[$label])) {
                    print("Label '$label' defined twice!");
                    return null;
                }
                $this->labels[$label] = count($bc);
                continue;
            }

            # Match opcodes
            if (preg_match_all('/^[ \t]*([a-z_]+)(.*)$/i', $line, $matches)) {

                // Parse opcode and operands from source line
                $opcode = $matches[1][0];
                $operands = trim($matches[2][0]);

                $operand = array();
                preg_match_all('/("[^"]*"|\$?\d+|#?[a-z0-9_]+)(?:,\t*)*/i', $operands, $matches);
                foreach ($matches[1] as $match) $operand[] = $match;

                // Convert opcode to hex
                $opcode_hex = $this->_fetch_hex_opcode($opcode);
                if ($opcode_hex == -1) return null;


                $this->linenos[count($bc)] = $ln;

                // Calculate number of operands
                $operand_count = 0;
                if (($opcode_hex & 0x80) == 0x80) $operand_count = 1;
                if (($opcode_hex & 0xC0) == 0xC0) $operand_count = 2;

                // Sanity check on operand counts
                if (count($operand) != $operand_count) {
                    printf("Opcode ".$opcode." on line ".$this->lineno." requires ".$operand_count." operands, but ".count($operand)." given!\n");
                    return null;
                }


                // Convert operands to constant / identifiers / bytecode offsets
                for ($i=0; $i!=$operand_count; $i++) {
                    if ($operand[$i][0] == '#') {
                        // Loop
                        // Note: We will fill offsets later on, when we actually have parsed ALL the codes
                        $label_pass[count($bc)] = substr($operand[$i], 1);
                        $operand[$i] = 0xFFFF;
                    } else if ($operand[$i][0] == '"') {
                        // String
                        $operand[$i] = $this->_convert_constant("string", substr($operand[$i], 1, -1));
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
                $bc[] = $opcode_hex;
                for ($i=0; $i!=$operand_count; $i++) {
                    $bc[] = $this->lo($operand[$i]);
                    $bc[] = $this->hi($operand[$i]);
                }

                continue;
            }

            # Matches nothing...
        }

        // Fill the labels
        foreach ($label_pass as $off => $label) {
            if (! isset($this->labels[$label])) {
                printf("Cannot find label $label\n");
                return null;
            }

            $label_off = $this->labels[$label];

            if (in_array($bc[$off], array(0x86, 0x92, 0x93))) {  // JUMP_ABSOLUTE
                $new_offset = $label_off;
            } else {
                $new_offset = $label_off - $off - 2 - 1;
            }
            $bc[$off+1] = $new_offset;

        }

        $tmp = new StdClass();
        $tmp->code = $bc;
        $tmp->constants = $this->constants;
        $tmp->identifiers = $this->identifiers;
        return $tmp;
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


    public function c_print($bc) {
        $name = basename(str_replace(".", "_", $this->filename));
        echo "/**\n";
        echo " * Generated bytecode from ".$this->filename."\n";
        echo " */\n";
        echo "t_bytecode *generate_dummy_bytecode_".$name."(void) {\n";
        echo "    char byte_code[] = \n";


        for ($i=0; $i!=count($bc->code); $i++) {
            $ln = $this->linenos[$i];

            // If this offset has a label, print it as well..
            if (in_array($i, $this->labels)) {
                $k = array_search($i, $this->labels);
                echo "    // #".$k.":\n";
            }

            // Construct opcode hex codes
            $s = "";
            $opr = $bc->code[$i];
            $s .= sprintf("        \"\x%02X", $opr);

            if (($opr & 0xC0) == 0xC0) {
                $s .= sprintf("\x%02X\x%02X", $bc->code[++$i], $bc->code[++$i]);
                $s .= sprintf("\x%02X\x%02X", $bc->code[++$i], $bc->code[++$i]);
            }
            if (($opr & 0xC0) == 0x80) {
                $s .= sprintf("\x%02X\x%02X", $bc->code[++$i], $bc->code[++$i]);
            }
            $s .= sprintf("\"");

            // Print codes plus the line in comments
            printf("%-30s // %s\n", $s, trim($this->lines[$ln]));
        }
        echo "    ;\n";


        echo "    t_bytecode *bc = (t_bytecode *)smm_malloc(sizeof(t_bytecode));\n";
        echo "    bzero(bc, sizeof(t_bytecode));\n";
        echo "    bc->stack_size = 10;\n";
        echo "    bc->code_len = sizeof(byte_code);\n";
        echo "    bc->code = smm_malloc(bc->code_len);\n";
        echo "    memcpy(bc->code, byte_code, bc->code_len);\n";
        echo "    \n";
        echo "    bc->constants = NULL;   // Important to start constants and identifiers on NULL\n";
        echo "    bc->identifiers = NULL;\n";
        echo "    \n";
        echo "    // constants\n";

        for ($i=0; $i!=count($bc->constants); $i++) {
            if ($bc->constants[$i]['type'] == "numerical") {
                echo "    _new_constant_long(bc, ".$bc->constants[$i]['constant'].");\n";
            }
            if ($bc->constants[$i]['type'] == "string") {
                echo "    _new_constant_string(bc, \"".$bc->constants[$i]['constant']."\");\n";
            }
        }

        echo "    // identifier names\n";
        for ($i=0; $i!=count($bc->identifiers); $i++) {
            echo "    _new_name(bc, \"".$bc->identifiers[$i]."\");\n";
        }
        echo "    \n";
        echo "    return bc;\n";
        echo "}\n";
    }

    /**
     * @param array $bc
     */
    public function debug_print($bc) {
        printf(".code\n");
        for ($i=0; $i!=count($bc->code); $i++) {
            $opr = $bc->code[$i];
            printf("  %04X %02X", $i, $opr);

            if (($opr & 0xC0) == 0xC0) {
                printf(" %02X", $bc->code[++$i]);
                printf(" %02X", $bc->code[++$i]);
            }
            if (($opr & 0xC0) == 0x80) {
                printf(" %02X", $bc->code[++$i]);
            }
            printf("\n");
        }
        printf("\n");

        printf(".constants\n");
        for ($i=0; $i!=count($bc->constants); $i++) {
            printf("  %-10s %s\n", $bc->constants[$i]['type'], $bc->constants[$i]['constant']);
        }
        printf("\n");

        printf(".variables\n");
        for ($i=0; $i!=count($bc->identifiers); $i++) {
            printf("  %s\n", $bc->identifiers[$i]);
        }
        printf("\n");
    }

}