<?php
/*
 * HELPER FUNCTIONS
 */

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
  $metadata['title_text'] = strtr($metadata['title'], array('$' => '&#36;'));

  // now add stuff we can get from the file itself
  $metadata['infile'] = $file;
  $metadata['filename'] = pathinfo($file, PATHINFO_FILENAME);
  $metadata['outfile_uri'] = $metadata['filename'] . '.html';
  $metadata['outfile_path'] = $outputdir . '/' . $metadata['outfile_uri'];

  // posted becomes date, unless there's no date
  $metadata['posted'] = empty($metadata['date']) ? filemtime($file) : $metadata['date'];

  // just going to add these, to get rid of potental E_ stuff later
  $metadata['pre_content'] = '';
  $metadata['post_content'] = '';

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
      $metadata['post_content'] = '<div class="webshot"><a href="' . $metadata['url'] . '"><img src="' . $metadata['image_url'] . '"></a></div><!-- webshot -->';
      break;
    case 'quote': // pre and post with a div
      $metadata['pre_content'] = '<div class="quote">';
      $metadata['post_content'] = '</div><!-- quote -->';
      break;
    case 'image': // add image to pre-content
      $metadata['image_url'] = cache_image($metadata['url'], $metadata['posted']);
      if (isset($metadata['source'])) { // we've got a source, link it
        $metadata['pre_content'] = '<div class="image"><a href="' . $metadata['source'] . '"><img src="' . $metadata['image_url'] . '"></a></div><!-- image -->';
      }
      else {
        $metadata['pre_content'] = '<div class="image"><img src="' . $metadata['image_url'] . '"></div><!-- image -->';
      } // endifelse source
      break;
    case 'video': // get thumbnail, add embed
      $metadata['image_url'] = get_video_thumbnail($metadata['url'], $metadata['posted']);
      $metadata['pre_content'] = '<div class="image"><img src="' . $metadata['image_url'] . '"></div><!-- image -->';
      $metadata['post_content'] = '<div class="embed">' . make_video_embed($metadata['url']) . '</div><!-- embed -->';
    default:
      break;
    } // endswitch type
  } // endif type
  else { // no type, we'll set it to 'unknown'
    $metadata['type'] = 'unknown';
  } // endifelse type

  if (!empty($metadata['source'])) { // Add source and original list to post-content
    $metadata['post_content'] .= '<ul class="sources">';
    $metadata['post_content'] .= '<li><a class="source" href="' . $metadata['source'] . '">(source)</a>';
    $metadata['post_content'] .= isset($metadata['url']) ? '<li><a class="original" href="' . $metadata['url'] . '">(view original)</a>' : '';
    $metadata['post_content'] .= '</ul><!-- sources -->';
  }

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
  $patterns['head']       = '/<!-- head stuff goes here -->/';
  $patterns['header']     = '/<!-- header goes here -->/';
  $patterns['footer']     = '/<!-- footer goes here -->/';
  $patterns['sidebar']    = '/<!-- sidebar content goes here -->/';
  $patterns['title_text'] = '/<!-- title_text goes here -->/';
  $patterns['title']      = '/<!-- title goes here -->/';
  $patterns['date']       = '/<!-- date goes here -->/';
  $patterns['meat']       = '/<!-- meat goes here -->/';
  $patterns['tags']       = '/<!-- tags go here -->/';
  $patterns['bottom']     = '/<!-- bottom section goes here -->/';
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
  $content  = $metadata['pre_content'];
  $content .= process_article(file_get_contents($metadata['infile']));
  $content .= $metadata['post_content'];
  if (!$fullpage) {
    return $content;
  } // endif don't generate full page
  $replacements['title_text'] = $metadata['title_text'];
  $replacements['title']      = $metadata['title'];
  $replacements['tags']       = $metadata['tags_ul'];
  $replacements['meat']       = strtr($content, array('$' => '&#36;'));
  $replacements['date']       = date('j F Y', $metadata['posted']);
  $template = 'pages.html';
  $output = expand_template($replacements, $template);
  return $output;
} // endfunction generate_content

// if it doesn't already exist, make a screenshot from a URL via
// external script. Return the URI of the file
function make_webshot($url, $posted) {
  global $outputdir;
  global $webrooturl;
  global $webshotscript;

  $localdir = $outputdir . '/images/webshots';
  $localimage = $localdir . '/' . $posted . '.png';

  if (!file_exists($localimage)) {
    $commandline = $webshotscript . ' ' . $url . ' ' . $posted . ' ' . $localdir;
    $output = shell_exec($commandline);
  } // endif haven't grabbed this one
  return $webrooturl . '/images/webshots/' . $posted . '.png';
} // endfunction make_webshot

function cache_image($url, $posted) {
  global $outputdir;
  global $webrooturl;

  $localimage = $outputdir . '/images/' . $posted . '.jpg';

  if (!file_exists($localimage)) {
    copy($url, $localimage);
  } // endif haven't grabbed this one
    return $webrooturl . '/images/' . $posted . '.jpg';
} // endfunction cache_image

function build_rss_feed($posts) { // RSS feed of the last 10 items
  global $webrooturl;
  global $outputdir;

  $xml  = '<?xml version="1.0" encoding="iso-8859-1"?>' . "\n";
  $xml .= '<rss version="2.0" xmlns:atom="http://www.w3.org/2005/Atom">' . "\n<channel>\n";
  $xml .= '<atom:link href="http://static.eafarris.com/rss.xml" rel="self" type="application/rss+xml" />';
  $xml .= "<title>eafarris.com</title>\n<link>http://www.eafarris.com</link>\n";
  $xml .= "<description>RSS feed of the last 10 items from eafarris.com</description>\n";
  $xml .= "<lastBuildDate>" . date('r') . "</lastBuildDate>\n<language>en-us</language>\n";
  for ($a = 0 ; $a < 9; $a++) {
    $xml .= "<item>\n";
    $xml .= '<title>' . $posts[$a]['title_text'] . "</title>\n";
    if ($posts[$a]['type'] == 'link') {
      $xml .= '<link>' . htmlspecialchars($posts[$a]['url']) . "</link>\n";
    }
    else {
      $xml .= '<link>' . $webrooturl . '/' . $posts[$a]['outfile_uri'] . "</link>\n";
    }
    $xml .= '<guid>' . $webrooturl . '/' . $posts[$a]['outfile_uri'] . "</guid>\n";
    $xml .= '<pubDate>' . date('r', $posts[$a]['posted']) . "</pubDate>\n";
    $xml .= '<description>' . htmlspecialchars(generate_content($posts[$a], FALSE)) . '</description>';
    $xml .= "</item>\n\n";
  } // endfor looping through items for RSS feed
  $xml .= "</channel>\n</rss>\n";
  $ofn = $outputdir . '/rss.xml';
  $ofh = fopen($ofn, 'w');
  fwrite($ofh, $xml);
  fclose($ofh);
} // endfunction build_rss_feed

function generate_twitter_block() {
  global $twitterpath;

  $tweetblock  = '<div class="tweetblock"><ul class="tweetblock">';

  // read from the end
  $offset = -1;
  $c = '';
  $read = '';
  $i = 0;
  $lines = 50;
  $infilep = @fopen($twitterpath, "r");
  while ( $lines && fseek($infilep, $offset, SEEK_END) >= 0 ) {
    $c = fgetc($infilep);
    if ($c == "\n") {
      $lines--;
      $tweets[] = strrev($read);
      $read='';
    }
    else {
      $read .= $c;
    }
    $offset--;
  }
  for ($a = 0; $a < 10; $a++) {
    $row = $a * 5 + 4;
    $tweet = $tweets[$row];
    $date = $tweets[$row - 1];
    $url = $tweets[$row - 2];

    $tweetblock .= '<li><a class="tweet" href="' . $url . '">' . $tweet . '</a><p class="tweetdate">' . $date . '</p>';
  }
  $tweetblock .= '</ul><!-- tweetblock --></div><!-- tweetblock -->';

  return $tweetblock;
} // endfunction generate_twitter_block

// Creates a map between the 'icon' metadata and the character for
// ModernPictsNormal
function map_icons() {
  $icons = array(
    'camera' => 'A',
    'books' => 'B',
    'music' => 'C',
    'facebook' => 'G',
    'movies' => 'H',
    'dislike' => 'L',
    'ipod' => 'M',
    'iphone' => 'O',
  );
  return $icons;
}

// Returns a local url for a thumbnail of a video. Only handles YouTube
// right now, but could be enhanced for other services.
//
function get_video_thumbnail($url, $posted) {
  global $outputdir;
  global $webrooturl;

  $localimage = $outputdir . '/images/thumbnails/' . $posted . '.jpg';
  if (!file_exists($localimage)) {
    if (preg_match('/youtube.com/', $url)) { // URL is from YouTube
      preg_match('/v=([^&]+)/', $url, &$matches);
      copy ('http://img.youtube.com/vi/' . $matches[1] . '/0.jpg', $localimage);
    } // endif youtube url
  } // endif haven't grabbed this one
  return $webrooturl . '/images/thumbnails/' . $posted . '.jpg';
} // endfunction get_youtube_thumbnail

// Returns an embed HTML snippet for a video. Only handled YouTube right
// now, but could be enhanced for other services.
//
function make_video_embed($url) {
  if (preg_match('/youtube.com/', $url)) { // youtube embed
    preg_match('/v=([^&]+)/', $url, &$matches);
    return '<iframe width="640" height="360" src="http://www.youtube.com/embed/' . $matches[1] . '" frameborder="0" allowfullscreen></iframe>';
  } // endif from youtube
} // endfunction make_video_embed

