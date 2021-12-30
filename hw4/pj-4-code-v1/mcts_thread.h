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

#include <unistd.h>
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


class mcts_thread_obj{
public:
	mcts_thread_obj(int aaa, std::vector<action::place> space, std::vector<action::place> space_a,
	double c, double simulation_count, double simulation_time, std::string enemy_state_playmode,
	std::default_random_engine engine) : 
	thread_id(aaa), space_(space), space_a_(space_a), enemy_state_playmode_(enemy_state_playmode),
	c_(c), simulation_count_(simulation_count),
	simulation_time_(simulation_time), rd_engine(engine){};

	void operator()(node* rootnode, clock_t limit_time_, board fixstate){
		// node* rootnode1 = new node[1];
		
		int sim_times = 0;
		
		board after;

		while(limit_time_ > millisec()){
			self_simulate_win = false;
			after = fixstate;
			playOneSequence(rootnode, after);
			updatenode(self_simulate_win);
			sim_times++;
			if(simulation_time_ == 10005 && sim_times >= simulation_count_) break;
		}
		// std::cout << sim_times << std::endl;
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
		
		std::vector<action::place> inspace1 = space_a_;
		std::vector<action::place> inspace2 = space_;
		if(!is_self){
			inspace1 = space_;
			inspace2 = space_a_;
		}

		// bool self_loss_flag;
		std::shuffle(inspace1.begin(), inspace1.end(), rd_engine);
		// std::random_shuffle(inspace1.begin(), inspace1.end());
		// std::random_shuffle(inspace2.begin(), inspace2.end());
		std::shuffle(inspace2.begin(), inspace2.end(), rd_engine);
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
			}
		}
		return false;
	}

	bool randomplay_loss(board& after, std::vector<action::place> color_space){

		std::shuffle(color_space.begin(), color_space.end(), rd_engine);
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

			std::shuffle(space_.begin(), space_.end(), rd_engine);

			for (const action::place move : space_) {
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
				v = tno->winvalue/tno->totalmove_count+c_*sqrt(log(rootnode->totalmove_count)/tno->totalmove_count);
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
			std::shuffle(space_a_.begin(), space_a_.end(), rd_engine);
			for (const action::place move : space_a_) {
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
		if(enemy_state_playmode_ == "random"){
			std::shuffle(rootnode->level_vector.begin(), rootnode->level_vector.end(), rd_engine);
			
			tpnode = rootnode->level_vector[0];
			tpnode->move.apply(state);
			
			// std::cout << state;
		}else if(enemy_state_playmode_ == "mctsn"){

			double maxtnoval = -1;

			for(auto tno : rootnode->level_vector){
				
				if(tno->totalmove_count == 0){

					tno->move.apply(state);
					update_node_vector.push_back(tno);

					if(simulate_one(state, false)) self_simulate_win = true;
					// delete [] tpnode;
					return;

				}else{
					
					v = tno->winvalue/tno->totalmove_count+c_*sqrt(log(rootnode->totalmove_count)/tno->totalmove_count);
					
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
					v = 1-(tno->winvalue/tno->totalmove_count)+c_*sqrt(log(rootnode->totalmove_count)/tno->totalmove_count);
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

time_t millisec() {
		auto now = std::chrono::system_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
	}

// protected:
// 	std::default_random_engine engine;

private:
	int thread_id;
	std::vector<action::place> space_;
	std::vector<action::place> space_a_;

	std::vector<node*> update_node_vector;
	bool self_simulate_win;
	std::string enemy_state_playmode_;
	
	double c_;
	double simulation_count_;
	double simulation_time_;
	std::default_random_engine rd_engine;
};