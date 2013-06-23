<?php

namespace Saffire\Output;

/**
 * Simple output class that outputs to console
 */
class console implements iOutput {

    /**
     * Output to stdout
     * @param $str
     */
    function output($str) {
        print $str;
    }
}
