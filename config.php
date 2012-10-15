<?php
/*
 * config file for OIF baked blogging platform
 *
 */

// TZ setting
date_default_timezone_set('America/New_York');

// WEBROOTURL: Full URL to the root of this website. No trailing slash.
$webrooturl = 'http://eafarris.com';

// DATAFILES: path where raw posts are stored
$datafiles = '/Users/eafarris/Dropbox/oif';

// TEMPLATES: where to find the templates
$templates = '/Users/eafarris/Dropbox/oif/templates';

// OUTPUTDIR: where to place the output files
$outputdir = '/Users/eafarris/Dropbox/Sites';

// MMDPATH: full system path to the Multimarkdown binary
$mmdpath = '/usr/local/bin/mmd';

// TWEETSPATH: full system path to the tweets.txt file (created by Dr. 
//   Drang's archive-tweets.py script
$twitterpath = '/Users/eafarris/Dropbox/twitter/twitter.txt';

// WEBSHOTSCRIPT: full path to a shell script that will make a suitable 
// screenshot of a webpage. The example script uses webkit2png and 
// imagemagick.
//   
//   The script will be called with three parameters:
//     1. The URL of the page to be shot
//     2. The "posted" metadata, which is a unixtime that will be used 
//        to make the file and keep it unique.
//     3. The directory in which the final file should be placed.
//  
//  The make_webshot() function in functions.inc expects that this 
//  script will create a file called $posted.png, where "$posted" is the 
//  posted metadata, described above. 

$webshotscript = '/Users/eafarris/one-inch-frame/webshot.sh';

