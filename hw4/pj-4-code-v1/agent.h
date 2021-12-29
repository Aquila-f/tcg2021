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
#include <ctime>
#include "board.h"
#include "action.h"



#include <thread>
#include <fstream>

class thread_obj;



// struct node{
// 		double winvalue;
// 		double totalmove_count;
// 		int available_node_count;
// 		action::place move;
// 		std::vector<node*> level_vector;
// 		// std::unordered_map<std::string, node*> same_level_node_table;
// 		node() : winvalue(0),totalmove_count(0),available_node_count(-1),move(1,0){}
// 	};


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
	virtual std::string mcts_c() const { return property("c"); }
	virtual std::string enemy_state_playtype() const { return property("simenemy"); }
	virtual std::string s_count() const { return property("N"); }
	virtual std::string t_count() const { return property("T"); }
	virtual std::string time_m_type() const { return property("TMT"); }
	

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
		space_a(board::size_x * board::size_y), /*who_a(board::empty),*/
		enemy_state_playmode("mctsn"),c(0.1),simulation_count(100),simulation_time(10005),
		time_management(1)
		{
			
		
		if (name().find_first_of("[]():; ") != std::string::npos)
			throw std::invalid_argument("invalid name: " + name());
		if (role() == "black"){ who = board::black; /*who_a = board::white;*/}else{who = board::white;}
		/*if (role() == "white"){ who = board::white; who_a = board::black;}*/
		if (enemy_state_playtype() != "mctsn") enemy_state_playmode = enemy_state_playtype();
		if (mcts_c() != "0") c = atof(mcts_c().c_str());
		if (s_count() != "100") simulation_count = atof(s_count().c_str());
		if (t_count() != "0"){simulation_time = atof(t_count().c_str());} 
		if (time_m_type() != "normal"){time_management_type = time_m_type();} 
		if (who == board::empty)
			throw std::invalid_argument("invalid role: " + role());
		/*if (who_a == board::empty)
			throw std::invalid_argument("invalid role: " + role());*/

		for (size_t i = 0; i < space.size(); i++)
			space[i] = action::place(i, who);

		for (size_t i = 0; i < space_a.size(); i++)
			space_a[i] = action::place(i, who == board::white ? board::black : board::white);
		
	}

	virtual void open_episode(const std::string& flag = "") {

	}
	//td
	virtual void close_episode(const std::string& flag = "") {
		time_management = 1;

	}



	virtual action take_action(const board& state) {
		// std::cout << state;
		// std::cout << state.info() << "\n";
		if(name() == "mcts"){

			node* rootnode = new node[1];
			
			
			board after = state;
			double v;
			int sim_times = 0;

			clock_t now = clock()+time_managment();

			



			// thread_obj snb(5, space, space_a, c, simulation_count, simulation_time, enemy_state_playmode);

			// std::thread th1(snb, &rootnode[0] ,now ,after);

			// th1.join();

			// for(auto tno : rootnode->level_vector){
				
			// 	outputnode(tno);
			// }


			
			while(now > clock()){
				self_simulate_win = false;
				after = state;
				playOneSequence(rootnode, after);
				updatenode(self_simulate_win);
				sim_times++;
				if(simulation_time == 10005 && sim_times >= simulation_count) break;
			}

			std::cout << sim_times << "-\n";

			// outputnode(&rootnode[0]);
			// outputnode(&rootnode[1]);
			
			
			// for(int i = 0;i<simulation_count;i++){
			// 	self_simulate_win = false;
			// 	after = state;
			// 	playOneSequence(rootnode, after);
			// 	updatenode(self_simulate_win);
			// }

			action::place final_move(0,0);
			
			// std::cout << action << "-\n";
			double maxvalue = -1;

			
			for(auto tno : rootnode->level_vector){
				// v = tno->winvalue/tno->totalmove_count;
				v = tno->totalmove_count;
				if(maxvalue <= v){
					maxvalue = v;
					final_move = tno->move;
					// maxtmpnode = tno;
				}
				// outputnode(tno);
			}

			after = state;
			final_move.apply(after);

			noderelease(rootnode);

			// std::cout << final_move << "-\n";

			// if(final_move)
			return final_move;

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

	bool simulate_one(board after, bool is_self){
		
		std::vector<action::place> inspace1 = space_a;
		std::vector<action::place> inspace2 = space;
		if(!is_self){
			inspace1 = space;
			inspace2 = space_a;
		}

		// bool self_loss_flag;
		std::shuffle(inspace1.begin(), inspace1.end(), engine);
		// std::shuffle(inspace2.begin(), inspace2.end(), engine);
		int rmc = 0;
		for (const action::place& move : inspace1) {
			if (move.apply(after) == board::legal){
				// self_loss_flag = true;
				move.apply(after);
				// std::cout << "enemy : \n"<< after;

				for(;rmc<inspace2.size();rmc++){
					if(inspace2[rmc].apply(after) == board::legal){

						inspace2[rmc].apply(after);
						// self_loss_flag = false;
						break;
					}
				}
				if(rmc == inspace2.size()) return true;

				// self_loss_flag = randomplay_loss(after, inspace2);

				// if(self_loss_flag) std::cout << "self loss\n";
				// if(self_loss_flag) return true;
			}
		}
		// if(!self_loss_flag) std::cout << " enemy loss\n";
		// if(!self_loss_flag) return false;
		return false;
	}

	bool randomplay_loss(board &after, std::vector<action::place> &color_space){

		std::shuffle(color_space.begin(), color_space.end(), engine);

		for (const action::place& move_a : color_space){
			if (move_a.apply(after) == board::legal){
				move_a.apply(after);
				// std::cout << "self : \n" << after;
				return false;
			}
		}
		//loss return ture
		return true;
	}

	void playOneSequence(node*& rootnode,board& state){

		update_node_vector.push_back(rootnode);

		board after = state;

		if(rootnode->available_node_count == -1){
			int heurtmp = 0;

			std::shuffle(space.begin(), space.end(), engine);

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

		if(rootnode->level_vector.size() == 0) return;

		node* tpnode = new node[1];
		double maxtnoval = -1;
		double v;

		for(auto tno : rootnode->level_vector){
			
			if(tno->totalmove_count == 0){
				// tno->totalmove_count += 1;
				tno->move.apply(state);
				update_node_vector.push_back(tno);

				if(!simulate_one(state, true)) self_simulate_win = true;
				// std::cout << &tpnode << ",";
				// delete [] tpnode;
				return;

			}else{
				// std::cout << c*sqrt(log(rootnode->totalmove_count)/tno->totalmove_count) << "\n";
				v = tno->winvalue/tno->totalmove_count+c*sqrt(log(rootnode->totalmove_count)/tno->totalmove_count);
				// v = tno->totalmove_count;
				if(maxtnoval <= v){
					maxtnoval = v;
					tpnode = tno;
				}
			}
		}

		tpnode->move.apply(state);
		simulate_enemy_move(tpnode, state);

	};

	void simulate_enemy_move(node*& rootnode, board& state){
		
		// std::cout << playmode << ",";
		update_node_vector.push_back(rootnode);

		double v;

		

		board after = state;

		if(rootnode->available_node_count == -1){
			int heurtmp = 0;
			std::shuffle(space_a.begin(), space_a.end(), engine);
			for (const action::place move : space_a) {
				after = state;
				if (move.apply(after) == board::legal){
					node* tmpnode = new node[1];
					tmpnode->move = move;
					rootnode->level_vector.push_back(tmpnode);
					heurtmp ++;	
				}
			}
			rootnode->available_node_count = heurtmp;
			if(heurtmp == 0){
				self_simulate_win = true;
				return;
			}
		}

		if(rootnode->level_vector.size() == 0){
			self_simulate_win = true;
			return;
		}
		node* tpnode = new node[1];
		

		// std::cout << "white move size - " << rootnode->level_vector.size() << "\n";
		if(enemy_state_playmode == "random"){
			std::shuffle(rootnode->level_vector.begin(), rootnode->level_vector.end(), engine);
			
			tpnode = rootnode->level_vector[0];
			tpnode->move.apply(state);
			
			// std::cout << state;
		}else if(enemy_state_playmode == "mctsn"){

			double maxtnoval = -1;

			for(auto tno : rootnode->level_vector){
				
				if(tno->totalmove_count == 0){

					tno->move.apply(state);
					update_node_vector.push_back(tno);

					if(simulate_one(state, false)) self_simulate_win = true;
					// delete [] tpnode;
					return;

				}else{
					
					v = tno->winvalue/tno->totalmove_count+c*sqrt(log(rootnode->totalmove_count)/tno->totalmove_count);
					
					// double v = tno->winvalue/tno->totalmove_count;
					if(maxtnoval < v){
						maxtnoval = v;
						tpnode = tno;
					}
				}
			}
			tpnode->move.apply(state);			
		}else{
			double maxtnoval = -1;

			for(auto tno : rootnode->level_vector){
				
				if(tno->totalmove_count == 0){

					tno->move.apply(state);
					update_node_vector.push_back(tno);

					if(simulate_one(state, false)) self_simulate_win = true;
					// delete [] tpnode;
					return;

				}else{
					v = 1-(tno->winvalue/tno->totalmove_count)+c*sqrt(log(rootnode->totalmove_count)/tno->totalmove_count);
					// double v = 1-(tno->winvalue/tno->totalmove_count);
					if(maxtnoval <= v){
						maxtnoval = v;
						tpnode = tno;
					}
				}
			}
			tpnode->move.apply(state);	
		}
		playOneSequence(tpnode, state);
		
		
	}

	double time_managment(){
		double return_time;
		if(time_management_type == "er"){
			return_time = ceil(7*pow(time_management, -0.8)*1000)*simulation_time;
		}else if(time_management_type == "lr"){
			return_time = (1.9-0.05*time_management)*1000*simulation_time;
		}else{
			return_time = 1000*simulation_time;
		}
		time_management++;
		return return_time;
	}

	void noderelease(node* release_node){

		while(release_node->level_vector.size() > 0){
			// node* dd = release_node->level_vector.back();
			// outputnode(dd);
			noderelease(release_node->level_vector.back());
			// outputnode(dd);
			
			
			release_node->level_vector.pop_back();
		}
		release_node->level_vector.clear();
		// std::cout << release_node->level_vector.size() << ",";
		delete [] release_node;
		// return;
	}

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
		std::cout << "move: " << action::place(tno->move) << " ";
		std::cout << "ttcu: " << tno->totalmove_count << " ";
		std::cout << "winc: " << tno->winvalue << " ";
		std::cout << "ttcu: " << tno->available_node_count << "\n";
	}

	void timetransform(const time_t now){
		tm *ltm = localtime(&now);
		std::cout << "time: "<< ltm->tm_hour << ":";
		std::cout << ltm->tm_min << ":";
		std::cout << ltm->tm_sec << std::endl;
	}

	

private:
	std::vector<action::place> space;
	board::piece_type who;
	std::vector<action::place> space_a;

	
	std::vector<node*> update_node_vector;
	bool self_simulate_win;
	std::string enemy_state_playmode;
	double c;
	double simulation_count;
	double simulation_time;

	
	std::string time_management_type;
	double time_management;
};


