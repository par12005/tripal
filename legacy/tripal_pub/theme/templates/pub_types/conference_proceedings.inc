<?php
// the $headers array is an array of fields to use as the colum headers. 
// additional documentation can be found here 
// https://api.drupal.org/api/drupal/includes%21theme.inc/function/theme_table/7
// This table for the analysis has a vertical header (down the first column)
// so we do not provide headers here, but specify them in the $rows array below.
$headers = [];

// the $rows array contains an array of rows where each row is an array
// of values for each column of the table in that row.  Additional documentation
// can be found here:
// https://api.drupal.org/api/drupal/includes%21theme.inc/function/theme_table/7 
$rows = [];

// Title row
$title = '';
if ($url) {
  $title = l(htmlspecialchars($pub->title), $url, ['attributes' => ['target' => '_blank']]);
}
elseif ($dbxref and $dbxref->db_id->urlprefix) {
  $title = l(htmlspecialchars($pub->title), $dbxref->db_id->urlprefix . $dbxref->accession, ['attributes' => ['target' => '_blank']]);
}
else {
  $title = htmlspecialchars($pub->title);
}
$rows[] = [
  [
    'data' => 'Title',
    'header' => TRUE,
    'width' => '20%',
  ],
  $title,
];
// Authors row
$rows[] = [
  [
    'data' => 'Authors',
    'header' => TRUE,
  ],
  $authors_list,
];
// Type row
$rows[] = [
  [
    'data' => 'Type',
    'header' => TRUE,
  ],
  $pub->type_id->name,
];
// Media Title
$rows[] = [
  [
    'data' => 'Conference Name',
    'header' => TRUE,
    'nowrap' => 'nowrap',
  ],
  $pub->series_name,
];
// Volume
$rows[] = [
  [
    'data' => 'Volume',
    'header' => TRUE,
  ],
  $pub->volume ? $pub->volume : 'N/A',
];
// Issue
$rows[] = [
  [
    'data' => 'Issue',
    'header' => TRUE,
  ],
  $pub->issue ? $pub->issue : 'N/A',
];
// Year
$rows[] = [
  [
    'data' => 'Year',
    'header' => TRUE,
  ],
  $pub->pyear,
];
// Pages
$rows[] = [
  [
    'data' => 'Page(s)',
    'header' => TRUE,
  ],
  $pub->pages ? $pub->pages : 'N/A',
];
// Citation row
$rows[] = [
  [
    'data' => 'Citation',
    'header' => TRUE,
  ],
  htmlspecialchars($citation->value),
];
// allow site admins to see the pub ID
if (user_access('administer tripal')) {
  // Pub ID
  $rows[] = [
    [
      'data' => 'Pub ID',
      'header' => TRUE,
      'class' => 'tripal-site-admin-only-table-row',
    ],
    [
      'data' => $pub->pub_id,
      'class' => 'tripal-site-admin-only-table-row',
    ],
  ];
}
// Is Obsolete Row
if ($pub->is_obsolete == TRUE) {
  $rows[] = [
    [
      'data' => '<div class="tripal_pub-obsolete">This publication is obsolete</div>',
      'colspan' => 2,
    ],
  ];
}
// the $table array contains the headers and rows array as well as other
// options for controlling the display of the table.  Additional
// documentation can be found here:
// https://api.drupal.org/api/drupal/includes%21theme.inc/function/theme_table/7
$table = [
  'header' => $headers,
  'rows' => $rows,
  'attributes' => [
    'id' => 'tripal_pub-table-base',
    'class' => 'tripal-data-table',
  ],
  'sticky' => FALSE,
  'caption' => '',
  'colgroups' => [],
  'empty' => '',
];

// once we have our table array structure defined, we call Drupal's theme_table()
// function to generate the table.
print theme_table($table);
if ($abstract_text) { ?>
    <p><b>Abstract</b></p>
    <div style="text-align: justify"><?php print $abstract_text; ?></div> <?php
} 