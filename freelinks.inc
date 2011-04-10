<?php
/*
 * FREELINKING
 *
 * Allows wiki-like [[linking]] between content and [[w:external sources]].
 * Based on some of my old code for the freelinking Drupal module.
 */

function expand_freelinks($text) {
  global $webrooturl;
  $regex = '/\[\[([^:]+)]]/Uu'; // find freelinks that don't contain a colon¬
  $matches = preg_match_all($regex, $text, &$simplematches, PREG_SET_ORDER);
  if ($matches > 0) { // found at least one freelink
    foreach ($simplematches as $match) { // loop through simple freelinks
      $patterns[] = '/' . preg_quote($match[0]) . '/';
      $replacements[] = '<a href="' . $webrooturl . '/'. strtr($match[1], ' ', '-') . '.html">' . $match[1] . '</a>';
    } // endforeach looping through simple matches
  } // endif at least one simple match

  // wikipedia links
  $regex = '/\[\[w:(.+)]]/Uu';
  $matches = preg_match_all($regex, $text, &$wikimatches, PREG_SET_ORDER);
  if ($matches > 0) { // found at least one wikipedia link
    foreach ($wikimatches as $match) { // loop through wikipedia freelinks
      $patterns[] = '/' . preg_quote($match[0]) . '/';
      $replacements[] = '<a href="http://en.wikipedia.org/wiki/' . strtr($match[1], ' ', '_') . '">' . $match[1] . '</a>';
    } // endforeach looping through wikipedia matches
  } // endif at least one wikipedia match

  if (!empty($patterns)) {
    return (preg_replace($patterns, $replacements, $text));
  }
  else {
    return $text;
  }
} // endfunction expand_freelinks

// vim: tw=300 nowrap syn=php
