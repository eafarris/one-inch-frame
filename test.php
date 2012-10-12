<?php
/*
 * TEST
 *
 * just a script to test functions, etc.
 */

// INTERNAL HELPER FUNCTIONS
require_once('functions.inc');

// CONFIGURATION
require_once('config.php');

/*
 * GRAB METADATA
 */
foreach (glob($datafiles. '/*.md')  as $infile) {
  if (is_file($infile)) {
    $sources[] = get_metadata($infile);
  } // endif filename matches a file
} // endforeach looping through files

$posts = blog_order($sources);

print_r($posts[0]);

