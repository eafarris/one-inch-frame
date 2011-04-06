<?php
/*
 * HELPER FUNCTIONS
 */

function process_less() {
  global $webrooturl;
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
    $head[] = '<link rel="stylesheet" type="text/css" href="' . $webrooturl . '/' . $cssfile . '">' . "\n";
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
  $contents = file_get_contents($file);
  $metadata['infile']       = $file;
  $metadata['access_time']  = filemtime($file);
  $metadata['filename']     = pathinfo($file, PATHINFO_FILENAME);
  $metadata['outfile_uri']  = $metadata['filename'] . '.html';
  $metadata['outfile_path'] = $outputdir . '/' . $metadata['outfile_uri'];

  preg_match('/^TITLE: (.*)/m' , $contents, &$title);
  $metadata['title'] = $title[1];

  preg_match('/^TAGS: (.*)/m', $contents, &$tagstring);
  $tags = explode(',', $tagstring[1]);
  array_walk($tags, create_function('&$text', '$text = trim($text);'));
  $metadata['tags'] = $tags;
  $metadata['tags_ul'] = "<ul>\n";
  foreach ($tags as $tag) {
    $metadata['tags_ul'] .= '<li><a class="tag" href="tags/' . $tag .  '.html">' . $tag . "</a>\n";
  } // endforeach looping through tags
  $metadata['tags_ul'] .= "</ul>";

  foreach ($metadata as $file => $data) {
    $dates[] = $data['access_time'];
  }
  array_multisort ($metadata, SORT_DESC, $dates);
  // $sources array now sorted in reverse cron order

  return $metadata;
} // endfunction get_metadata

// function l -- returns HTML link to a file
function l($file) {
  global $webrooturl;
  $output = '<a href="' . $webrooturl . '/' . $file['outfile_uri'] . '">' . $file['title'] . '</a>';
  return $output;
}

function generate_recent_content_block($sources) {
  $block  = '<div id="recent-content" class="block">' . "\n";
  $block .= "<h2>Recent content</h2>\n";
  $block['output'] = "<ol>\n";
  for ($a = 0; $a < 5; $a++) {
    $block .= '<li>' . l($sources[$a]). "\n";;
  } // endfor need first five from array
  $block .= "</div><!-- recent-content -->\n";
  return $block;
} // endfunction generate_recent_content_block

