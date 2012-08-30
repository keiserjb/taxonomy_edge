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
  $form['taxonomy_edge_static_caching'] = array(
    '#title' => t('Use static caching'),
    '#description' => t('Use static caching for taxoomy_get_tree(). If experiencing memory exhausts, try disabling this.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('taxonomy_edge_static_caching', TAXONOMY_EDGE_STATIC_CACHING),
  );
  $form['taxonomy_edge_optimized_get_tree'] = array(
    '#title' => t('Use optimized version of taxonomy_get_tree'),
    '#description' => t('Taxonomy Edge implements two versions of taxonomy_get_tree(). One slightly optimized and one a bit more optimized. The more optimized version can in some cases be slower in highly dynamic taxonomy environments.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('taxonomy_edge_optimized_get_tree', TAXONOMY_EDGE_OPTIMIZED_GET_TREE),
  );

  $form = system_settings_form($form);
  return $form;
}

/**
 * Confirmation for rebuilding trees
 */
function taxonomy_edge_rebuild_page_confirm($form, &$form_state, $vocabulary, $type) {
  if (!lock_may_be_available('taxonomy_edge_rebuild_' . $type . '_' . $vocabulary->vid)) {
    drupal_set_message(t('Rebuild already in progress'), 'warning');
    drupal_goto();
  }

  $form = array();
  $form['vocabulary'] = array(
    '#type' => 'value',
    '#value' => $vocabulary
  );
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $type
  );
  return confirm_form($form, t('Are you sure you want to rebuild %type for vocabulary %name?', array(
                                 '%type' => $type,
                                 '%name' => $vocabulary->name
                               )), 'admin/structure/taxonomy');
}

/**
 * Submit callback; rebuild tree.
 *
 * @ingroup forms
 */
function taxonomy_edge_rebuild_page_confirm_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/taxonomy';
  module_load_include('rebuild.inc', 'taxonomy_edge');
  switch ($form_state['values']['type']) {
    case 'edges':
      return taxonomy_edge_rebuild_edges_batch(array($form_state['values']['vocabulary']->vid));
    case 'order':
      return taxonomy_edge_rebuild_order_batch(array($form_state['values']['vocabulary']->vid));
    case 'all':
      return taxonomy_edge_rebuild_all_batch(array($form_state['values']['vocabulary']->vid));
  }
}
