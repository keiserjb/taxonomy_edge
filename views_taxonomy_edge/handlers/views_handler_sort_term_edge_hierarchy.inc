<?php

/**
 * Views sort handler to sort by taxonomy terms' hierarchy order.
 */
class views_handler_sort_term_edge_hierarchy extends views_handler_sort {
  /**
   * Overridden to add the ORDER BY clause and join required tables. 
   */
  function query() {
    $edge = $this->query->ensure_table('taxonomy_term_edge');

    if (taxonomy_edge_is_order_invalid()) {
      $sort_path = _taxonomy_edge_generate_term_path_query("$edge.tid");
      $alias = $this->query->add_field(NULL, $sort_path, 'sort_path');
      $this->query->add_orderby(NULL, NULL, $this->options['order'], 'sort_path');
    }
    else {
      $alias = $this->query->ensure_table('taxonomy_term_edge_order');
      $this->query->add_orderby($alias, 'oid', $this->options['order']);
    }
  }
}
