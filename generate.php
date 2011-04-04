<?php
/*
 * GENERATE
 *
 * Generate the HTML for the site
 */

// EXTERNAL LIBRARIES
require_once('includes/markdownphp/markdown.php');
require_once('includes/lessphp/lessc.inc.php');

// CONFIGURATION
require_once('config.php');

// INTERNAL FILTERS
require_once('freelinks.inc');

process_less();
$headtempl[] = '/<head>/';
$headrepl[]  = "<head>\n" . implode('', $head);

foreach (glob($datafiles. '/*.mkd')  as $infile) {
  if (is_file($infile)) {
    $ifn = pathinfo($infile, PATHINFO_FILENAME);
    print "Processing file: $infile\n";
    $input = file($infile);

    $sources[] = get_metadata($infile);

    $templstrings = $headtempl;
    $replstrings  = $headrepl;

    $title = process_title(array_shift($input));
    $templstrings[] = '/<!-- title goes here -->/';
    $replstrings[]  = $title;

    $tags = process_tags(array_shift($input));

    $templstrings[] = '/<!-- meat goes here -->/';
    $replstrings[]  = process_article(implode('', $input));

    $template = file_get_contents($templates . '/pages.html');

    $output = preg_replace($templstrings, $replstrings, $template);

    $ofn = $outputdir . '/' . $ifn . '.html';
    $ofh = fopen($ofn, 'w');
    fwrite($ofh, $output);
    fclose($ofh);
    print "Wrote file: $ofn\n";

  } // endif filename matches a file
} // endforeach looping through files

// FUNCTIONS BELOW

function process_less() {
  global $lessfiles;
  global $templates;
  global $outputdir;
  global $head;
  foreach ($lessfiles as $lessfile) {
    $lessfn = $templates . '/' . $lessfile;
    $cssfile = pathinfo($lessfn, PATHINFO_FILENAME) . '.css';
    $cssfn  = $outputdir . '/' . $cssfile;
    print "Processing file: $lessfn\n";
    try {
      lessc::ccompile($lessfn, $cssfn);
    } // endtry complle lesscss file
    catch (exception $ex) {
      exit('FATAL ERROR in LESS compiler: ' . $ex->getMessage());
    } // endcatch lesscss errors
    print "Wrote file: $cssfn\n";
    $head[] = '<link rel="stylesheet" type="text/css" href="' . $cssfile . '">' . "\n";
  } // endforeach looping through less assets
} // endfunction process_less

function process_title($text) {
  preg_match('/^TITLE: (.*)/', $text, &$title);
  return $title[1];
} // endfunction process_title

function process_tags($text) {
  preg_match('/^TAGS: (.*)/', $text, &$tagtext);
  $tags = explode(',', $tagtext[1]);
  array_walk($tags, create_function('&$text', '$text = trim($text);'));
  return $tags;
} // endfunction process_tags

function process_article($text) {
  return markdown(expand_freelinks($text));
}

function get_metadata($file) {
  global $outputdir;
  $metadata['access_time']  = filemtime($file);
  $metadata['filename']     = pathinfo($file, PATHINFO_FILENAME);
  $metadata['outfile_uri']  = $metadata['filename'] . '.html';
  $metadata['outfile_path'] = $outputdir . '/' . $metadata['outfile_uri'];

  return $metadata;
} // endfunction get_metadata
