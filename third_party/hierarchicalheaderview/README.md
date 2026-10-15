# HierarchicalHeaderView

Multi-level (tree-structured) header for `QTableView`, by Krasnoshchekov Petr (2009),
distributed under the BSD license in [LICENSE](LICENSE).

Local modifications:

- ported to Qt 5.15 / Qt 6 (no deprecated `QMatrix`, `QMouseEvent::x()`, `QHoverEvent::pos()`);
- fixed a memory leak (the private implementation was never deleted);
- switched to compile-time checked `connect()` syntax.
