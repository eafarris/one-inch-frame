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

// read metadata from the file. That's all. Processing is done by process_metadata().
function get_metadata($file) {
  global $outputdir;
  global $webrooturl;
  $contents = file($file, FILE_IGNORE_NEW_LINES); // whole file is in array by line
  $add_to_metadata = FALSE;
  for ($a = 0; $a < count($contents); $a++) {
    if ($contents[$a] == '-->') { // end of metadata
      $add_to_metadata = FALSE;
      break;
    }
    if ($add_to_metadata) {
      $lines[] = $contents[$a];
    }
    if ($contents[$a] == '<!-- METADATA') { // start of metadata
      $add_to_metadata = TRUE;
    }
  } // endfor looping through file
  foreach ($lines as $line) {
    $data = preg_split('/: /', $line); // split on [colon][space]
    $key = strtolower($data[0]);
    if ($key == 'tags') { // expand tags
      $tags = explode(',', $data[1]);
      array_walk($tags, create_function('&$text', '$text = trim($text);'));
      $metadata['tags'] = $tags;
    }
    else {
      $metadata[$key] = $data[1];
    }
  } //endforeach looping through metadata

  // right now, they're the same, but process_metadata() could change that.
  $metadata['title_text'] = $metadata['title'];

  // now add stuff we can get from the file itself
  $metadata['infile'] = $file;
  $metadata['filename'] = pathinfo($file, PATHINFO_FILENAME);
  $metadata['outfile_uri'] = $metadata['filename'] . '.html';
  $metadata['outfile_path'] = $outputdir . '/' . $metadata['outfile_uri'];

  // posted becomes date, unless there's no date
  $metadata['posted'] = empty($metadata['date']) ? filemtime($file) : $metadata['date'];

  return process_metadata($metadata);
} //endfunction get_metadata

// process metadata. This will expand the metadata array based on itself.
function process_metadata(&$metadata) {
  global $webrooturl;
  if (!empty($metadata['type'])) { // if there's a type, how to handle?
    switch ($metadata['type']) {
    case 'link': // wrap the title in an anchor tag, manipulate webshot
      $metadata['title'] = '<a href="' . $metadata['url'] . '">' . $metadata['title_text'] . '</a>';
      $metadata['image_url'] = make_webshot($metadata['url'], $metadata['posted']);
      $metadata['post_content'] = '<div class="webshot"><img src="' . $metadata['image_url'] . '"></div><!-- webshot -->';
      break;
    case 'quote': // pre and post with a div
      $metadata['pre_content'] = '<div class="quote">';
      $metadata['post_content'] = '</div><!-- quote -->';
      break;
    case 'image': // add image to pre-content
      $metadata['pre_content'] = '<div class="image"><img src="' . $metadata['url'] . '"></div><!-- image -->';
      break;
    default:
      break;
    } // endswitch type
  } // endif type

  // make tags_ul
  $metadata['tags_ul'] = '<ul class="tags">';
  foreach ($metadata['tags'] as $tag) {
    $metadata['tags_ul'] .= '<li><a href="' . $webrooturl . '/tags/' . $tag . '.html">' . $tag . '</a>';
  }
  $metadata['tags_ul'] .= '</ul><!-- tags -->';

  return $metadata;
} // endfunction process_metadata

// return the sources array sorted in reverse chron (ie., blog) order
function blog_order($sources) {
  foreach ($sources as $source => $data) {
    $dates[] = $data['posted'];
  }
  array_multisort ($dates, SORT_DESC, $sources);
  return $sources;
}


/*
function get_metadata($file) {

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
 */

// function l -- returns HTML link to a file
function l($file) {
  global $webrooturl;
  $output = '<a href="' . $webrooturl . '/' . $file['outfile_uri'] . '">' . $file['title_text'] . '</a>';
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
  $patterns['head']       = '/<head>/';
  $patterns['header']     = '/<!-- header goes here -->/';
  $patterns['footer']     = '/<!-- footer goes here -->/';
  $patterns['sidebar']    = '/<!-- sidebar content goes here -->/';
  $patterns['title_text'] = '/<!-- title_text goes here -->/';
  $patterns['title']      = '/<!-- title goes here -->/';
  $patterns['date']       = '/<!-- date goes here -->/';
  $patterns['meat']       = '/<!-- meat goes here -->/';
  $patterns['tags']       = '/<!-- tags go here -->/';
  $patterns['bottom']     = '/<!-- bottom section goes here -->/';
  $replacements['head']   = "<head>\n" . implode('', $replacements['head']);
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
  global $webrooturl;
  $content = process_article(file_get_contents($metadata['infile']));
  $content .= '<p>This is a ' . $metadata['type'] .' post.</p>';
  if ($metadata['type'] == 'link') {
    $content .= '<div class="webshot"><img src="' . $webrooturl . '/images/webshots/' . $metadata['posted'] . '-clipped.png"></div><!-- webshot -->';
  }
  if (!$fullpage) {
    return $content;
  } // endif don't generate full page
  $replacements['title_text'] = $metadata['title_text'];
  $replacements['title']      = $metadata['title'];
  $replacements['tags']       = $metadata['tags_ul'];
  $replacements['meat']       = $content;
  $replacements['date']       = date('j F Y', $metadata['posted']);
  $template = 'pages.html';
  $output = expand_template($replacements, $template);
  return $output;
} // endfunction generate_content

// if it doesn't already exist, make a screenshot from a URL with webkit2png, name it after the 'posted' metadata
// return the URI of the file
function make_webshot($url, $posted) {
  global $outputdir;
  global $webkit2png;
  global $webrooturl;

  if (file_exists($outputdir . '/images/webshots/' . $posted . '-clipped.png')) {
    print "File for this URL already exists. Not created.\n";
  }
  else {
    $commandline = $webkit2png . '--dir=' . $outputdir . '/images/webshots --filename=' . $posted . ' ' . $url;
    //$output = shell_exec($commandline);
  }
  return $webrooturl . '/images/webshots/' . $posted . '-clipped.png';
}


// vim: tw=300 nowrap syn=php
