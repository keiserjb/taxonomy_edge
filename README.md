# Taxonomy Edge

Selecting all children of a given taxonomy term can be a pain.
This module makes it easier to do this, by maintaining a complete list of edges
for each term using the adjecency matrix graph theory:
  http://en.wikipedia.org/wiki/Adjacency_matrix

## Installation

- Install this module using the [official Backdrop CMS instructions](https://backdropcms.org/guide/modules).

## Maintenance

Rebuild edges from /admin/content/taxonomy/edge

Edges can also be rebuild through cron if a hook_cronapi() compatible cron module is installed (Elysia Cron, Ultimate Cron, etc.)

Rebuilding of edges can be necessary if the table gets out of sync.

## Issues

Bugs and Feature requests should be reported in the [Issue Queue](https://github.com/backdrop-contrib/shs/issues)

## Current Maintainers

- [Justin Keiser](https://github.com/keiserjb)
- Co-maintainers desired

## Credits

- Port to Backdrop by [Justin Keiser](https://github.com/keiserjb)
- Maintained and created for Drupal by [Thomas Gielfeldt](https://www.drupal.org/u/gielfeldt)

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.

## SQL

The following select statements can be used as help or inspiration to use the taxonomy_term_edge table:

Get the top level term IDs for the term 14:

SELECT DISTINCT e2.parent
FROM taxonomy_term_edge e
JOIN taxonomy_term_edge e2 ON e2.tid = e.tid AND e2.distance = e.distance - 1 AND e2.parent <> e.parent
WHERE e.tid = 14
AND e.parent = 0
AND e.vocabulary = e2.vocabulary

Generate a list of materialized paths for each term in vocabulary vocabulary:1, in the correct order:

SELECT d2.*, e2.parent, e2.distance,
(
SELECT CONCAT('"', GROUP_CONCAT(d.name ORDER BY e.distance DESC SEPARATOR '/'), '"') AS path FROM taxonomy_term_edge e JOIN taxonomy_term_data d ON e.parent = d.tid WHERE e.tid = e2.tid ORDER BY e.distance DESC
) AS path,
(
SELECT GROUP_CONCAT(d.weight + 1500, '    ', d.name ORDER BY e.distance DESC SEPARATOR '    ') AS path FROM taxonomy_term_edge e JOIN taxonomy_term_data d ON e.parent = d.tid WHERE e.tid = e2.tid ORDER BY e.distance DESC
) AS sort_path
FROM taxonomy_term_edge e2
JOIN taxonomy_term_data d ON d.tid = e2.tid
WHERE e2.vocabulary = 1
AND e2.parent = 0
ORDER BY sort_path;

Get all parents of a term tid:12

SELECT * FROM taxonomy_term_edge WHERE tid = 12 AND distance > 0;

Get all children of a term tid:12

SELECT * FROM taxonomy_term_edge WHERE parent = 12;

## Misc.

Other popular alternatives to the adjecency matrix model are the nested set and materialized path.

Nested set (http://en.wikipedia.org/wiki/Nested_set_model):
 - Advantages
   - Fast retrieval of sorted tree.
 - Disadvantages:
   - Complex.
   - Can be heavy on insert/update (can be mitigated with gaps, but increases complexity).

Materialized path (http://www.dba-oracle.com/t_sql_patterns_trees.htm):
 - Advantages
   - Simple to understand.
 - Disadvantages:
   - Can be heavy on select/insert/update due to string compare.
   - Requires multiple queries for proper use of indexes on subtree selection.
   - Depth limit (due to index size limitation).

