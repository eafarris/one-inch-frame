<?php
/*
 * GENERATE
 *
 * Generate the HTML for the site
 */

// EXTERNAL LIBRARIES
require_once('includes/markdownphp/markdown.php');
require_once('includes/lessphp/lessc.inc.php');

// INTERNAL HELPER FUNCTIONS
require_once('functions.inc');

// CONFIGURATION
require_once('config.php');

// INTERNAL FILTERS
require_once('freelinks.inc');

$headrepl = process_less();
$headerrepl = file_get_contents($templates . '/header.html');
$footerrepl  = file_get_contents($templates . '/footer.html');

/*
 * GRAB METADATA
 */
print 'Processing files.';
foreach (glob($datafiles. '/*.mkd')  as $infile) {
  if (is_file($infile)) {
    print '.';
    $sources[] = get_metadata($infile);
  } // endif filename matches a file
} // endforeach looping through files

// sort the metadata in reverse cron order
foreach ($sources as $key => $metadata) {
  $dates[] = $metadata['access_time'];
}
array_multisort($dates, SORT_DESC, $sources);

print "\n";

$sidebarrepl  = generate_recent_content_block($sources);

/*
 * GENERATE INDEX PAGE
 *
 * For index.html, we'll show the full text for the most recent post, 
 * and links for the next 9 (10 total). No sidebar on the index page.
 */

$indexcontent  = '<h1>' . $sources[0]['title'] . "</h1>\n";
$indexcontent .= process_article(file_get_contents($sources[0]['infile']));
$indexcontent .= '<div id="nextnine">' . "\n";
$indexcontent .= "<h2>Recent content</h2>\n<ul>\n";
for ($a = 1; $a < 10; $a++) {
  $indexcontent .= '<li>' . l($sources[$a]) . "\n";
}
$indexcontent .= "</ul>\n";

$meatrepl = $indexcontent;
$tagsrepl = $sources[0]['tags_ul'];

$template = file_get_contents($templates . '/index.html');
$output = expand_template($headrepl, $headerrepl, $footerrepl, $sidebarrepl, NULL, $meatrepl, $tagsrepl, 'index.html');
$ofn = $outputdir . '/index.html';
$ofh = fopen($ofn, 'w');
fwrite($ofh, $output);
fclose($ofh);

/*
 * GENERATE PAGES OUTPUT
 */
foreach ($sources as $key => $metadata) {
  $titlerepl = $metadata['title'];
  $tagsrepl = $metadata['tags_ul'];
  $meatrepl = process_article(file_get_contents($metadata['infile']));
  $template = file_get_contents($templates . '/pages.html');

  $output = expand_template($headrepl, $headerrepl, $footerrepl, $sidebarrepl, $titlerepl, $meatrepl, $tagsrepl, 'pages.html');

  $ofn = $metadata['outfile_path'];
  $ofh = fopen($ofn, 'w');
  fwrite($ofh, $output);
  fclose($ofh);
} // endforeach looping through sources array

/*
 * GENERATE TAGS PAGES
 */

$alltags = array();
foreach ($sources as $file => $metadata) {
  $alltags = array_merge($alltags, $metadata['tags']);
}
$tags = array_unique($alltags);

foreach ($sources as $key => $metadata) {
  foreach ($metadata['tags'] as $tag) {
    $tagged[$tag] .= '<li><a href="' . $webrooturl . '/' . $metadata['outfile_uri'] . '">' . $metadata['title'] . '</a>';
  } // endforeach looping through tags
} // endforeach

$tagindex = "<ul>\n";

foreach ($tags as $tag) {
  $tagfn = $tag . '.html';
  $tagsrepl = 'Tag: ' . $tag;
  $meatrepl = '<ul>' . $tagged[$tag] . '</ul>';

  $tagindex .= '<li><a href="' . $webrooturl . '/tags/' . $tagfn . '">' . $tag . "</a>\n";
  
  $output = expand_template($headrepl, $headerrepl, $footerrepl, $sidebarrepl, $titlerepl, $meatrepl, $tagsrepl, 'pages.html');
  $ofn = $outputdir . '/tags/' . $tagfn;
  $ofh = fopen($ofn, 'w');
  fwrite($ofh, $output);
  fclose($ofh);
} // endforeach looping through tags

$tagindex .= "</ul>\n";

$titlerepl = 'Tag index';
$meatrepl = $tagindex;
$output = expand_template($headrepl, $headerrepl, $footerrepl, $sidebarrepl, $titlerepl, $meatrepl, $tagsrepl, 'index.html');
$ofn = $outputdir . '/tags/index.html';
$ofh = fopen($ofn, 'w');
fwrite($ofh, $output);
fclose($ofh);
