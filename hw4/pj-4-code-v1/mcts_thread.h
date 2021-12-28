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

struct node{
		double winvalue;
		double totalmove_count;
		int available_node_count;
		action::place move;
		std::vector<node*> level_vector;
		// std::unordered_map<std::string, node*> same_level_node_table;
		node() : winvalue(0),totalmove_count(0),available_node_count(-1),move(1,0){}
	};


class thread_obj{
public:
	thread_obj(int aaa) : c(aaa) {};

	void operator()(node* rootnode, clock_t now, board fixstate){
		std::cout << c << "\n";
		// int sim_times = 0;
		// rootnode->winvalue += 12123;
		// board after;
		// while(now > clock()){
		// 	self_simulate_win = false;
		// 	after = fixstate;
		// 	playOneSequence(rootnode, after);
		// 	updatenode(self_simulate_win);
		// 	sim_times++;
		// 	if(simulation_time == 10005 && sim_times >= simulation_count) break;
		// }
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

    bool simulate_one(board after, bool is_self){
		
		std::vector<action::place> inspace1 = space_a;
		std::vector<action::place> inspace2 = space;
		if(!is_self){
			inspace1 = space;
			inspace2 = space_a;
		}

		bool self_loss_flag;
		std::shuffle(inspace1.begin(), inspace1.end(), engine);
		for (const action::place& move : inspace1) {
			if (move.apply(after) == board::legal){
				self_loss_flag = true;
				move.apply(after);
				// std::cout << "enemy : \n"<< after;
				self_loss_flag = randomplay_loss(after, inspace2);

				// if(self_loss_flag) std::cout << "self loss\n";
				if(self_loss_flag) return true;
			}
		}
		// if(!self_loss_flag) std::cout << " enemy loss\n";
		if(!self_loss_flag) return false;
	}

	bool randomplay_loss(board& after, std::vector<action::place> color_space){

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

protected:
	std::default_random_engine engine;

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