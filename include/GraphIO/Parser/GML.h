#ifndef GRAPH_IO
#define GRAPH_IO
#include <boost/graph/adjacency_list.hpp>
#include <stdexcept>
#include <map>

namespace boost {
namespace GraphIO {
namespace GML {

template <typename Container, typename Graph>
struct InfoBundle {
	using Vertex = typename graph_traits<Graph>::vertex_descriptor;
	using Edge = typename graph_traits<Graph>::edge_descriptor;
	InfoBundle(Container &c):c(c) {}
	Container &c; 
	std::map<int, Vertex> idx_map;
} /* optional variable list */;

template <typename Container>
void read(Container &c) {
	using Graph = typename Container::value_type;
	InfoBundle<Container,Graph> ib(c);
}

} /* namespace GML */
} /* namespace GraphIO */
} /* namespace boost */
#endif /* ifndef GRAPH_IO */
