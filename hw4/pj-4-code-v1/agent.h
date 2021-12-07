/**
 * Framework for NoGo and similar games (C++ 11)
 * agent.h: Define the behavior of variants of the player
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

		for(auto k : meta){
			std::cout << k.first << " | " << k.second.value << "\n";
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
 * random player for both side
 * put a legal piece randomly
 */
class player : public random_agent {
public:
	player(const std::string& args = "") : random_agent("name=random role=unknown " + args),
		space(board::size_x * board::size_y), who(board::empty) {
			
		
		if (name().find_first_of("[]():; ") != std::string::npos)
			throw std::invalid_argument("invalid name: " + name());
		if (role() == "black") who = board::black;
		if (role() == "white") who = board::white;
		if (who == board::empty)
			throw std::invalid_argument("invalid role: " + role());
		for (size_t i = 0; i < space.size(); i++)
			space[i] = action::place(i, who);
		// for(auto k : space){
		// 	std::cout << k << "\n";
		// }
	}

	virtual void open_episode(const std::string& flag = "") {

	}
	virtual void close_episode(const std::string& flag = "") {

	}

	virtual action take_action(const board& state) {
		// std::cout << state;
		if(name() == "mcts"){

		}else if(name() == "heur"){
			// std::cout << "heur" << "\n";
			int heurtmp = 0;
			for (const action::place& move : space) {
				board after = state;
				if (move.apply(after) == board::legal && heurtmp%2 != 0){
					return move;
				}
				heurtmp ++;
			}
			heurtmp = 0;
			
			for (const action::place& move : space) {
				board after = state;
				if (move.apply(after) == board::legal && check_neighbor(state, heurtmp)){
					return move;
				}
				heurtmp ++;
			}
			
			heurtmp = 0;
			std::shuffle(space.begin(), space.end(), engine);
			for (const action::place& move : space) {
				board after = state;
				if (move.apply(after) == board::legal){
					return move;
				}
				heurtmp ++;
			}
			return action();

		}else{
			std::shuffle(space.begin(), space.end(), engine);
			for (const action::place& move : space) {
				board after = state;
				if (move.apply(after) == board::legal){
					
					return move;
				}
				
			}
			return action();
		}
	}

	bool check_neighbor(const board& b, const int num){
		int target_color = 1;
		if(who == board::black){
			target_color = 2;
		}
		// std::cout << num << " : " << who << " : " << target_color << "\n";
		

		std::vector<int> dir_num;
		dir_num.clear();
		if((num-1) > 0) dir_num.push_back(num-1);
		if((num+1) < 81 ) dir_num.push_back(num+1);
		if((num-9) > 0 ) dir_num.push_back(num-9);
		if((num+9) < 81 ) dir_num.push_back(num+9);

		for(auto n : dir_num){
			// std::cout << n << "+" << b[n/9][n%9] << " , ";
			if (b[n/9][n%9] == target_color){
				return true;
			}
		}
		// std::cout << "\n";
		return false;
	}

	// void updateValue(node,)

private:
	std::vector<action::place> space;
	board::piece_type who;
	std::string arg;
};
