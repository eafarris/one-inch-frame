<?php
/*
 * HELPER FUNCTIONS
 */

function process_less() {
  global $webrooturl;
  global $lessfiles;
  global $templates;
  global $outputdir;
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
  return $head;
} // endfunction process_less

function process_article($text) {
  global $mmdpath;
  // to use php markdown:
  // $output =  markdown(expand_freelinks($text));
  $tempname = tempnam('/tmp', 'php');
  $temphandle = fopen($tempname, 'w');
  fwrite($temphandle, expand_freelinks($text));
  fclose($temphandle);
  $output = shell_exec("cat $tempname | $mmdpath");
  unlink($tempname);
  return $output;
}

function get_metadata($file) {
  global $outputdir;
  global $webrooturl;
  $contents = file_get_contents($file);
  $metadata['infile']       = $file;
  $metadata['filename']     = pathinfo($file, PATHINFO_FILENAME);
  $metadata['outfile_uri']  = $metadata['filename'] . '.html';
  $metadata['outfile_path'] = $outputdir . '/' . $metadata['outfile_uri'];

  preg_match('/^DATE: (.*)/m', $contents, &$posted);
  $accessed_time = filemtime($file);
  if (empty($posted[1])) {
    $metadata['posted'] = $accessed_time;
  }
  else {
    $metadata['posted'] = $posted[1];
  }

  preg_match('/^TITLE: (.*)/m' , $contents, &$title);
  $metadata['title_text'] = $title[1];

  preg_match('/^TYPE: (.*)/m', $contents, &$type);
  $metadata['type'] = $type[1];

  preg_match('/^URL: (.*)/m', $contents, &$url);
  $metadata['url'] = $url[1];

  // if this is a link, wrap the title in an anchor tag
  if ($metadata['type'] == 'link') {
    $metadata['title'] = '<a href="' . $metadata['url'] . '">' . $metadata['title'] . '</a>';
  }
  else {
    $metadata['title'] = $metadata['title_text'];
  }

  preg_match('/^TAGS: (.*)/m', $contents, &$tagstring);
  $tags = explode(',', $tagstring[1]);
  array_walk($tags, create_function('&$text', '$text = trim($text);'));
  $metadata['tags'] = $tags;
  $metadata['tags_ul'] = "<ul>\n";
  foreach ($tags as $tag) {
    $metadata['tags_ul'] .= '<li><a class="tag" href="' . $webrooturl . '/tags/' . $tag .  '.html">' . $tag . "</a>\n";
  } // endforeach looping through tags
  $metadata['tags_ul'] .= "</ul>";

  foreach ($metadata as $file => $data) {
    $dates[] = $data['posted'];
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

// Takes content and a template file and returns a generated page
function expand_template($replacements, $templatefile) {
  global $templates;
  $patterns['head']    = '/<head>/';
  $patterns['header']  = '/<!-- header goes here -->/';
  $patterns['footer']  = '/<!-- footer goes here -->/';
  $patterns['sidebar'] = '/<!-- sidebar content goes here -->/';
  $patterns['title']   = '/<!-- title goes here -->/';
  $patterns['date']    = '/<!-- date goes here -->/';
  $patterns['meat']    = '/<!-- meat goes here -->/';
  $patterns['tags']    = '/<!-- tags go here -->/';
  $patterns['bottom']  = '/<!-- bottom section goes here -->/';
  $replacements['head']  = "<head>\n" . implode('', $replacements['head']);
  if (empty($replacements['bottom'])) {
    $replacements['bottom'] = '';
  }
  ksort($patterns);
  ksort($replacements);

  $template = file_get_contents($templates . '/' . $templatefile);
  $output = preg_replace($patterns, $replacements, $template);

  return $output;
} // endfunction expand_template

// takes a slice of $metadata and returns the HTML (page or snippet)
function generate_content($metadata, $fullpage = TRUE) {
  global $templates;
  global $replacements;
  $content = process_article(file_get_contents($metadata['infile']));
  if (!$fullpage) {
    return $content;
  } // endif don't generate full page
  $replacements['title'] = $metadata['title'];
  $replacements['tags']  = $metadata['tags_ul'];
  $replacements['meat']  = $content;
  $replacements['date']  = date('j F Y', $metadata['posted']);
  $template = 'pages.html';
  $output = expand_template($replacements, $template);
  return $output;
} // endfunction generate_content

// vim: tw=300 nowrap syn=php
