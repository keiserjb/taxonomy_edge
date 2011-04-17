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
  function query() {
    // If no filter values are present, then do nothing.
    if (count($this->value) == 0) {
      return;
    } 
    else if (count($this->value) == 1) {
      $placeholder = " = %d";
    } 
    else {
      $placeholder = " IN (" . implode(', ', array_fill(0, sizeof($this->value), '%d')) . ")";
    }

    // The normal use of ensure_my_table() here breaks Views.
    // So instead we trick the filter into using the alias of the base table.
    // See http://drupal.org/node/271833
    // If a relationship is set, we must use the alias it provides.
    if (!empty($this->relationship)) {
      $this->table_alias = $this->relationship;
    }
    // If no relationship, then use the alias of the base table.
    else if (isset($this->query->table_queue[$this->query->base_table]['alias'])) {
      $this->table_alias = $this->query->table_queue[$this->query->base_table]['alias'];
    }
    // This should never happen, but if it does, we fail quietly.
    else {
      return;
    }

    // Now build the subqueries.
    $tids = $this->value;
    $subquery = "\n  SELECT tn.vid FROM {term_node} tn\n";
    $where = "  WHERE \n";

    if ($this->options['depth'] == 'all') {
      $subquery .= "    INNER JOIN {term_edge} te ON te.tid = tn.tid\n";
      $where .= "    te.parent $placeholder\n";
      $args = $tids;
    }
    elseif ($this->options['depth'] > 0) {
      $subquery .= "    INNER JOIN {term_edge} te ON te.tid = tn.tid\n";
      $where .= "    te.parent $placeholder\n";
      $where .= "    AND te.distance <= %d\n";
      $args = $tids;
      $args[] = $this->options['depth'];
    }
    else if ($this->options['depth'] < 0) {
      $subquery .= "    INNER JOIN {term_edge} te ON te.tid = tn.tid\n";
      $where .= "    te.parent = (SELECT parent FROM {term_edge} WHERE tid $placeholder AND distance = %d)\n";
      $where .= "    AND te.distance <= %d\n";
      $args = $tids;
      $args[] = abs($this->options['depth']);
      $args[] = abs($this->options['depth']);
    }
    else {
      $where .= "    tn.tid $placeholder\n";
      $args = $tids;
    }

    $this->query->add_where(0, "$this->table_alias.$this->real_field IN ($subquery$where  )", $args);
  }
}
