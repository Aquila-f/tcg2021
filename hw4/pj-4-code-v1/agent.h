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

			std::cout << state;

			node rootnode;
			std::stack<action::place> action_stack;
			std::vector<node*> update_node;
			board after = state;
			;

			int heurtmp = 0;
			for (const action::place move : space) {
				after = state;
				if (move.apply(after) == board::legal){
					
					action_stack.push(move);
					heurtmp ++;	
				}
			}
			if(heurtmp == 0) return action();
			
			std::cout << heurtmp << "-\n"; 

			for(int i=0;i<100;i++){
				
				after = state;
				if(action_stack.size() != 0){
					node* tmpnode = new node[1];
					// node* tmpnode = mall; 
					
					tmpnode->move = action_stack.top();
					tmpnode->totalmove_count += 1;
					tmpnode->move.apply(after);

					// std::cout << after;
					

					if(!simulate_one(after)){
						tmpnode->winvalue += 1;
					};

					rootnode.level_vector.push_back(tmpnode);
					action_stack.pop();

					// std::cout << action_stack.size() << "\n";
				}else{
					// std::cout << rootnode.level_vector.size() << ",";


					// std::cout << i << ",\n";
					after = state;
					node* maxtmpnode = new node[1];
			
					double maxvalue = -1;
					
					for(auto tno : rootnode.level_vector){
						double v = tno->winvalue/tno->totalmove_count;
						if(maxvalue <= v){
							maxvalue = v;
							maxtmpnode = tno;

						}
						// outputnode(tno);
					}

					// outputnode(maxtmpnode);

					maxtmpnode->totalmove_count +=1;
					maxtmpnode->move.apply(after);
					if(!simulate_one(after)){
						maxtmpnode->winvalue += 1;
					};
				}

			}
			

			after = state;
			node* maxtmpnode = new node[1];
			double maxvalue = -1;

			
			for(auto tno : rootnode.level_vector){
				double v = tno->winvalue/tno->totalmove_count;
				if(maxvalue <= v){
					maxvalue = v;
					maxtmpnode = tno;
				}
				outputnode(tno);
			}

			maxtmpnode->move.apply(after);
			// if(assss == 3) exit(0);
			// assss += 1;
			return maxtmpnode->move;
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
			// 	}
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

	int a;
	unsigned b;
	action::place s(int,unsigned);
	struct node{
		double winvalue;
		double totalmove_count;
		// int available_node_count = 0;
		action::place move;
		std::vector<node*> level_vector;
		// std::unordered_map<std::string, node*> same_level_node_table;
		node() : winvalue(0),totalmove_count(0),move(1,1){}
	};
	// struct node1{
	// 	unorder
	// }

	std::vector<node*> history;

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

	void outputnode(const node* tno){
		std::cout << "move: " << tno->move << " ";
		std::cout << "ttcu: " << tno->totalmove_count << " ";
		std::cout << "winc: " << tno->winvalue << " \n";
	}

private:
	std::vector<action::place> space;
	board::piece_type who;
	std::vector<action::place> space_a;
	board::piece_type who_a;
	std::string arg;
	node* statenode;
	int assss = 0;
};
