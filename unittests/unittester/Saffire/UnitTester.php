<?php

namespace Saffire;

class UnitTester {
    protected $_it;                     // Directory iterator
    protected $_outputs;                // Different outputs;
    protected $_results = array();      // The actual results
    protected $_current = array();      // Information about the current test(s)

    protected $_timeStart;              // Time started
    protected $_timeEnd;                // Time ended

    protected $_mandatoryHeaders = array("title", "author");


    protected $_saffireBinary = null;       // Actual binary to test
    protected $_saffireBinaryVersion;       // Actual binary version (for version checking)

    // Result chars
    const PASS = ".";
    const FAIL = "X";
    const IGNORE = "I";
    const SKIP = "S";

    /**
     *
     */
    function __construct(\iterator $it) {
        $this->_it = $it;
        $this->_outputs = new \SplObjectStorage();


        // Get the binary to test from the environment
        $this->_saffireBinary = getenv("SAFFIRE_TEST_BIN");
        if (! $this->_saffireBinary) {
            print "Please set the SAFFIRE_TEST_BIN environment setting to the correct saffire binary version.\n\n";
            exit(1);
        }

        // Execute (and thus: find) the binary to find version
        exec($this->_saffireBinary." --version", $output, $result);
        if ($result != 0) {
            print "Cannot find the test binary set by the SAFFIRE_TEST_BIN environment, or error while fetching its version.\n\n";
            exit(1);
        }

        // Parse version and store it so we can use this info for some of the header tags (since, until etc)
        if (!preg_match("/v([0-9\.]+)/", $output[0], $match)) {
            print "Cannot find the test binary set by the SAFFIRE_TEST_BIN environment, or error while fetching its version.\n\n";
            exit(1);
        }
        $this->_saffireBinaryVersion = $match[1];
    }


    /**
     * Adds an output class
     *
     * @param Output\iOutput $output
     */
    function addOutput(Output\iOutput $output) {
        $this->_outputs->attach($output);
    }


    /**
     * Removes an output class if was added before
     *
     * @param Output\iOutput $output
     */
    function removeOutput(Output\iOutput $output) {
        if ($this->_outputs->contains($output)) {
            $this->_outputs->detach($output);
        }
    }


    /**
     * Run the tester
     */
    function run() {
        $this->_init();

        // Header
        $this->_output("Saffire Test Suite v0.1 - The Saffire Group\n");

        // Timer
        $this->_startTimer();
        $this->_output("Start time: ".date("H:i:s", time())."\n");

        // Iterate and test all files
        foreach ($this->_it as $filename) {
            if (is_array($filename)) $filename = $filename[0];
            $this->_runTest($filename);
        }

        // Output end time
        $this->_output("End time: ".date("H:i:s", time()));
        $this->_output(" (".$this->_timerLap()." seconds running time)\n");

        // Output status
        $this->_output("Status\n");
        $this->_output("\tTest files  : ".sprintf("%5d", $this->_results['total_files'])."\n");
        $this->_output("\tTotal tests : ".sprintf("%5d", $this->_results['total_tests'])."\n");
        $this->_output("\tPassed      : ".sprintf("%5d", $this->_results['passed'])." (".$this->_perc($this->_results['passed'], $this->_results['total_tests']).")\n");
        $this->_output("\tFailed      : ".sprintf("%5d", $this->_results['failed'])." (".$this->_perc($this->_results['failed'], $this->_results['total_tests']).")\n");
        $this->_output("\tIgnored     : ".sprintf("%5d", $this->_results['ignored'])." (".$this->_perc($this->_results['ignored'], $this->_results['total_tests']).")\n");
        $this->_output("\tSkipped     : ".sprintf("%5d", $this->_results['skipped'])." (".$this->_perc($this->_results['skipped'], $this->_results['total_tests']).")\n");
        $this->_output("\n");


        foreach ($this->_results['errors'] as $error) {
            $this->_output("=============================\n");
            $this->_output($error);
            $this->_output("\n");
        }
    }

    protected function _perc($p, $t) {
        if ($t == 0) return "0.00%";
        return round($p / $t * 100, 2)."%";
    }


    /**
     * Initialize information
     */
    protected function _init() {
        $this->_results = array();
        $this->_results['total_files'] = 0;
        $this->_results['total_tests'] = 0;
        $this->_results['passed'] = 0;
        $this->_results['failed'] = 0;
        $this->_results['skipped'] = 0;
        $this->_results['ignored'] = 0;

        $this->_results['errors'] = array();
        $this->_current = array();
    }


    /**
     * Start the timer
     */
    protected function _startTimer() {
        $this->_timeStart = microtime(true);
    }


    /**
     * Stop the timer
     */
    protected function _timerLap() {
        $tmp = microtime(true);
        $tmp = round($tmp - $this->_timeStart, 2);
        return $tmp;
    }


    /**
     * Outputs a string to all connected output classes
     *
     * @param $str
     */
    protected function _output($str) {
        // Output to all the output functions
        foreach ($this->_outputs as $output) {
            /** @var $output Output\iOutput */
            $output->output($str);
        }
    }


    /**
     * Run one specific test
     */
    protected function _runTest($filename) {
        // Increase the number of test FILES we are testing
        $this->_results['total_files']++;

        // Initialize the current suite result
        $this->_current = array();
        $this->_current['filename'] = $filename;
        $this->_current['lineno'] = 0;
        $this->_current['tags'] = array();

        $this->_output($this->_current['filename']." : ");

        // Read header
        $f = file_get_contents($filename);
        if ($f == null) {
            $this->_results['failed']++;
        }
        $this->_current['contents'] = $f;

        // Find header and body
        $pattern = "/^\*\*+/m";
        $header_and_body = preg_split($pattern, $this->_current['contents'], 4);
        if (count($header_and_body) != 2) {
            $this->_output("Error finding header in file\n");
            return;
        }

        if (! $this->_parseHeader($header_and_body[0])) {
            $this->_output("Error parsing header in file\n");
            return;
        }

        $this->_output($this->_current['tags']['title']." : [");

        // Do each test, separated by @@@
        $this->_runFunctionalTests($header_and_body[1]);

        $this->_output("]\n");
    }


    protected function _runFunctionalTests($body) {
        // Split each test
        $pattern = "/^\@\@+/m";
        $tests = preg_split($pattern, $body);

        foreach ($tests as $test) {
            // Run the actual test
            $result = $this->_runFunctionalTest($test);
            $this->_results['total_tests']++;

            switch ($result) {
                case self::PASS :
                    $this->_results['passed']++;
                    break;
                case self::FAIL :
                    $this->_results['failed']++;
                    break;
                case self::IGNORE :
                    $this->_results['ignored']++;
                    break;
                case self::SKIP :
                    $this->_results['skipped']++;
                    break;
            }
            $this->_output($result);
        }
    }


    /**
     */
    protected function _runFunctionalTest($test, $lineno = "unknown") {
        $tmpDir = sys_get_temp_dir();

        // Don't expect any output
        $outputExpected = false;

        // Split source and expected output
        $pattern = "/^==+\n/m";
        $tmp = preg_split($pattern, $test, 2);

        // Generate temp file(name)
        $tmpFile = tempnam($tmpDir, "saffire_test");

        // Create tempfile with source and optional expectation
        file_put_contents($tmpFile.".sf", $tmp[0]);
        if (isset($tmp[1])) {
            $outputExpected = true;
            $tmp[1] = preg_replace("/\n$/", "", $tmp[1]);
            file_put_contents($tmpFile.".exp", $tmp[1]);
        }

        // Run saffire test
        $ret = self::FAIL;  // Assume the worst
        exec($this->_saffireBinary." ".$tmpFile.".sf > ".$tmpFile.".out 2>&1", $output, $result);

        // No output expected and result is not 0. So FAIL
        if (! $outputExpected && $result != 0) {
            $ret = self::FAIL;
            goto cleanup;   // Yes, goto. Live with the pain
        }

        // No output expected, but result is 0. So PASS
        if (! $outputExpected && $result == 0) {
            $ret = self::PASS;
            goto cleanup;   // Yes, goto. Live with the pain
        }


        if ($outputExpected) {
            // We expect certain output. Let's try and diff it..
            exec("/usr/bin/diff --suppress-common-lines ".$tmpFile.".out ".$tmpFile.".exp", $output, $result);
            if ($result != 0) {
                $tmp = "";
                $tmp .= "Error in ".$this->_current['filename']." at ".$lineno."\n";
                $tmp .= join("\n", $output);
                $tmp .= "\n";

                // @TODO: when we are at X errors, quit anyway
                $this->_results['errors'][] = $tmp;

                $ret = self::FAIL;
                goto cleanup;
            } else {
                $ret = self::PASS;
                goto cleanup;
            }
        }

cleanup:
        // Unlink all temp files
        @unlink($tmpFile);
        @unlink($tmpFile.".sf");
        @unlink($tmpFile.".exp");
        @unlink($tmpFile.".out");
        @unlink($tmpFile.".diff");
        return $ret;
    }


    /**
     * Parses and validates the tags inside the headers
     */
    protected function _parseHeader($header) {
        foreach (explode("\n", $header) as $line) {
            $this->_current['lineno']++;
            $line = trim($line);
            if (empty($line)) continue;     // Skip empty lines
            if ($line[0] == "#") continue;  // Skip comments

            $tmp = explode(":", $line, 2);
            if (count($tmp) != 2) {
                $this->_output("Cannot find tag on line ".$this->_current['lineno']."\n");
                return false;
            }

            list($key, $value) = $tmp;
            $this->_current['tags'][strtolower($key)] = trim($value);
        }

        // Find all the mandatory headers
        $tmp = array_diff($this->_mandatoryHeaders, array_keys($this->_current['tags']));
        if (count($tmp) > 0) {
            $this->_output("Error finding mandatory headers: ".join(", ", $tmp)."\n");
            return false;
        }
        return true;
    }

}
