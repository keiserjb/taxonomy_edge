<?php
/**
 * @file
 *
 * This file is a copy/override of the default views taxonomy filter handler.
 *
 * Filter handler for taxonomy terms with depth using Taxonomy Edge.
 *
 * @see views_handler_filter_term_node_tid_depth_modifier.inc
 */
class views_handler_filter_term_edge_node_tid_depth extends views_handler_filter_term_node_tid_depth {
  function query($group_by = FALSE) {
    // If no filter values are present, then do nothing.
    if (count($this->value) == 0) {
      return;
    }
    elseif (count($this->value) == 1) {
      // Somethis $this->value is an array with a single element so convert it.
      if (is_array($this->value)) {
        $this->value = current($this->value);
      }
      $operator = '=';
    }
    else {
      $operator = 'IN';# " IN (" . implode(', ', array_fill(0, sizeof($this->value), '%d')) . ")";
    }

    // The normal use of ensure_my_table() here breaks Views.
    // So instead we trick the filter into using the alias of the base table.
    // See http://drupal.org/node/271833
    // If a relationship is set, we must use the alias it provides.
    if (!empty($this->relationship)) {
      $this->table_alias = $this->relationship;
    }
    // If no relationship, then use the alias of the base table.
    elseif (isset($this->query->table_queue[$this->query->base_table]['alias'])) {
      $this->table_alias = $this->query->table_queue[$this->query->base_table]['alias'];
    }
    // This should never happen, but if it does, we fail quietly.
    else {
      return;
    }

    // Now build the subqueries.
    $tids = $this->value;
    $subquery = db_select('taxonomy_index', 'tn');
    $subquery->addField('tn', 'nid');

    if (!$this->options['depth'] && $this->options['depth'] != 'all') {
      $subquery->condition('tn.tid', $tids, $operator);
    }
    else {
      $subquery->innerJoin('taxonomy_term_edge', 'te', 'te.tid = tn.tid');
      if ($this->options['depth'] == 'all') {
        $subquery->condition('te.parent', $tids, $operator);
      }
      elseif ($this->options['depth'] > 0) {
        $subquery->condition('te.parent', $tids, $operator);
        $subquery->condition('te.distance', $this->options['depth'], '<');
      }
      elseif ($this->options['depth'] < 0) {
        $subquery2 = db_select('taxonomy_term_edge', 'te2')
          ->fields('te2', array('parent'))
          ->condition('te2.tid', $tids, $operator)
          ->condition('te2.distance', abs($this->options['depth']), '<=');
        $subquery->condition('te.parent', $subquery2, 'IN');
        $subquery->condition('te.distance', abs($this->options['depth']), '<=');
      }
    }
  }
}
