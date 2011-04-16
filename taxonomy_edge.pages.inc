<?php
/**
 * @file
 * 
 * Pages for taxonomy edge settings and more.
 */

/**
 * Form build for the settings form
 *
 * @see taxonomy_edge_rebuild_submit()
 * @ingroup forms
 */
function taxonomy_edge_settings_form() {
  $form = array();

  $form['taxonomy_edge_max_depth'] = array(
    '#title' => t('Maximum depth'),
    '#description' => t('Fail safe for avoiding infite loops when rebuilding edges.'),
    '#type' => 'textfield',
    '#default_value' => variable_get('taxonomy_edge_max_depth', TAXONOMY_EDGE_MAX_DEPTH),
  );
  
  $form['rebuild'] = array(
    '#type' => 'fieldset',
    '#description' => t('Rebuild entire edge list.'),
  );
  $form['rebuild']['rebuild_edges'] = array(
    '#value' => t("Rebuild edges"),
    '#type' => 'submit',
    '#submit' => array('taxonomy_edge_rebuild_submit')
  );

  return system_settings_form($form);
}

/**
 * Submit callback; rebuild edges.
 *
 * @ingroup forms
 */
function taxonomy_edge_rebuild_submit($form, &$form_state) {
  $start = microtime(TRUE);
  $rows = taxonomy_edge_rebuild();
  $end = microtime(TRUE);
  $time = $end - $start;
  if (is_numeric($rows)) {
    drupal_set_message(t('Taxonomy edges rebuilt: %rows processed in %time seconds', array('%rows' => $rows, '%time' => $time)));
  }
  else {
    drupal_set_message(t('Taxonomy edges rebuilded: FAILED'), 'error');
  }
}
