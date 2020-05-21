# eulerian


## Description

Given an input file of specific format describing a graph,
this code computes a *Eulerian trail* (*Eulerian path*) through
that graph if and only if one exists. It is able to detect if there
cannot be such a trail prior to computation to increase the
responsiveness of the program.

#### Input format
The first line contains the number of nodes in the graph.
All following lines contain an edge description of the form
"`a b`". `a` and `b` are *labels* of nodes where there is
an edge from `a` to `b`. Node labels are typically numbers,
separated by *exactly one* space ('` `') symbol and end with a
*exactly one* newline ('`\n`') symbol.

#### Output format
If there is a *Eulerian trail* in the specified graph, a sequence
of node labels separated by exactly one space ('` `') is printed
to `STDOUT` (*standard output stream*, UNIX file descriptor `1`)
followed by one newline ('`\n`') symbol. If there is no trail
in the graph a `-1` followed by one newline ('`\n`') is printed
to `STDOUT`.
Warnings and errors are printed to `STDERR` (*standard error stream*,
UNIX file descriptor `2`) occasionally *in addition* to the `-1`.


If the node number in the first line of the input file is greater
then the total number of different node labels in the following lines
it is assumed that there are nodes of *degree 0* (not connected to any
other nodes). In this case there is no *Eulerian trail*, at all, and
a warning is printed to `STDERR`.

If a label occurs twice in one line in the input file that edge is
a *loop* at the node denoted with that label. The output will likely
contain the respective label twice consecutively.


## Reliability and performance note

This code passed several semi-automated tests including tests
for memory leaks. It is able to handle large input files with
hundreds of nodes and thousands of edges efficiently.


## References

* German (original)
    - [Eulerkreisproblem/ Algorithmus von Hierholzer](https://de.wikipedia.org/wiki/Eulerkreisproblem)
    - [Graph-Zusammenhang](https://de.wikipedia.org/wiki/Zusammenhang_%28Graphentheorie%29)
    - *Tiefensuche*:
          [Wikipedia](https://de.wikipedia.org/wiki/Tiefensuche),
          [Humboldt-Universität zu Berlin](http://www2.informatik.hu-berlin.de/~kschmidt/Tiefensuche.pdf)
    - [Algorithmnusbeschreibung](http://www.zahlendoktor.de/eulerweg_finden.html)

* English (adopted)
    - [Eulerian path/ Hierholzer's algorithm](https://en.wikipedia.org/wiki/Eulerian_path)
    - [Connectivity (Graph theory)](https://en.wikipedia.org/wiki/Connectivity_%28graph_theory%29)
    - [Depth-First Search (DFS)](https://en.wikipedia.org/wiki/Depth-first_search)


## Examples

An example of a correctly formatted input file representing a graph
("`1.graph`") and the respective output representing the
*Eulerian trail* through that graph ("`1.trail`") can be found in
the `sample` folder.


## Copyright notice

Copyright &copy; 2016, 2019 Daniel Haase

`eulerian` is licensed under the **GNU General Public License, version 3**.

Initial release: **Aug 2016**


## License disclaimer

```
eulerian - compute a Eulerian trail through a graph iff one exists
Copyright (C) 2016, 2019  Daniel Haase

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.
If not, see &lt;[https://www.gnu.org/licenses/gpl-3.0.txt](https://www.gnu.org/licenses/gpl-3.0.txt)&gt;.
```
