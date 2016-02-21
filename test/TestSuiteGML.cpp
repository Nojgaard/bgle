#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <Catch/single_include/catch.hpp>
#include <GraphIO/Parser/GML.h>
#include <boost/graph/adjacency_list.hpp>
#include <iostream>

struct empty_bundle {
	template<typename T>
	void operator() (std::string key, T val){}
};

struct fbundle {
	int id;
	std::string label;
	double v;
	void operator () (std::string key, int val) {
		if (strcmp(key.c_str(), "id") == 0) {
			id = val;
		}
	}
	void operator () (std::string key, std::string val) {
		if (strcmp(key.c_str(), "label") == 0) {
			label = val;
		}
	}
	void operator () (std::string key, double val) {
		if (strcmp(key.c_str(), "v") == 0) {
			v= val;
		}
	}
};

TEST_CASE( "Test empty graphs.", "[GML]" ) {
	using Graph = typename boost::adjacency_list<boost::vecS, boost::vecS, 
			boost::undirectedS, empty_bundle, empty_bundle, empty_bundle>;

	std::vector<Graph> graphs;
	std::string path = "data/test/GML/three_empty_graphs.gml";
	REQUIRE(GraphIO::Parser::GML::read(path,graphs));
	REQUIRE(graphs.size() == 3);
}

TEST_CASE( "Testing multi value graphs.", "[GML]" ) {
	using Graph = typename boost::adjacency_list<boost::vecS, boost::vecS, 
			boost::undirectedS, fbundle, fbundle, fbundle>;
	std::vector<Graph> graphs;
	std::string path = "data/test/GML/multi_graphs.gml";
	REQUIRE(GraphIO::Parser::GML::read(path,graphs));
	REQUIRE(graphs.size() == 2);
	REQUIRE(num_vertices(graphs[0]) == 2); REQUIRE(num_vertices(graphs[1]) == 2);
	REQUIRE(num_edges(graphs[0]) == 1);
	REQUIRE(num_edges(graphs[1]) == 1);
	REQUIRE(graphs[0][boost::graph_bundle].label == "g1");
	REQUIRE(graphs[0][boost::graph_bundle].id == 0);
	REQUIRE(graphs[0][boost::graph_bundle].v == 42.42);

	REQUIRE(graphs[0][0].label == "v1");
	REQUIRE(graphs[0][0].id== 1);
	REQUIRE(graphs[0][0].v== 2.2);
	REQUIRE(graphs[0][1].label == "v2");
	REQUIRE(graphs[0][1].id== 2);
	REQUIRE(graphs[0][1].v== 3.2);
	auto pe = boost::edge(0,1,graphs[0]);
	REQUIRE(pe.second);
	auto e = pe.first;
	REQUIRE(graphs[0][e].label == "e1");
	REQUIRE(graphs[0][e].id== 3);
	REQUIRE(graphs[0][e].v== 4.2);

	REQUIRE(graphs[1][boost::graph_bundle].label == "g2");
	REQUIRE(graphs[1][boost::graph_bundle].id == 1);
	REQUIRE(graphs[1][boost::graph_bundle].v == 24.24);
	REQUIRE(graphs[1][0].label == "v3");
	REQUIRE(graphs[1][0].id== 4);
	REQUIRE(graphs[1][0].v== 2.3);
	REQUIRE(graphs[1][1].label == "v4");
	REQUIRE(graphs[1][1].id== 5);
	REQUIRE(graphs[1][1].v== 3.4);
	pe = boost::edge(0,1,graphs[1]);
	REQUIRE(pe.second);
	e = pe.first;
	REQUIRE(graphs[1][e].label == "e5");
	REQUIRE(graphs[1][e].id== 5);
	REQUIRE(graphs[1][e].v== 4.5);

}
