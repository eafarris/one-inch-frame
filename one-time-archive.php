<?php
/* one-time-archive.php -- generate site archive for old posts
 *
 */

require_once('config.php');
require_once('functions.inc');
require_once('freelinks.inc');

// grab metadata
foreach (glob($datafiles . '/*.md') as $infile) {
  if (is_file($infile)) {
    $sources[] = get_metadata($infile);
  } // endif is_file
} // endforeach looping through files

// build date array
foreach ($sources as $key => $metadata) {
  $postdate = $metadata['posted'];
  $year = date('Y', $postdate);
  $month = date('m', $postdate);
  $day = date('d', $postdate);
  $dates[$year][$month][$day][] = $metadata['infile'];
}

$archivepath = $outputdir . '/archive';

foreach (array_keys($dates) as $year) {
  foreach(array_keys($dates[$year]) as $month) {
    foreach(array_keys($dates[$year][$month]) as $day) {
      if (!is_dir("$archivepath/$year/$month/$day")) {
        mkdir("$archivepath/$year/$month/$day", 0777, TRUE);
      }
    }
  }
}
