#include <GraphIO/Parser/GML.h>
#include <boost/graph/adjacency_list.hpp>

int main(int argc, const char *argv[]) {
	using Graph = typename boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
	std::vector<Graph> graphs;
	boost::GraphIO::GML::read(graphs);
	return 0;
}
