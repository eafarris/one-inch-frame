<?php
/*
 * GENERATE
 *
 * Generate the HTML for the site
 */

// EXTERNAL LIBRARIES
require_once('includes/lessphp/lessc.inc.php');

// INTERNAL HELPER FUNCTIONS
require_once('functions.inc');

// CONFIGURATION
require_once('config.php');

// INTERNAL FILTERS
require_once('freelinks.inc');

$replacements['head'] = process_less();
$replacements['header'] = file_get_contents($templates . '/header.html');
$replacements['footer']  = file_get_contents($templates . '/footer.html');

/*
 * GRAB METADATA
 */
foreach (glob($datafiles. '/*.md')  as $infile) {
  if (is_file($infile)) {
    $sources[] = get_metadata($infile);
  } // endif filename matches a file
} // endforeach looping through files

$sources = blog_order($sources);
// $sources (metadata array) is now sorted in reverse cron (ie., "blog") 
// order. $sources[0] is the most recently edited content.

build_rss_feed($sources);

// generate a titles array to search for supertags later
foreach ($sources as $key => $metadata) {
  $titles[$key] = strtolower($metadata['title_text']);
}

$replacements['sidebar'] = generate_recent_content_block($sources);

/*
 * GENERATE INDEX PAGE
 *
 * For index.html, we'll show the full text for the most recent post, 
 * and links for the next 15. No sidebar on the index page.
 */

$indexcontent  = '<h1>' . $sources[0]['title'] . "</h1>\n";

if ($sources[0]['type'] == 'image') {
  $indexcontent .= '<img src="' . $sources[0]['url'] . '" />';
}
$indexcontent .= process_article(file_get_contents($sources[0]['infile']));
$bottomcontent = "<h2>Recent content</h2>\n<ul>\n";
for ($a = 1; $a < 15; $a++) {
  $bottomcontent .= '<li>' . l($sources[$a]) . "\n";
}
$bottomcontent .= "</ul>\n";

$replacements['meat']   = $indexcontent;
$replacements['bottom'] = $bottomcontent;
$replacements['date']   = date('j F Y', $sources[0]['posted']);
$replacements['tags']   = $sources[0]['tags_ul'];

$template = file_get_contents($templates . '/index.html');
$output = expand_template($replacements, 'index.html');
$ofn = $outputdir . '/index.html';
$ofh = fopen($ofn, 'w');
fwrite($ofh, $output);
fclose($ofh);

/*
 * GENERATE PAGES OUTPUT
 */
print 'Generating files.';
foreach ($sources as $key => $metadata) {
  $output = generate_content($metadata);
  $ofn = $metadata['outfile_path'];
  $ofh = fopen($ofn, 'w');
  fwrite($ofh, $output);
  fclose($ofh);
  print '.';
} // endforeach looping through sources array
print "\n";

/*
 * GENERATE TAGS PAGES
 */

$alltags = array();
foreach ($sources as $file => $metadata) {
  $alltags = array_merge($alltags, $metadata['tags']);
}
$tags = array_unique($alltags);
sort($tags);

foreach ($sources as $key => $metadata) {
  foreach ($metadata['tags'] as $tag) {
    $tagline = '<li><a href="' . $webrooturl . '/' . $metadata['outfile_uri'] . '">' . $metadata['title'] . '</a>';
    $tagged[$tag] = isset($tagged[$tag]) ? $tagged[$tag] . $tagline : $tagline;
  } // endforeach looping through tags
} // endforeach

$tagindex = "<ul>\n";

print 'Generating tag pages.';
foreach ($tags as $tag) {
  $tagfn = $tag . '.html';
  $tagindex .= '<li><a href="' . $webrooturl . '/tags/' . $tagfn . '">' . $tag . "</a>\n";
  $supertag = array_search($tag, $titles);
  if ($supertag) { // this tag should be the same as the normal article
    $supertagcontent = generate_content($sources[$supertag], false);
    $replacements['title'] = $sources[$supertag]['title'];
    $replacements['meat']  = $supertagcontent . '<h2>Related posts</h2><ul>' . $tagged[$tag] . '</ul>';
    $replacements['tags']  = $sources[$supertag]['tag_ul'];
  } // endif supertag
  else {
    $replacements['title'] = 'Tag: ' . $tag;
    $replacements['meat'] = '<ul>' . $tagged[$tag] . '</ul>';
    $replacements['tags'] = '';
  } // endif not supertag
  $output = expand_template($replacements, 'pages.html');
  $ofn = $outputdir . '/tags/' . $tagfn;
  $ofh = fopen($ofn, 'w');
  fwrite($ofh, $output);
  fclose($ofh);
  print '.';

} // endforeach looping through tags
print "\n";

// GENERATE TAG INDEX PAGE

$tagindex .= "</ul>\n";

$replacements['title'] = 'Tag index';
$replacements['meat']  = $tagindex;
$output = expand_template($replacements, 'tagindex.html');
$ofn = $outputdir . '/tags/index.html';
$ofh = fopen($ofn, 'w');
fwrite($ofh, $output);
fclose($ofh);
print "Generated tag index file.\n";
