<?php
/**
 * @file
 * Pages for taxonomy edge settings and more.
 */

/**
 * Form build for the settings form.
 *
 * @see taxonomy_edge_rebuild_submit()
 * @ingroup forms
 */
function taxonomy_edge_settings_form() {
  $config = config('taxonomy_edge.settings');
  $form = array();
  $form['#config'] = 'taxonomy_edge.settings';

  $form['taxonomy_edge_max_depth'] = array(
    '#title' => t('Maximum depth'),
    '#description' => t('Fail safe for avoiding infite loops when rebuilding edges.'),
    '#type' => 'textfield',
    '#default_value' => $config->get('taxonomy_edge_max_depth'),
  );
  $form['taxonomy_edge_build_realtime'] = array(
    '#title' => t('Build tree realtime'),
    '#description' => t('Update tree upon taxonomy modification.'),
    '#type' => 'checkbox',
    '#default_value' => $config->get('taxonomy_edge_build_realtime'),
  );
  $form['taxonomy_edge_static_caching'] = array(
    '#title' => t('Use static caching'),
    '#description' => t('Use static caching for taxoomy_get_tree(). If experiencing memory exhausts, try disabling this.'),
    '#type' => 'checkbox',
    '#default_value' => $config->get('taxonomy_edge_static_caching'),
  );
  $form['taxonomy_edge_optimized_get_tree'] = array(
    '#title' => t('Use optimized version of taxonomy_get_tree'),
    '#description' => t('Taxonomy Edge implements two versions of taxonomy_get_tree(). One that acts just like the core version except it can select subtrees but still sorts the entire vocabulary on the DB, and one that can do select and sorting on subtrees only. Check this box to select the latter one. CPU wise, this method can sometimes be slower.'),
    '#type' => 'checkbox',
    '#default_value' => $config->get('taxonomy_edge_optimized_get_tree'),
  );
  $form['taxonomy_edge_override_term_pages'] = array(
    '#title' => t('Override term pages'),
    '#description' => t('Override taxonomy/term/%term pages (and feed pages also) supporting the depth modifier argument.'),
    '#type' => 'checkbox',
    '#default_value' => $config->get('taxonomy_edge_override_term_pages'),
  );


  $form['rebuild'] = array(
    '#type' => 'fieldset',
    '#title' => t('Rebuild'),
  );
  $form['rebuild']['rebuild_edges'] = array(
    '#markup' => l(t('Rebuild all edges'), 'admin/structure/taxonomy/rebuild/edges', array('query' => backdrop_get_destination())) . '<br/>',
  );
  $form['rebuild']['rebuild_order'] = array(
    '#markup' => l(t('Rebuild all orders'), 'admin/structure/taxonomy/rebuild/order', array('query' => backdrop_get_destination())) . '<br/>',
  );
  $form['rebuild']['rebuild_all'] = array(
    '#markup' => l(t('Rebuild EVERYTHING'), 'admin/structure/taxonomy/rebuild/all', array('query' => backdrop_get_destination())) . '<br/>',
  );

  $form = system_settings_form($form);
  $form['#submit'][] = 'taxonomy_edge_system_settings_form_submit_custom';
  return $form;
}

/**
 * Submit handler for rebuild menu after saving.
 *
 * Changing the override term pages setting requires rebuild of menu.
 * Since we have no proper atomic way of detecting whether or not the
 * setting has actually changed (theoretical race-condition), we always
 * rebuild when saving these settings.
 */
function taxonomy_edge_system_settings_form_submit_custom($form, &$form_state) {
  // If a rebuild is already in progress, chances are that our new settings
  // is not part of the rebuild. We therefore wait for the rebuild to finish
  // and then rebuild again.
  if (!lock_acquire('menu_rebuild')) {
    lock_wait('menu_rebuild');
    if (!lock_acquire('menu_rebuild')) {
      backdrop_set_message(t("Could not rebuild menu. Please rebuild menu / clear cache to make the term pages override take effect."), 'error');
      return;
    }
  }
  menu_rebuild();
}

/**
 * Confirmation for rebuilding trees.
 */
function taxonomy_edge_rebuild_page_confirm($form, &$form_state, $type, $vocabulary) {
  $form = array();
  if (!empty($vocabulary)) {
    $vocabulary = taxonomy_vocabulary_machine_name_load($vocabulary);

    if (!lock_may_be_available('taxonomy_edge_rebuild_' . $type . '_' . $vocabulary->vid)) {
      backdrop_set_message(t('Rebuild already in progress'), 'warning');
      backdrop_goto();
    }

    $form['vocabulary'] = array(
      '#type' => 'value',
      '#value' => $vocabulary,
    );
    $msg = t('Are you sure you want to rebuild %type for vocabulary %name?', array(
      '%type' => $type,
      '%name' => $vocabulary->name,
    ));
  }
  else {
    $msg = t('Are you sure you want to rebuild %type for all vocabularies?', array(
      '%type' => $type,
    ));
  }

  $form['type'] = array(
    '#type' => 'value',
    '#value' => $type,
  );
  return confirm_form($form, $msg, 'admin/structure/taxonomy');
}

/**
 * Submit callback; rebuild tree.
 *
 * @ingroup forms
 */
function taxonomy_edge_rebuild_page_confirm_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/taxonomy';
  module_load_include('rebuild.inc', 'taxonomy_edge');

  if (!empty($form_state['values']['vocabulary']->vid)) {
    $vids = array($form_state['values']['vocabulary']->vid);
  }
  else {
    $vocabularies = taxonomy_get_vocabularies();
    foreach ($vocabularies as $vocabulary) {
      $vids[] = $vocabulary->vid;
    }
  }

  switch ($form_state['values']['type']) {
    case 'edges':
      return taxonomy_edge_rebuild_edges_batch($vids);

    case 'order':
      return taxonomy_edge_rebuild_order_batch($vids);

    case 'all':
      return taxonomy_edge_rebuild_all_batch($vids);

  }
}
