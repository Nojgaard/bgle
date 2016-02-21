#ifndef GRAPH_IO
#define GRAPH_IO
#include <boost/graph/adjacency_list.hpp>
#include <stdexcept>
#include <iostream>
#include <map>
#include <cassert>
#include "pegtl.hh"
#include <boost/algorithm/string.hpp>

namespace GraphIO {
namespace Parser {
namespace GML {

template <typename Container>
struct InfoBundle {
	using Graph = typename Container::value_type;
	using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
	using Edge = typename boost::graph_traits<Graph>::edge_descriptor;
	InfoBundle(Container &c):c(c){}
	Container &c; 
	Vertex current_vertex,src,tar;
	bool bsrc,btar;
	Edge current_edge;
	std::map<int, Vertex> idx_map;
} /* optional variable list */;

std::pair<std::string,std::string> split_attr(std::string str) {
	std::vector<std::string> strs;
	std::string instr =str;
	boost::split(strs,instr,boost::is_any_of(" "));
	assert(strs.size() == 2);
	return std::pair<std::string,std::string>(strs[0],strs[1]);
}

namespace Grammar {
	using namespace pegtl;
	struct ign_tok : one<' ', '\n', '\t'> {};
	struct lb : pad<one<'['>, ign_tok> {};
	struct rb : pad<one<']'>, ign_tok> {};
	struct str : seq<one<'"'>,star<not_one<'"'>>,one<'"'>> {};
	struct dbl : seq<plus<digit>,one<'.'>,plus<digit>> {};
	struct graph_tok : string<'g','r','a','p','h'> {};

	struct attr_int : seq<identifier, one<' '>, plus<digit>> {};
	struct attr_str : seq<identifier, one<' '>, str> {};
	struct attr_dbl : seq<identifier, one<' '>, dbl> {};

	/* Node grammar */
	struct node_attr_int : attr_int {};
	struct node_attr_str : attr_str {};
	struct node_attr_dbl : attr_dbl {};
	struct node_attr : sor<node_attr_dbl,node_attr_int,node_attr_str> {};
	struct node_attr_list :star<pad<node_attr, ign_tok>> {};

	struct node_tok : string<'n','o','d','e'> {};
	struct node : seq<node_tok, lb, node_attr_list, rb> {};
	struct node_list : star<node> {};

	/* Edge grammar */
	struct edge_tok : string<'e','d','g','e'> {};
	struct edge_attr_int : attr_int {};
	struct edge_attr_str : attr_str {};
	struct edge_attr_dbl : attr_dbl {};
	struct edge_attr : sor<edge_attr_dbl,edge_attr_int,edge_attr_str> {};
	struct edge_attr_list :star<pad<edge_attr, ign_tok>> {};
	struct edge: seq<edge_tok, lb, edge_attr_list, rb> {};
	struct edge_list : star<edge> {};

	/* Graph grammar */
	struct graph_attr_int : attr_int {};
	struct graph_attr_str : attr_str {};
	struct graph_attr_dbl : attr_dbl {};
	struct graph_attr : sor<graph_attr_dbl,graph_attr_int,graph_attr_str> {};
	struct graph_attr_list : star<pad<graph_attr,ign_tok>> {};
	struct graph :must<graph_tok,lb,graph_attr_list,node_list,edge_list,rb> {};
	struct grammar : must<graph> {};
	
	/* Actions */
	//Class template for user-defined actions that does
	// nothing by default.
	template< typename Rule >
		struct action
		: pegtl::nothing< Rule > {};

	template<> struct action<graph_tok> {
		template<typename Bundle>
		static void apply(const input &in, Bundle &ib) {
			ib.c.insert(ib.c.end(), typename Bundle::Graph());
		}
	};

	template<> struct action<node_tok> {
		template<typename Bundle>
			static void apply(const input &in, Bundle &ib) {
				assert(ib.c.size() > 0);
				ib.current_vertex = add_vertex(ib.c.back());
			}
	};

	template<> struct action<node> {
		template<typename Bundle>
			static void apply(const input &in, Bundle &ib) {
			}
	};

	template<> struct action<node_attr_int> {
		template<typename Bundle>
		static void apply(const input &in, Bundle &ib) {
			auto p = split_attr(in.string());
			int val = stoi(p.second);
			std::string key = p.first;
			if (key == "id") {
				if (ib.idx_map.find(val) != ib.idx_map.end()) {
					throw pegtl::parse_error("Node id "+std::to_string(val)+" already exists", in );
				}
				ib.idx_map[val] = ib.current_vertex;
			}
			ib.c.back()[ib.current_vertex](key,val);
		}
	};

	template<> struct action<node_attr_str> {
		template<typename Bundle>
		static void apply(const input &in, Bundle &ib) {
			auto p = split_attr(in.string());
			std::string key = p.first;
			std::string val = p.second.substr(1,p.second.size()-2);
			ib.c.back()[ib.current_vertex](key,val);
		}
	};

	template<> struct action<node_attr_dbl> {
		template<typename Bundle>
		static void apply(const input &in, Bundle &ib) {
			auto p = split_attr(in.string());
			std::string key = p.first;
			double val = std::stod(p.second);
			ib.c.back()[ib.current_vertex](key,val);
		}
	};

	template<> struct action<edge_attr_int> {
		template<typename Bundle>
		static void apply(const input &in, Bundle &ib) {
			auto p = split_attr(in.string());
			std::string key = p.first;
			int val = stoi(p.second);
			if (key == "source") {
				if (ib.idx_map.find(val) == ib.idx_map.end()) {
					throw parse_error("Source does not exist", in);
				}
				ib.bsrc = true;
				ib.src = ib.idx_map[val];
			} else if (key == "target") {
				if (ib.idx_map.find(val) == ib.idx_map.end()) {
					throw parse_error("Target does not exist", in);
				}
				ib.btar = true;
				ib.tar = ib.idx_map[val];
			}
			ib.c.back()[ib.current_edge](key,val);
		}
	};

	template<> struct action<edge_attr_dbl> {
		template<typename Bundle>
		static void apply(const input &in, Bundle &ib) {
			auto p = split_attr(in.string());
			std::string key = p.first;
			double val = stod(p.second);
			ib.c.back()[ib.current_edge](key,val);
		}
	};

	template<> struct action<edge_attr_str> {
		template<typename Bundle>
		static void apply(const input &in, Bundle &ib) {
			auto p = split_attr(in.string());
			std::string key = p.first;
			std::string val = p.second.substr(1,p.second.size()-2);
			ib.c.back()[ib.current_edge](key,val);
		}
	};


	template<> struct action<edge> {
		template<typename Bundle>
		static void apply(const input &in, Bundle &ib) {
			if (!ib.bsrc || !ib.btar) {
				throw parse_error("Need to declare a source and target.",in);
			}
			add_edge(ib.src,ib.tar,ib.c.back());
		}
	};

	template<> struct action<graph_attr_int> {
		template<typename Bundle>
		static void apply(const input &in, Bundle &ib) {
			auto p = split_attr(in.string());
			std::string key = p.first;
			int val = stoi(p.second);
			ib.c.back()[boost::graph_bundle](key,val);
		}
	};
	template<> struct action<graph_attr_dbl> {
		template<typename Bundle>
		static void apply(const input &in, Bundle &ib) {
			auto p = split_attr(in.string());
			std::string key = p.first;
			double val = stod(p.second);
			ib.c.back()[boost::graph_bundle](key,val);
		}
	};
	template<> struct action<graph_attr_str> {
		template<typename Bundle>
		static void apply(const input &in, Bundle &ib) {
			auto p = split_attr(in.string());
			std::string key = p.first;
			std::string val = p.second.substr(1,p.second.size()-2);
			ib.c.back()[boost::graph_bundle](key,val);
		}
	};

} /* namespace Grammar */


template <typename Container>
bool read(std::string path, Container &c) {
	using Graph = typename Container::value_type;
	InfoBundle<Container> ib(c);
	pegtl::read_parser p(path);
	try {
		p.parse<Grammar::grammar,Grammar::action>(ib);
	} catch (const pegtl::parse_error &e) {
		std::cout << e.what() << std::endl;
	}
}

} /* namespace GML */
} /* namespace Parser */
} /* namespace GraphIO */
#endif /* ifndef GRAPH_IO */
