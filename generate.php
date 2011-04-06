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

process_less();
$headtempl = '/<head>/';
$headrepl  = "<head>\n" . implode('', $head);
$headertempl = '/<!-- header goes here -->/';
$headerrepl = file_get_contents($templates . '/header.html');
$footertempl = '/<!-- footer goes here -->/';
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
array_multisort($sources, SORT_DESC, $dates);

print "\n";

$sidebartempl = '/<!-- sidebar content goes here -->/';
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

$templstrings = array();
$replstrings  = array();
$templstrings[] = $headtempl;
$replstrings[]  = $headrepl;
$templstrings[] = $headertempl;
$replstrings[]  = $headerrepl;
$templstrings[] = $footertempl;
$replstrings[]  = $footerrepl;
$templstrings[] = '/<!-- meat goes here -->/';
$replstrings[]   = $indexcontent;

$template = file_get_contents($templates . '/index.html');
$output = preg_replace($templstrings, $replstrings, $template);
$ofn = $outputdir . '/index.html';
$ofh = fopen($ofn, 'w');
fwrite($ofh, $output);
fclose($ofh);

/*
 * GENERATE PAGES OUTPUT
 */
foreach ($sources as $key => $metadata) {
  $templstrings = array();
  $replstrings  = array();
  $templstrings[] = $headtempl;
  $replstrings[]  = $headrepl;
  $templstrings[] = $headertempl;
  $replstrings[]  = $headerrepl;
  $templstrings[] = $footertempl;
  $replstrings[]  = $footerrepl;
  $templstrings[] = $sidebartempl;
  $replstrings[]  = $sidebarrepl;

  $templstrings[] = '/<!-- title goes here -->/';
  $replstrings[]  = $metadata['title'];

  $templstrings[] = '/<!-- tags go here -->/';
  $replstrings[]  = $metadata['tags_ul'];

  $templstrings[] = '/<!-- meat goes here -->/';
  $replstrings[]  = process_article(file_get_contents($metadata['infile']));

  $template = file_get_contents($templates . '/pages.html');

  $output = preg_replace($templstrings, $replstrings, $template);

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
  $templstrings = array();
  $replstrings  = array();
  $templstrings[] = $headtempl;
  $replstrings[]  = $headrepl;
  $templstrings[] = $headertempl;
  $replstrings[]  = $headerrepl;
  $templstrings[] = $footertempl;
  $replstrings[]  = $footerrepl;
  $templstrings[] = $sidebartempl;
  $replstrings[]  = $sidebarrepl;
  $templstrings[] = '/<!-- title goes here -->/';
  $replstrings[]  = 'Tag: ' . $tag;
  $templstrings[] = '/<!-- meat goes here -->/';
  $replstrings[]  = '<ul>' . $tagged[$tag] . '</ul>';

  $tagindex .= '<li><a href="' . $webrooturl . '/tags/' . $tagfn . '">' . $tag . "</a>\n";
  
  $template = file_get_contents($templates . '/pages.html');
  $output = preg_replace($templstrings, $replstrings, $template);
  $ofn = $outputdir . '/tags/' . $tagfn;
  $ofh = fopen($ofn, 'w');
  fwrite($ofh, $output);
  fclose($ofh);
} // endforeach looping through tags

$tagindex .= "</ul>\n";

$templstrings = array();
$replstrings  = array();
$templstrings[] = $headtempl;
$replstrings[]  = $headrepl;
$templstrings[] = $headertempl;
$replstrings[]  = $headerrepl;
$templstrings[] = $footertempl;
$replstrings[]  = $footerrepl;
$templstrings[] = $sidebartempl;
$replstrings[]  = $sidebarrepl;
$templstrings[] = '/<!-- title goes here -->/';
$replstrings[]  = 'Tag index';
$templstrings[] = '/<!-- meat goes here -->/';
$replstrings[]  = $tagindex;
$output = preg_replace($templstrings, $replstrings, $template);
$ofn = $outputdir . '/tags/index.html';
$ofh = fopen($ofn, 'w');
fwrite($ofh, $output);
fclose($ofh);
