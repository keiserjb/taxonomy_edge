<?php
/**
 * @file
 * This file contains the functions for reuilding various tables.
 */

/**
 * Start batch job for rebuild of edges.
 */
function taxonomy_edge_rebuild_edges_batch($vocabularies) {
  $batch = array(
    'finished' => 'taxonomy_edge_rebuild_finished',
    'file' => backdrop_get_path('module', 'taxonomy_edge') . '/taxonomy_edge.rebuild.inc',
    'title' => t('Rebuilding taxonomy edges'),
    'init_message' => t('Rebuilding taxonomy edges'),
  );
  foreach ($vocabularies as $vocabulary) {
    $batch['operations'][] = array('taxonomy_edge_rebuild_edges', array($vocabulary));
  }
  batch_set($batch);
}

/**
 * Start batch job for rebuild of order.
 */
function taxonomy_edge_rebuild_order_batch($vocabularies) {
  $batch = array(
    'finished' => 'taxonomy_edge_rebuild_finished',
    'file' => backdrop_get_path('module', 'taxonomy_edge') . '/taxonomy_edge.rebuild.inc',
    'title' => t('Rebuilding taxonomy edge order'),
    'init_message' => t('Rebuilding taxonomy edge order'),
  );
  foreach ($vocabularies as $vocabulary) {
    $batch['operations'][] = array('taxonomy_edge_rebuild_order', array
    ($vocabulary));  // Correct function call
  }
  batch_set($batch);
}


/**
 * Start batch job for rebuild of edges and order.
 */
function taxonomy_edge_rebuild_all_batch($vocabularies) {
  $batch = array(
    'finished' => 'taxonomy_edge_rebuild_finished',
    'file' => backdrop_get_path('module', 'taxonomy_edge') . '/taxonomy_edge.rebuild.inc',
    'title' => t('Rebuilding taxonomy edges and order'),
    'init_message' => t('Rebuilding taxonomy edges and order'),
  );
  foreach ($vocabularies as $vocabulary) {
    $batch['operations'][] = array('taxonomy_edge_rebuild_edges', array($vocabulary));
    $batch['operations'][] = array('taxonomy_edge_rebuild_order', array($vocabulary));
  }
  batch_set($batch);
}

/**
 * Finished function for rebuild tree batch operation.
 */
function taxonomy_edge_rebuild_finished($success, $results, $operations) {
  if ($success) {
    // Here we do something meaningful with the results.
    $message = theme('item_list', array('items' => $results));
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
  }
  backdrop_set_message($message, 'status');
}

/**
 * Rebuild entire edge list.
 *
 * @return integer
 *   Total number of rows inserted.
 */
function taxonomy_edge_rebuild_edges($vocabulary_name, &$context) {
  $depth = 0;
  $max_depth = config_get('taxonomy_edge.settings', 'taxonomy_edge_max_depth');

  $vocabulary = taxonomy_vocabulary_load($vocabulary_name);
  if (!$vocabulary) {
    $context['success'] = FALSE;
    $context['results'][] = t('Invalid vocabulary: %name', array('%name' => $vocabulary_name));
    return;
  }

  if (!lock_acquire('taxonomy_edge_rebuild_edges_' . $vocabulary->machine_name)) {
    $context['success'] = FALSE;
    $context['results'][] = t('Could not acquire lock!');
    return;
  }

  $time = microtime(TRUE);
  set_time_limit(86400);

  db_delete('taxonomy_term_edge')
    ->condition('vocabulary', $vocabulary->machine_name)
    ->execute();

  $total_rows = 0;

  $result = db_query("INSERT INTO {taxonomy_term_edge} (vocabulary, tid, parent, distance)
    SELECT :vocabulary, h.tid, h.tid, 0
    FROM {taxonomy_term_hierarchy} h
    INNER JOIN {taxonomy_term_data} d ON d.tid = h.tid
    WHERE h.tid <> 0 AND h.tid <> h.parent AND d.vocabulary = :vocabulary",
    array(':vocabulary' => $vocabulary->machine_name));
  $total_rows += $result->rowCount();

  $context['message'] = t('Processed %rows rows at initial depth.', array('%rows' => $total_rows));

  while ($max_depth-- > 0) {
    $result = db_query("INSERT INTO {taxonomy_term_edge} (vocabulary, tid, parent, distance)
      SELECT e.vocabulary, e.tid, h.parent, e.distance + 1
      FROM {taxonomy_term_edge} e
      INNER JOIN {taxonomy_term_hierarchy} h ON h.tid = e.parent
      WHERE e.distance = :depth AND e.vocabulary = :vocabulary",
      array(':vocabulary' => $vocabulary->machine_name, ':depth' => $depth));

    $rows = $result->rowCount();
    if ($rows <= 0) {
      break;
    }

    $depth++;
    $total_rows += $rows;
    $context['message'] = t('Processed %rows rows - current depth: %depth', array('%rows' => $total_rows, '%depth' => $depth));
  }

  lock_release('taxonomy_edge_rebuild_edges_' . $vocabulary->machine_name);
  $context['success'] = TRUE;
  $context['finished'] = 1;
  $context['results'][] = t('%name rebuilt: %rows processed with depth %depth in %time seconds', array(
    '%name' => $vocabulary->name,
    '%rows' => $total_rows,
    '%depth' => $depth,
    '%time' => sprintf("%.03f", microtime(TRUE) - $time)
  ));

  return $total_rows;
}

/**
 * Rebuild the sorted tree.
 */
function taxonomy_edge_rebuild_order($vocabulary_name, &$context) {
  $vocabulary = taxonomy_vocabulary_load($vocabulary_name);
  if (!$vocabulary) {
    $context['success'] = FALSE;
    $context['message'] = t('Invalid vocabulary: %vocabulary', array('%vocabulary' => $vocabulary_name));
    return;
  }

  // Acquire lock.
  if (!lock_acquire('taxonomy_edge_rebuild_edges_' . $vocabulary->machine_name)) {
    $context['success'] = FALSE;
    $context['message'] = t('Could not acquire lock!');
  }

  $tx = db_transaction();

  $time = microtime(TRUE);
  taxonomy_edge_invalidate_order($vocabulary->machine_name);

  db_query("DELETE FROM {taxonomy_term_edge_order} WHERE vocabulary = :vocabulary", array(':vocabulary' => $vocabulary->machine_name));

  $total_rows = db_query("INSERT INTO {taxonomy_term_edge_order} (vocabulary, parent, eid)
    SELECT op.vocabulary, op.parent, op.eid
    FROM {taxonomy_term_edge} op
    WHERE op.vocabulary = :vocabulary
    ORDER BY " . _taxonomy_edge_generate_term_path_query('op.tid'),
    array(':vocabulary' => $vocabulary->machine_name))->rowCount();

  $context['success'] = TRUE;
  $context['finished'] = 1;
  $context['results'][] = t('%name sorted: %rows edges in %time seconds',
    array('%rows' => $total_rows, '%time' => sprintf("%.03f", microtime(TRUE) - $time), '%name' => $vocabulary->name)
  );

  db_insert('taxonomy_term_edge_order')
    ->fields(array('vocabulary' => $vocabulary->machine_name))
    ->execute();

  lock_release('taxonomy_edge_rebuild_edges_' . $vocabulary->machine_name);
  }




