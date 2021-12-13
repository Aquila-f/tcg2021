/**
 * Framework for 2048 & 2048-like Games (C++ 11)
 * agent.h: Define the behavior of variants of agents including players and environments
 *
 * Author: Theory of Computer Games (TCG 2021)
 *         Computer Games and Intelligence (CGI) Lab, NYCU, Taiwan
 *         https://cgilab.nctu.edu.tw/
 */

#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include "board.h"
#include "action.h"
#include <fstream>

class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss("name=unknown role=unknown " + args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			meta[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string property(const std::string& key) const { return meta.at(key); }
	virtual void notify(const std::string& msg) { meta[msg.substr(0, msg.find('='))] = { msg.substr(msg.find('=') + 1) }; }
	virtual std::string name() const { return property("name"); }
	virtual std::string role() const { return property("role"); }

protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> meta;
};

/**
 * base agent for agents with randomness
 */
class random_agent : public agent {
public:
	random_agent(const std::string& args = "") : agent(args) {
		if (meta.find("seed") != meta.end())
			engine.seed(int(meta["seed"]));
	}
	virtual ~random_agent() {}

protected:
	std::default_random_engine engine;
};

/**
 * random environment
 * add a new random tile to an empty cell
 * 2-tile: 90%
 * 4-tile: 10%
 */
class rndenv : public random_agent {
public:
	rndenv(const std::string& args = "") : random_agent("name=random role=environment " + args),
		space({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }), popup(0, 9) {}

	virtual action take_action(const board& after) {
		std::shuffle(space.begin(), space.end(), engine);
		for (int pos : space) {
			if (after(pos) != 0) continue;
			board::cell tile = popup(engine) ? 1 : 2;
			return action::place(pos, tile);
		}
		return action();
	}

private:
	std::array<int, 16> space;
	std::uniform_int_distribution<int> popup;
};

/**
 * dummy player
 * select a legal action randomly
 */
class player : public random_agent {
public:
	player(const std::string& args = "") : random_agent("name=dummy role=player " + args),
		opcode({ 0, 1, 2, 3 }) {arg = args;}
		
	std::tuple<unsigned int , int> get_maxtile_info(const board inp){
		unsigned int maxtileplace=0;
		unsigned int maxtile = 0;
			board::grid t = inp.infot();
			for(int r=0;r<4;r++){
				for(int c=0;c<4;c++){
					if (maxtile < t[r][c]){maxtileplace = r*4+c; maxtile = t[r][c];} 
				}
			}
		return std::make_tuple(maxtileplace, maxtile);
	}

	virtual action take_action(const board& before) {
		if (arg == "greedy"){
			int greedymove = 0,maxreward = 0;
			for (int op : opcode) {
				board::reward reward = board(before).slide(op);
				if(maxreward <= reward){greedymove = op, maxreward = reward;}
				}
			return action::slide(greedymove);
		}
		else if (arg == "heuristics2step"){
			int greedymove = 0,maxreward = 0;
			for (int op1 : opcode) {
				board temp = board(before);
				board::reward reward = temp.slide(op1);
				if (reward == -1) continue;
				for (int op2 : opcode){
					board::reward totalreward = reward + temp.slide(op2);
					if(maxreward <= totalreward){greedymove = op1, maxreward = totalreward;}
				}
				}
			return action::slide(greedymove);
		}
		else if (arg == "heuristic"){

			auto maxinfo = get_maxtile_info(before);
			unsigned int maxtile_pos_before = std::get<0>(maxinfo);
			int maxtile_val = std::get<1>(maxinfo);
			int greedymove = 0,maxreward = 0;


			for (int op1 : opcode) {
				board temp = board(before);
				board::reward reward = temp.slide(op1);
				if (reward == -1) continue;
				auto maxinfo2 = get_maxtile_info(temp);
				unsigned int maxtile_pos_before2 = std::get<0>(maxinfo2);
				int maxtile_val2 = std::get<1>(maxinfo2);
				if(maxtile_pos_before2 == maxtile_pos_before && maxtile_val > 12) reward += board::fibonacci(maxtile_val - 2);

				
				for (int op2 : opcode){
					board temp1 = board(temp);
					board::reward reward1 = temp1.slide(op2);
					board::reward totalreward = reward + reward1;
					if(std::get<0>(get_maxtile_info(temp1)) == maxtile_pos_before2 && maxtile_val2 > 6) reward += board::fibonacci(maxtile_val2 - 2);
					if(maxreward <= totalreward){greedymove = op1, maxreward = totalreward;}}
				}
			
			return action::slide(greedymove);
			
		}
		else {
			std::shuffle(opcode.begin(), opcode.end(), engine);
			for (int op : opcode) {
				
				board::reward reward = board(before).slide(op) ;

				if (reward != -1) return action::slide(op);}
		}
		// std::cout << before << std::endl;
		// std::cout << s++ << std::endl;
		return action();
	}

private:
	std::array<int, 4> opcode;
	std::string arg;
	int s = 0;
};
