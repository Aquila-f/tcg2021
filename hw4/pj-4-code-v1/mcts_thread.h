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



struct node_result{
		std::vector<double> result_vector;
		std::vector<int> ava_vector;
		node_result() : result_vector(81){}
	};

struct node{
		int move_position;
		double winvalue;
		double totalmove_count;
		double rave_win_count;
		double rave_totalmove_count;
		int available_node_count;
		// action::place move;
		std::vector<node*> level_vector;
		std::vector<int> empty;
		std::vector<int> random_vector;
		// std::unordered_map<std::string, node*> same_level_node_table;
		// node(1) : winvalue(0),totalmove_count(0),available_node_count(-1),move(1,0){}
		// node() : winvalue(0),totalmove_count(0),available_node_count(-1),move(1,0){}
		node() : winvalue(0),totalmove_count(0),rave_win_count(0),rave_totalmove_count(0),available_node_count(-1){}
	};


class mcts_thread_obj{
public:
	mcts_thread_obj(int aaa, std::vector<action::place> space, std::vector<action::place> space_a,
	double c, double simulation_count, double simulation_time, std::string enemy_state_playmode,
	std::default_random_engine engine) : 
	thread_id(aaa), space_(space), space_a_(space_a), enemy_state_playmode_(enemy_state_playmode),
	c_(c), simulation_count_(simulation_count),
	simulation_time_(simulation_time), rd_engine(engine){};

	void operator()(node_result* rootnode, clock_t limit_time_, board fixstate){
		node* rootnode1 = new node[1];
		release_node_vector.push_back(rootnode1);
		
		int sim_times = 0;
		
		board after;

		while(limit_time_ > millisec()){
			self_simulate_win = false;
			after = fixstate;
			playOneSequence(rootnode1, after);
			updatenode(self_simulate_win);
			sim_times++;
			if(simulation_time_ == 10005 && sim_times >= simulation_count_) break;
		}
		std::cout << sim_times << std::endl;

		// for(int i=0;i<rootnode1->level_vector.size();i++){	
		// 	outputnode(rootnode1->level_vector[i]);
		// }
		// std::cout << "++++++++++++++++++++++++++++++++++++++++++++++\n";
		
		// for(int i=0;i<rootnode1->level_vector.size();i++){	
		// 	outputnode(rootnode1->level_vector[i]);
		// }
		
		int final_position = 82;

		double maxvalue = 0;
		
		for(auto n : rootnode1->random_vector){
			// outputnode(rootnode1->level_vector[n]);
			rootnode->result_vector[n] = rootnode1->level_vector[n]->totalmove_count;
			// rootnode->result_vector[n] = rootnode1->level_vector[n]->rave_win_count/rootnode1->level_vector[n]->rave_totalmove_count;
			// std::cout << n << " : " << rootnode->result_vector[n] << "\n";
		}
		// std::cout << "-- " << final_position << " : " << maxvalue << " --\n";
		rootnode->ava_vector = rootnode1->random_vector;

		for(auto a : release_node_vector){
			delete [] a;
		}
		
	}

	void updatenode(bool win){
		
		// for(int i=0;i<play_sequence.size();i++){
		// 	std::cout << play_sequence[i] << ",";
		// }
		// std::cout << "\n";

		if(win){
			for(int i=0;i<update_node_vector.size();i++){
				// std::cout << update_node_vector[i]->move_position << " : " << play_sequence[i] << "\n";
				if(i != update_node_vector.size()-1){
					for(int j=i; j<play_sequence.size();j++){
						
						update_node_vector[i]->level_vector[play_sequence[j]]->rave_totalmove_count += 1;
						if(j % 2 == 0)	update_node_vector[i]->level_vector[play_sequence[j]]->rave_win_count += 1;
					}
					
				}
				update_node_vector[i]->totalmove_count += 1;
				update_node_vector[i]->winvalue += 1;
			}

		}else{
			for(int i=0;i<update_node_vector.size();i++){
				
				// std::cout << update_node_vector[i]->move_position << " : " << play_sequence[i] << "\n";
				if(i != update_node_vector.size()-1){
					for(int j=i; j<play_sequence.size();j++){
						
						update_node_vector[i]->level_vector[play_sequence[j]]->rave_totalmove_count += 1;
						if(j % 2 == 1) update_node_vector[i]->level_vector[play_sequence[j]]->rave_win_count += 1; 
					}
				}
				update_node_vector[i]->totalmove_count += 1;
				// updatenode->winvalue += 1;
			}
		}
		// std::cout << "updatenode-----end\n";
		update_node_vector.clear();
		play_sequence.clear();
		
	}

    bool simulate_one(board& after, node* nodeforempty){

		std::shuffle(nodeforempty->empty.begin(), nodeforempty->empty.end(), rd_engine);
		
		int b = nodeforempty->empty.size()-1;
		for(int a=0; a<nodeforempty->empty.size(); a++){
			if(after.place(nodeforempty->empty[a]) == board::legal){
				play_sequence.push_back(nodeforempty->empty[a]);
				for(;b>=0;b--){
					if(after.place(nodeforempty->empty[b]) == board::legal){
						play_sequence.push_back(nodeforempty->empty[b]);
						break;
					}
				}
				if(b == -1){/*sleep(1); std::cout << after;*/ return true;}
			}
		}
		return false;

	}

    void playOneSequence(node*& rootnode,board& state){

		

		board after = state;

		if(rootnode->available_node_count == -1){
			int heurtmp = 0;

			node* tmpnode = new node[81];
			release_node_vector.push_back(tmpnode);
			for (int i = 0; i < 81; i++){
				// node* tmpnode = new node[1];
				rootnode->level_vector.push_back(&tmpnode[i]);
				after = state;// std::cout << state;
				if(after(i) == board::empty){
					rootnode->empty.push_back(i);
					if(after.place(i) == board::legal){
						tmpnode[i].move_position = i;
						rootnode->random_vector.push_back(i);
						heurtmp ++;
					}	
				}
			}


			rootnode->available_node_count = heurtmp;
			if(heurtmp == 0) return;
			std::shuffle(rootnode->random_vector.begin(), rootnode->random_vector.end(), rd_engine);
		}

		if(rootnode->random_vector.size() == 0) return;
		update_node_vector.push_back(rootnode);




		double maxtnoval = -1;
		double v;
		int max_board_place = 0;

		for(auto board_place : rootnode->random_vector){
			if(rootnode->level_vector[board_place]->totalmove_count == 0){
				// std::cout << state;
				// std::cout << board_place << ":" << rootnode->level_vector[board_place]->move_position << "\n";
				state.place(board_place);
				play_sequence.push_back(board_place);
				update_node_vector.push_back(rootnode->level_vector[board_place]);
				if(!simulate_one(state, rootnode)) self_simulate_win = true;
				// std::cout << state;
				return;
				// // std::cout << state;
				// exit(0);
			}else{
				int b = 0;
				// v = rootnode->level_vector[board_place]->winvalue/rootnode->level_vector[board_place]->totalmove_count+c_*sqrt(log(rootnode->totalmove_count)/rootnode->level_vector[board_place]->totalmove_count);
				v = rootnode->level_vector[board_place]->rave_win_count/rootnode->level_vector[board_place]->rave_totalmove_count;
				if(maxtnoval <= v){
					maxtnoval = v;
					max_board_place = board_place;
				}
			}
		}
		state.place(max_board_place);
		play_sequence.push_back(max_board_place);

		simulate_enemy_move(rootnode->level_vector[max_board_place], state);
	};

	void simulate_enemy_move(node*& rootnode, board& state){
		
		// std::cout << playmode << ",";
		

		double v;

		

		board after = state;

		node* tmpnode = new node[81];
		release_node_vector.push_back(tmpnode);
		if(rootnode->available_node_count == -1){
			int heurtmp = 0;
			for (int i = 0; i < 81; i++){
				// node* tmpnode = new node[1];
				rootnode->level_vector.push_back(&tmpnode[i]);
				after = state;
				if(after(i) == board::empty){
					rootnode->empty.push_back(i);
					if(after.place(i) == board::legal){
						tmpnode[i].move_position = i;
						rootnode->random_vector.push_back(i);
						heurtmp ++;
					}
				}
			}


			rootnode->available_node_count = heurtmp;
			if(heurtmp == 0){
				self_simulate_win = true;
				return;
			}
			std::shuffle(rootnode->random_vector.begin(), rootnode->random_vector.end(), rd_engine);
		}

		if(rootnode->random_vector.size() == 0){
			self_simulate_win = true;
			return;
		}
		update_node_vector.push_back(rootnode);

		
		double maxtnoval = -1;
		int max_board_place = -1;

		for(auto board_place : rootnode->random_vector){
			if(rootnode->level_vector[board_place]->totalmove_count == 0){
				// std::cout << state;
				// std::cout << board_place << ":" << rootnode->level_vector[board_place]->move_position << "\n";
				state.place(board_place);
				play_sequence.push_back(board_place);
				update_node_vector.push_back(rootnode->level_vector[board_place]);
				if(simulate_one(state, rootnode)) self_simulate_win = true;
				// std::cout << state;
				return;
				
				exit(0);
			}else{
				// v = 1-(rootnode->level_vector[board_place]->winvalue/rootnode->level_vector[board_place]->totalmove_count)+c_*sqrt(log(rootnode->totalmove_count)/rootnode->level_vector[board_place]->totalmove_count);
				v = rootnode->level_vector[board_place]->rave_win_count/rootnode->level_vector[board_place]->rave_totalmove_count;

				if(maxtnoval <= v){
					maxtnoval = v;
					
					max_board_place = board_place;
				}
			}
		}

		state.place(max_board_place);
		play_sequence.push_back(max_board_place);
		playOneSequence(rootnode->level_vector[max_board_place], state);
	}

time_t millisec() {
		auto now = std::chrono::system_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
	}
void outputnode(const node* tno){
		std::cout << "move: " << tno->move_position << " ";
		std::cout << "ttcu: " << tno->totalmove_count << " ";
		std::cout << "winc: " << tno->winvalue << " ";
		std::cout << "r_ttcu: " << tno->rave_totalmove_count << " ";
		std::cout << "r_winc: " << tno->rave_win_count << " ";
		std::cout << "ttcu: " << tno->available_node_count << "\n";
	}


private:
	int thread_id;
	std::vector<action::place> space_;
	std::vector<action::place> space_a_;

	std::vector<int> play_sequence;
	std::vector<node*> update_node_vector;
	std::vector<node*> release_node_vector;
	bool self_simulate_win;
	std::string enemy_state_playmode_;
	
	double c_;
	double simulation_count_;
	double simulation_time_;
	std::default_random_engine rd_engine;
};