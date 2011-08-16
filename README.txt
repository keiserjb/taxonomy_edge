Description
-----------

Selecting all children of a given taxonomy term can be a pain.
This module makes it easier to do this, by maintaining a complete list of edges
for each term using the adjecency matrix graph theory:
  http://en.wikipedia.org/wiki/Adjacency_matrix

Visit the project page:
  http://drupal.org/sandbox/gielfeldt/1090284


Requirements
------------
Currently only MySQL is supported.
Elysia Cron or Parallel Cron (or other hook_cronapi() compatible cron system) for cronjob to work.


Installation
------------
Place in modules folder and enable it from /admin/build/modules


Maintenance
-----------
Rebuild edges from /admin/content/taxonomy/edge

Edges can also be rebuild through cron if a hook_cronapi() compatible cron module is installed (Elysia Cron, Parallel Cron, etc.)

Rebuilding of edges can be necessary if the table gets out of sync.


SQL
------

The following select statements can be used as help or inspiration to use the term_edge table:

Get the top level term IDs for each term in vocabulary vid:1:

SELECT te.tid, h.tid AS top_tid
FROM term_edge te
JOIN term_hierarchy h ON te.parent = h.tid
JOIN term_data d ON d.tid = te.tid
WHERE h.parent = 0
AND d.vid = 1;


Generate a list of materialized paths for each term in vocabulary vid:1, in the correct order:

SELECT d2.*, e2.parent, e2.distance,
(
SELECT CONCAT('"', GROUP_CONCAT(d.name ORDER BY e.distance DESC SEPARATOR '/'), '"') AS path FROM term_edge e JOIN term_data d ON e.parent = d.tid WHERE e.tid = e2.tid ORDER BY e.distance DESC
) AS path,
(
SELECT GROUP_CONCAT(d.weight + 1500, '    ', d.name ORDER BY e.distance DESC SEPARATOR '    ') AS path FROM term_edge e JOIN term_data d ON e.parent = d.tid WHERE e.tid = e2.tid ORDER BY e.distance DESC
) AS sort_path
FROM term_edge e2
JOIN term_hierarchy h ON h.tid = e2.parent
JOIN term_data d2 ON e2.tid = d2.tid
WHERE d2.vid = 1
AND h.parent = 0
ORDER BY sort_path;


Get all parents of a term tid:12

SELECT * FROM term_edge WHERE tid = 12 AND distance > 0;


Get all children of a term tid:12

SELECT * FROM term_edge WHERE parent = 12;



Misc.
-----
Other popular alternatives to the adjecency matrix model are the nested set and materialized path.

Nested set (http://en.wikipedia.org/wiki/Nested_set_model):
 - Advantages
   - Fast retrieval of sorted tree.
 - Disadvantages:
   - Complex.
   - Can be heavy on insert/update.

Materialized path (http://www.dba-oracle.com/t_sql_patterns_trees.htm):
 - Advantages
   - Simple to understand.
 - Disadvantages:
   - Can be heavy on select/insert/update due to string compare.
