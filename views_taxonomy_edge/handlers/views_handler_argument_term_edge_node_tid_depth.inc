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
  function query() {
    $this->ensure_my_table();

    if (!empty($this->options['break_phrase'])) {
      $tids = new stdClass();
      $tids->value = $this->argument;
      $tids = views_break_phrase($this->argument, $tids);
      if ($tids->value == -1) {
        return FALSE;
      }

      if (count($tids->value) > 1) {
        $placeholder = " IN (" . implode(', ', array_fill(0, sizeof($tids->value), '%d')) . ")";
      }
      else {
        $placeholder = " = %d";
      }

      $tids = $tids->value;
    }
    else {
      $placeholder = "= %d";
      $tids = array($this->argument);
    }
    
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
