<?php

namespace bytecode;

class Ansi {
    const BLACK   = 0;
    const RED     = 1;
    const GREEN   = 2;
    const YELLOW  = 3;
    const BLUE    = 4;
    const MAGENTA = 5;
    const CYAN    = 6;
    const WHITE   = 7;

    protected $fg_colors = array(30, 31, 32, 33, 34, 35, 36, 37);
    protected $bg_colors = array(40, 41, 42, 43, 44, 45, 46, 47);


    function fg($idx) {
        if (! isset($this->fg_colors[$idx])) return;
        print "\033[".$this->fg_colors[$idx]."m";
    }

    function bg($idx) {
        if (! isset($this->bg_colors[$idx])) return;
        print "\033[".$this->bg_colors[$idx]."m";
    }

    function bold() {
        print "\033[1m";
    }

    function reset() {
        print "\033[0m";
    }

}
