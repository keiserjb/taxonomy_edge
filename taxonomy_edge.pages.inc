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
  $form['taxonomy_edge_build_realtime'] = array(
    '#title' => t('Build tree realtime'),
    '#description' => t('Update tree upon taxonomy modification.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('taxonomy_edge_build_realtime', TAXONOMY_EDGE_BUILD_REALTIME),
  );

  $form = system_settings_form($form);
  $form['buttons']['rebuild_edges'] = array(
    '#value' => t("Rebuild edges"),
    '#type' => 'submit',
    '#submit' => array('taxonomy_edge_rebuild_submit')
  );
  
  return $form;
}

/**
 * Submit callback; rebuild edges.
 *
 * @ingroup forms
 */
function taxonomy_edge_rebuild_submit($form, &$form_state) {
  taxonomy_edge_rebuild_batch();
}
