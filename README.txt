Description
-----------

Selecting all children of a given taxonomy term can be a pain.
This module makes it easier by implementing an "id path" for each term.

For a full description of the module, visit the project page:
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

