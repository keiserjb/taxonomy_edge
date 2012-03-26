<?php
/**
 * @file
 *
 * This file is a copy/override of the default views taxonomy argument handler.
 *
 * Argument handler for taxonomy terms with depth using Taxonomy Edge.
 *
 * @see views_handler_argument_term_node_tid_depth.inc
 */
class views_handler_argument_term_edge_node_tid_depth extends views_handler_argument_term_node_tid_depth {
  function query($group_by = FALSE) {
    $this->ensure_my_table();

    if (!empty($this->options['break_phrase'])) {
      $tids = new stdClass();
      $tids->value = $this->argument;
      $tids = views_break_phrase($this->argument, $tids);
      if ($tids->value == -1) {
        return FALSE;
      }

      if (count($tids->value) > 1) {
        $operator = 'IN';
      }
      else {
        $operator = '=';
      }

      $tids = $tids->value;
    }
    else {
      $operator = "=";
      $tids = $this->argument;
    }
    // Now build the subqueries.
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

    $this->query->add_where(0, "$this->table_alias.$this->real_field", $subquery, 'IN');
  }

}
