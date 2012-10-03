<?php
/*
 * config file for OIF baked blogging platform
 *
 */

// TZ setting
date_default_timezone_set('America/New_York');

// WEBROOTURL: Full URL to the root of this website
$webrooturl = 'http://static.eafarris.com';

// DATAFILES: path where raw posts are stored
$datafiles = '/Users/eafarris/Dropbox/oif';

// TEMPLATES: where to find the templates
$templates = '/Users/eafarris/Dropbox/oif/templates';

// OUTPUTDIR: where to place the output files
$outputdir = '/Users/eafarris/Sites';

// LESSFILES: array of files in lesscss format in TEMPLATES directory
$lessfiles[] = 'oif.less';

// MMDPATH: full system path to the Multimarkdown binary
$mmdpath = '/usr/local/bin/mmd';

// TWEETSPATH: full system path to the tweets.txt file (created by Dr. 
//   Drang's archive-tweets.py script
$tweetspath = '/Users/eafarris/Dropbox/twitter/tweets.txt';

// WEBKIT2PNG: full path and options for webkit2png (paul hammond)
//   function make_web_screenshot will add '--dir' and '--filename' 
//   options
$webkit2png = '/Users/eafarris/webkit2png/webkit2png --clipped --clipwidth=600 --clipheight=400 ';
