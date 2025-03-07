<?php
/**
 * @file
 * Theme functions for Taxonomy Edge.
 */

/**
 * Overview vocabularies theme function.
 *
 * Copy of theme_taxonomy_overview_vocabularies, but with the addition of a
 * column, and an increase in colspan.
 */
function theme_taxonomy_edge_overview_vocabularies($variables) {
  $form = $variables['form'];
  $rows = array();
  foreach (element_children($form) as $key) {
    if (isset($form[$key]['name'])) {
      $vocabulary = &$form[$key];
      $row = array();
      $row[] = backdrop_render($vocabulary['name']);
      if (isset($vocabulary['weight'])) {
        $vocabulary['weight']['#attributes']['class'] = array(
          'vocabulary-weight',
        );
        $row[] = backdrop_render($vocabulary['weight']);
      }
      $row[] = backdrop_render($vocabulary['edit']);
      $row[] = backdrop_render($vocabulary['list']);
      $row[] = backdrop_render($vocabulary['add']);
      $row[] = backdrop_render($vocabulary['rebuild_edges']);
      $row[] = backdrop_render($vocabulary['rebuild_order']);
      $row[] = backdrop_render($vocabulary['rebuild_all']);
      $rows[] = array(
        'data' => $row,
        'class' => array(
          'draggable',
        ),
      );
    }
  }
  $header = array(
    t('Vocabulary name'),
  );
  if (isset($form['actions'])) {
    $header[] = t('Weight');
    backdrop_add_tabledrag('taxonomy', 'order', 'sibling', 'vocabulary-weight');
  }
  $header[] = array(
    'data' => t('Operations'),
    'colspan' => '3',
  );
  $header[] = array(
    'data' => t('Taxonomy Edge'),
    'colspan' => '3',
  );
  return theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'empty' => t('No vocabularies available. <a href="@link">Add vocabulary</a>.', array(
      '@link' => url('admin/structure/taxonomy/add'),
    )),
    'attributes' => array(
      'id' => 'taxonomy',
    ),
  )) . backdrop_render_children($form);
}
