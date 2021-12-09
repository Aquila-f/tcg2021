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
#include <unordered_map>
#include <stack>
#include <type_traits>
#include <algorithm>
// #include "episode.h"
#include "board.h"
#include "action.h"

#include <fstream>

class episode;

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
		space(board::size_x * board::size_y), who(board::empty),
		space_a(board::size_x * board::size_y), who_a(board::empty){
			
		
		if (name().find_first_of("[]():; ") != std::string::npos)
			throw std::invalid_argument("invalid name: " + name());
		if (role() == "black"){ who = board::black; who_a = board::white;}
		if (role() == "white"){ who = board::white; who_a = board::black;}
		if (who == board::empty)
			throw std::invalid_argument("invalid role: " + role());
		if (who_a == board::empty)
			throw std::invalid_argument("invalid role: " + role());

		for (size_t i = 0; i < space.size(); i++)
			space[i] = action::place(i, who);

		for (size_t i = 0; i < space_a.size(); i++)
			space_a[i] = action::place(i, who_a);

	}

	virtual void open_episode(const std::string& flag = "") {
		history.clear();
	}
	virtual void close_episode(const std::string& flag = "") {

	}

	virtual action take_action(const board& state) {
		// std::cout << state;
		if(name() == "mcts"){

			node* rootnode = new node[1];
			board after = state;

			for(int i = 0;i<100;i++){
				simulate_win = false;
				after = state;
				playOneSequence(rootnode, after);
				updatenode(simulate_win);
			}

			// outputnode(rootnode);
			// for(auto k : rootnode->level_vector){
			// 	outputnode(k);
			// }

			// after = state;
			// std::cout << after;
			// randomplay(after, space);
			// std::cout << after;



			node* maxtmpnode = new node[1];
			double maxvalue = -1;

			
			for(auto tno : rootnode->level_vector){
				double v = tno->winvalue/tno->totalmove_count;
				if(maxvalue <= v){
					maxvalue = v;
					maxtmpnode = tno;
				}
				// outputnode(tno);
			}

			after = state;
			maxtmpnode->move.apply(after);
			// if(assss == 3) exit(0);
			// assss += 1;
			return maxtmpnode->move;

			exit(0);

		}else if(name() == "heur"){
			// // std::cout << "heur" << "\n";
			// int heurtmp = 0;
			// for (const action::place& move : space) {
			// 	board after = state;
			// 	if (move.apply(after) == board::legal && heurtmp%2 != 0){
			// 		return move;
			// 	}
			// 	heurtmp ++;
			// }
			// heurtmp = 0;

			// for (const action::place& move : space) {
			// 	board after = state;
			// 	if (move.apply(after) == board::legal && check_neighbor(state, heurtmp)){
			// 		return move;
			// 	}
			// 	heurtmp ++;
			// }
			
			// heurtmp = 0;
			// std::shuffle(space.begin(), space.end(), engine);
			// for (const action::place& move : space) {
			// 	board after = state;
			// 	if (move.apply(after) == board::legal){
			// 		return move;
			// 	}state
			// 	heurtmp ++;
			// }
			// return action();

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
	bool simulate_one(board after){
		// std::cout << "self : "<< who << "\n";
		bool self_loss_flag;
		std::shuffle(space_a.begin(), space_a.end(), engine);
		for (const action::place& move : space_a) {
			if (move.apply(after) == board::legal){
				self_loss_flag = true;
				move.apply(after);
			
				// std::cout << "enemy : \n"<< after;

				self_loss_flag = randomplay(after, space);


				// if(self_loss_flag) std::cout << "self loss\n";
				if(self_loss_flag) return true;
			}
		}
		// if(!self_loss_flag) std::cout << " enemy loss\n";
		if(!self_loss_flag) return false;
	}

	bool randomplay(board& after, std::vector<action::place> color_space){

		std::shuffle(color_space.begin(), color_space.end(), engine);
		for (const action::place& move_a : color_space){
			if (move_a.apply(after) == board::legal){
				move_a.apply(after);
				// std::cout << "self : \n" << after;
				return false;
			}
		}
		return true;
	}

	struct node{
		double winvalue;
		double totalmove_count;
		int available_node_count;
		action::place move;
		std::vector<node*> level_vector;
		// std::unordered_map<std::string, node*> same_level_node_table;
		node() : winvalue(0),totalmove_count(0),available_node_count(-1),move(1,1){}
	};
	// struct node1{
	// 	unorder
	// }

	std::vector<node*> history;

	void playOneSequence(node*& rootnode,board& state){

		update_node_vector.push_back(rootnode);

		board after = state;

		if(rootnode->available_node_count == -1){
			int heurtmp = 0;
			for (const action::place move : space) {
				after = state;
				if (move.apply(after) == board::legal){
					node* tmpnode = new node[1];
					tmpnode->move = move;
					rootnode->level_vector.push_back(tmpnode);
					heurtmp ++;	
				}
			}
			rootnode->available_node_count = heurtmp;
			if(heurtmp == 0) return;
		}

		node* tpnode = new node[1];
		double maxtnoval = -1;
		bool deeper_flag = true;

		for(auto tno : rootnode->level_vector){
			
			
			
			if(tno->totalmove_count == 0){
				// tno->totalmove_count += 1;
				tno->move.apply(state);
				update_node_vector.push_back(tno);

				if(!simulate_one(state)){
					simulate_win = true;
					// tno->winvalue += 1;
				};

				deeper_flag = false;
				break;

			}else{
				double v = tno->winvalue/tno->totalmove_count;
				if(maxtnoval <= v){
					maxtnoval = v;
					tpnode = tno;
				}
			}
		}

		

		if(deeper_flag){
			// std::cout << "####################\n";	
			// for(auto tno : rootnode.level_vector){
			// 	// outputnode(tno);
			// }

			// std::cout << tpnode->move.position() << "\n";

			tpnode->move.apply(state);
			if(simulate_white_move(state)){
				simulate_win = false;
				return;
			};
			// update_node_vector.push_back(tpnode);

			playOneSequence(tpnode, state);
		}
	};

	bool simulate_white_move(board& state){
		// std::cout << state;
		if(randomplay(state, space_a)) return true;
		// std::cout << state;
		return false;
	}

	// bool check_neighbor(const board& b, const int num){
	// 	int target_color = 1;
	// 	if(who == board::black){
	// 		target_color = 2;

	// 	}
	// 	// std::cout << num << " : " << who << " : " << target_color << "\n";
		

	// 	std::vector<int> dir_num;
	// 	dir_num.clear();
	// 	if((num-1) > 0) dir_num.push_back(num-1);
	// 	if((num+1) < 81 ) dir_num.push_back(num+1);
	// 	if((num-9) > 0 ) dir_num.push_back(num-9);
	// 	if((num+9) < 81 ) dir_num.push_back(num+9);

	// 	for(auto n : dir_num){
	// 		// std::cout << n << "+" << b[n/9][n%9] << " , ";
	// 		if (b[n/9][n%9] == target_color){
	// 			return true;
	// 		}
	// 	}
	// 	// std::cout << "\n";
	// 	return false;
	// }

	// void updateValue(node,)

	void updatenode(bool win){
		// std::cout << "updatenode-----\n";
		if(win){
			for(auto updatenode : update_node_vector){	
				updatenode->totalmove_count += 1;
				updatenode->winvalue += 1;
				// outputnode(updatenode);
			}

		}else{
			for(auto updatenode : update_node_vector){
				updatenode->totalmove_count += 1;
				// outputnode(updatenode);
			}
		}
		// std::cout << "updatenode-----end\n";
		update_node_vector.clear();
	}

	void outputnode(const node* tno){
		std::cout << "move: " << action::place(tno->move).position() << " ";
		std::cout << "ttcu: " << tno->totalmove_count << " ";
		std::cout << "winc: " << tno->winvalue << " ";
		std::cout << "ttcu: " << tno->available_node_count << "\n";
	}

private:
	std::vector<action::place> space;
	board::piece_type who;
	std::vector<action::place> space_a;
	board::piece_type who_a;
	std::string arg;
	node* statenode;
	std::vector<node*> update_node_vector;
	bool simulate_win;

};
