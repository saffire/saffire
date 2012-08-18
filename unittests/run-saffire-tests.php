#!/usr/bin/php
<?php

$it = new RecursiveDirectoryIterator(".");
$it = new RecursiveIteratorIterator($it);
$it = new RegexIterator($it, '/^.+\.sfu$/i', RecursiveRegexIterator::GET_MATCH);

$suite = new UnitTestSuite($it);
$suite->run();
$suite->result();

class UnitTestSuite {
    protected $_it;
    protected $_results = array();
    protected $_current = array();

    protected $_timeStart;
    protected $_timeEnd;

    /**
     *
     */
    function __construct(iterator $it) {
        $this->_it = $it;
    }

    /**
     *
     */
    protected function _init() {
        $this->_results = array();
        $this->_results['pass'] = 0;
        $this->_results['fail'] = 0;
        $this->_current = array();
    }

    /**
     *
     */
    protected function _startTimer() {
        $this->_timeStart = microtime(true);
    }

    /**
     *
     */
    protected function _endTimer() {
        $this->_timeEnd = microtime(true);
    }

    /**
     *
     */
    function run() {
        $this->_init();
        $this->_startTimer();

        foreach ($this->_it as $filename) {
            $this->_runTest($filename[0]);
        }
        $this->_endTimer();
    }

    /**
     *
     */
    protected function _runTest($filename) {
        print "Reading test file: $filename\n";

        $this->_current['filename'] = $filename;
        $this->_current['lineno'] = 0;

        // Read header
        $f = file_get_contents($filename);
        if ($f == null) {
            $this->_results['fail']++;
        }
        $this->_current['contents'] = $f;

        // Find header
        $pattern = "/\*+(.*?)\*+/";
        preg_match_all($pattern, $this->_current['contents'], $matches);
        print_r ($matches);

        // Do each test, separated by @@@
        $this->_runFunctionalTest();
    }

    protected function _runFunctionalTest() {
    }
}
