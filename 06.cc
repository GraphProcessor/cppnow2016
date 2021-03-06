#include <iostream>
#include <vector>

#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>

using namespace boost;


// Use-case: bidirectional Dijkstra st-shortest path.
// In the following we construct a graph and run a Dijkstra search starting at the source on it.
// We also construct a reverse graph and run a Dijkstra search starting at the target on it.
// The question is how to stop in the "middle" where both searches meet (assuming there exists a path).
// Follow the execution and have a look where the "*" is getting printed.
// One approach is to use threads, synchronization and throwing exceptions from the visitors.

struct edge_data_t { int distance = 0; };

using graph_t = compressed_sparse_row_graph<bidirectionalS, no_property, edge_data_t>;
using vertex_t = graph_traits<graph_t>::vertex_descriptor;

struct dijkstra_stepwise : default_dijkstra_visitor {
  dijkstra_stepwise(vertex_t middle_) : middle(middle_) {}

  template <typename graph_t>
  void examine_vertex(const vertex_t vertex, const graph_t&) const {
    std::cout << vertex << std::endl;
  }

  vertex_t middle;
};

int main() {
  /*
   * 0 <-> 1 <-> 2
   * ^     ^     ^
   * |     |     |
   * v     v     v
   * 5 <-> 4 <-> 3
   */
  auto n_vertices = 6u;

  std::vector<std::pair<vertex_t, vertex_t>> edges{{0, 1}, {1, 2}, {1, 4}, {2, 3}, {3, 4}, {4, 5}, {5, 0}};
  std::vector<edge_data_t> distances{{1}, {1}, {2}, {1}, {1}, {2}, {2}};

  auto tag = edges_are_unsorted_multi_pass;
  graph_t graph{tag, edges.begin(), edges.end(), distances.begin(), n_vertices};

  vertex_t source{0};
  vertex_t target{3};

  auto reversed_graph = make_reverse_graph(graph);

  std::vector<vertex_t> forward_prev(num_vertices(graph));
  std::vector<vertex_t> backward_prev(num_vertices(graph));

  auto forward_prev_map = make_iterator_property_map(forward_prev.begin(), get(vertex_index, graph));
  auto backward_prev_map = make_iterator_property_map(backward_prev.begin(), get(vertex_index, reversed_graph));

  vertex_t middle{target};

  dijkstra_shortest_paths_no_color_map(graph, source,
      weight_map(get(&edge_data_t::distance, graph))
      .predecessor_map(forward_prev_map)
      .visitor(dijkstra_stepwise{middle}));

  std::cout << "*" << std::endl;

  dijkstra_shortest_paths_no_color_map(reversed_graph, target,
      weight_map(get(&edge_data_t::distance, reversed_graph))
      .predecessor_map(backward_prev_map)
      .visitor(dijkstra_stepwise{middle}));
}
