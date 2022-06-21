//change that doesn't work well
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <array>

#include <limits>
#include <unordered_set>
#include <list>
#include <set>
#include <string>
#include <sstream>

#define LOG_ON false
#define MM_DEBUG false
#define SVF_TOGGLE false
#define svf_value_check false
#define svf_win_array_check false
#define svf_win_array_check_initial false
#define svf_debug_win_array_change false
#define print_win_ways false
#define win_method_dimension_check false

enum SPOT_STATE {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

int player;
const int SIZE = 15;
std::array<std::array<int, SIZE>, SIZE> board;
typedef std::array<std::array<int, SIZE>, SIZE> state;

typedef std::pair<int,int> p_type;

//empty and occupied array
std::set<std::array<int,3>> empty_location;
std::set<std::array<int,3>> occu_location;
//winning array
std::array <std::array<std::array<bool,572>,SIZE>,SIZE>winning_methods;
//winning progress array
std::array<int,573> my_stra_win;
std::array<int,573> enemy_stra_win;


///ultility func
void my_log(std::string str){
    FILE* fptr=fopen("log.txt","a");
    fputs (str.c_str(),fptr);
    fputs("\n",fptr);
    fclose (fptr);
    return ;
}
void my_log_reset(){
    FILE* fptr=fopen("log.txt","w");
    fclose (fptr);
    return ;
}
void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];

            //record empty location and occupied location
            //if current is a stone
            if(board[i][j]){
                std::array<int,3> temp={i,j,board[i][j]};
                occu_location.insert(temp);
            }
            else{//current is empty
                std::array<int,3> temp={i,j,board[i][j]};
                empty_location.insert(temp);
            }
        }
    }
}
    std::string encode_player(int state) {
        if (state == BLACK) return "O";
        if (state == WHITE) return "X";
        return "Draw";
    }
    std::string encode_spot(int x, int y) {
        if (board[x][y]== EMPTY) return ".";
        if (board[x][y] == BLACK) return "O";
        if (board[x][y] == WHITE) return "X";
        return " ";
    }
    std::string encode_output() {
        int i, j;
        std::stringstream ss;
        ss << "+-----------------------------+\n";
        ss<<"-|0 1 2 3 4 5 6 7 8 9 a b c d e|\n";
        char lib[15]={'0', '1','2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd','e'};
        for (i = 0; i < SIZE; i++) {
            ss << lib[i]<<"|";
            for (j = 0; j < SIZE-1; j++) {
                ss << encode_spot(i, j) << " ";
            }
            ss << encode_spot(i, j) << "|\n";
        }
        ss << "===============================\n";
        return ss.str();
    }
    std::string winning_array_output() {
        std::stringstream ss;
        ss << "+-------------my win----------------+\n";
        int counting=0;
        for(auto data:my_stra_win){
            counting++;

            ss<<std::to_string(data)<<" ";

            if(counting==10){
                ss<<"\n";
                counting=0;
            }
        }

        counting=0;
        ss << "+-------------enemy win----------------+\n";
        for(auto data:enemy_stra_win){
            counting++;
            ss<<std::to_string(data)<<" ";

            if(counting==10){
                ss<<"\n";
                counting=0;
            }
        }
        ss << "===============================\n";
        return ss.str();
    }


std::string string_movement (std::array<int,3>a_movement){
    std::string ret_str;
    ret_str="row:"+std::to_string(a_movement[0])+" col:"+std::to_string(a_movement[0])+" type:"+std::to_string(a_movement[2]);
    return ret_str;
}

//structs
//for minimax
typedef std::array<int,3> movement;//x,y,by who
typedef std::list<movement> list_of_movement;
typedef struct minimax_result_{
    int long long minimax_value;
    movement chosen_movement;
}minimax_result;

typedef struct svf_result_{
    long long int svf_value;
    int gameover;
}svf_result;

//declaration of functions
minimax_result minimax(list_of_movement given_movement, int depth, bool maximizingPlayer);
minimax_result alphabeta(list_of_movement given_list, int depth,long long int alpha,long long int beta, bool maximizingPlayer);
void win_array_gen();
svf_result state_value_function(list_of_movement given_list);
void write_valid_spot(std::ofstream& fout);

///generate winninng array
void win_array_gen(){
    //initialize winning methods
    //confirm correct
    std::array<std::array<bool,572>,SIZE> temp_two_d_array;temp_two_d_array.fill({false,false,false,false,false});

    winning_methods.fill(temp_two_d_array);//fill 2d array



    my_stra_win.fill(0);
    enemy_stra_win.fill(0);

    //fill 0 successful
    if(svf_win_array_check_initial){my_log(winning_array_output());}

    //make sure not empty
    if(occu_location.empty()){return ;}

    int count=0;
    //horizontal wins
    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 11; j++) {
            if(print_win_ways){my_log("win way: "+std::to_string(count));}
            for (int k = 0; k < 5; k++) {

                winning_methods[i][j+k][count] = true;
                if(print_win_ways){my_log("row: "+std::to_string(i)+" col:"+std::to_string(j+k));}
            }
            count++;
        }
    }

    //vertical wins
    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 15; j++) {
            if(print_win_ways){my_log("win way: "+std::to_string(count));}
            for (int k = 0; k < 5; k++) {
                winning_methods[i+k][j][count] = true;
                if(print_win_ways){my_log("row: "+std::to_string(i+k)+" col:"+std::to_string(j));}
            }
            count++;
        }
    }

    //ltr wins
    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {
            if(print_win_ways){my_log("win way: "+std::to_string(count));}
            for (int k = 0; k < 5; k++) {
                winning_methods[i + k][j + k][count] = true;
                if(print_win_ways){my_log("row: "+std::to_string(i+k)+" col:"+std::to_string(j+k));}
            }
            count++;
        }
    }

    //rtl wins
    for (int i = 14; i >=4; i--) {
        for (int j = 0; j < 11; j++) {
            if(print_win_ways){my_log("win way: "+std::to_string(count));}
            for (int k = 0; k < 5; k++) {
                winning_methods[i - k][j + k][count] = true;
                if(print_win_ways){my_log("row: "+std::to_string(i-k)+" col:"+std::to_string(j+k));}
            }
            count++;
        }
    }

    if(win_method_dimension_check){
        for(auto i:winning_methods){
            for(auto j:i){
                for(auto k:j){
                    std::cout<<k<<" ";
                }
                std::cout<<"\n";
            }
        }
    }

    ///first evaluation of the board to make the win array and enemy win array

    for(auto points:occu_location){
        for(int way=0;way<572;way++){
            if(winning_methods[points[0]/*row*/][points[1]/*col*/][way]){
                //update my winning strategy
                if(points[2]==player){

                    if(my_stra_win[way]>=0){my_stra_win[way]++;}
                    enemy_stra_win[way]=-2;
                }

                //update enemy winning strategy
                else if (points[2]==3-player){
                    if(enemy_stra_win[way]>=0){enemy_stra_win[way]++;}
                    my_stra_win[way]=-2;
                }
                //if(svf_debug_win_array_change
            }
        }
    }
    if(svf_win_array_check){my_log(winning_array_output());}

    if(SVF_TOGGLE){my_log("we have wininig method of "+std::to_string(count));}
    return ;
}

///state value func


svf_result state_value_function(list_of_movement given_list){
    svf_result wrap_result;wrap_result.gameover=0;wrap_result.svf_value=0;
    long long int player_mark=0;
    //modify board first//will be functioning if we could reused moves generation
    for(auto single_movement:given_list){
        if(single_movement[2]!=0){board[single_movement[0]][single_movement[1]]=single_movement[2];}
    }

    ///evaluation starts
    //recording winning progess

    //we have my win
    //make a copy
    std::array<int,573> my_temp=my_stra_win;

    //we have enemy win
    //make a copy
    std::array<int,573> enemy_temp=enemy_stra_win;

    ///evaluate based on inserted moves
    //check winning array

    //modified the win array
    for(auto single_movement:given_list){
        //if valid move
        if(single_movement[2]!=0){
            for(int way=0;way<572;way++){
                if(winning_methods[single_movement[0]/*row*/][single_movement[1]/*col*/][way]){
                    //update my winning strategy
                    if(single_movement[2]==player){
                        if(my_temp[way]>=0){my_temp[way]++;}
                        enemy_temp[way]=-2;
                    }

                    //update enemy winning strategy
                    else if (single_movement[2]==3-player){
                        if(enemy_temp[way]>=0){enemy_temp[way]++;}
                        my_temp[way]=-2;
                    }
                }
            }

        }
    }

    //get score
    for(auto score:my_temp){
        if(score>0){
            //different values given to each winning
            if(score==2)     {player_mark+=1;}
            else if(score==3){player_mark+=100;}
            else if(score==4){player_mark+=100000;}
            else if(score==5){player_mark+=10000000;wrap_result.gameover=1;}
        }
    }

    //get score
    for(auto score:enemy_temp){
        if(score>0){
            //different values given to each winning
            if(score==2){player_mark-=2;}
            else if(score==3){player_mark-=200;}
            else if(score==4){player_mark-=200000;}
            else if(score==5){player_mark-=20000000;wrap_result.gameover=-1;}
        }
    }







    ///evaluation ends



//    //restore
//    for(auto single_movement:given_list){
//        if(single_movement[2]!=0){board[single_movement[0]][single_movement[1]]=EMPTY;}
//    }
    if(svf_value_check){my_log(encode_output());my_log("result mark: "+std::to_string(player_mark));}

    wrap_result.svf_value=player_mark;
    return wrap_result;
}

///writing result
void write_valid_spot(std::ofstream& fout) {
    srand(time(NULL));
    int init_depth=1;
    long long int best_value=std::numeric_limits<int>::max()*(-1);//best value
    int row=-1;//best row
    int col=-1;//best col

    //to secure we always make a valid move use random first
    ///come back later
//    int num_of_empty=empty_location.size();
//    int index=0;
//    if(num_of_empty>0){
//        index=rand()%num_of_empty;
//
//    }


    //if we start first
    if(occu_location.empty()){
        fout << 7 << " " << 7 << std::endl;
        fout.flush();
        return ;
    }


    while(true&&init_depth<=2){
        //call minimax with depth
        movement initial_dummy_move={-1,-1,0};
        list_of_movement initial_list;initial_list.push_back(initial_dummy_move);

        //get the best movement in current depth
        minimax_result to_choose_result=alphabeta(initial_list,init_depth,std::numeric_limits<int>::max()*(-1),std::numeric_limits<int>::max(),true);
        if(best_value<to_choose_result.minimax_value){
                best_value=to_choose_result.minimax_value;
                row=to_choose_result.chosen_movement[0];
                col=to_choose_result.chosen_movement[1];
        }

        //if the input is valid move
        if((row>=0)&&(col>=0)&&(row<SIZE)&&(col<SIZE)&&(board[row][col]==EMPTY)){
            //log
            if(LOG_ON){
                std::string log="made a move of row:"+std::to_string(row)+" col:"+std::to_string(col);
                my_log(log);
            }


            fout << row << " " << col << std::endl;
            fout.flush();
            std::cout<<"Depth "<<init_depth<<" search done\n";
            init_depth+=1;

        }

        else{
            //random
                int row_rand=rand()%SIZE;
                int col_rand=rand()%SIZE;
                if(board[row_rand][col_rand]==EMPTY){
                    fout << row_rand << " " << col_rand << std::endl;
                    fout.flush();

                }
        }

    }
    return ;
}

///minimax
minimax_result minimax(list_of_movement given_list, int depth, bool maximizingPlayer){
    if(MM_DEBUG){
        std::string log="depth "+std::to_string(depth)+" given movement row:"+std::to_string(given_list.back()[0])+" col:"+std::to_string(given_list.back()[1])+" type:"+std::to_string(given_list.back()[2]);

        if(maximizingPlayer){log+=" maximizing\n";my_log(log);}
        else{log+=" minimizing\n";my_log(log);}
    }

    //return value
    minimax_result value;

    //append and check for gameover
    //append
    for(auto single_movement:given_list){
        if(single_movement[2]!=0){board[single_movement[0]][single_movement[1]]=single_movement[2];}
    }

    //after modification check the board
    if(MM_DEBUG){my_log(encode_output());}

    //check
    svf_result svf_evaluation_result=state_value_function(given_list);
    int evaluation_result=svf_evaluation_result.svf_value;

    //base case
    if((depth ==0)||(svf_evaluation_result.gameover==-1)||(svf_evaluation_result.gameover==1)){//depth==0 or gameover
        //evaluate
        value.minimax_value=evaluation_result;
        value.chosen_movement=given_list.back();

        //return
        //restore
        for(auto single_movement:given_list){
            if(single_movement[2]!=0){board[single_movement[0]][single_movement[1]]=EMPTY;}
        }

        return value;
    }

    //recursive cases
    else{


        ///generate all the possible next moves //can be improved by reusing the result
        std::set<movement> next_move_set;
        //my_log("inserting for the movement "+string_movement(given_movement));
        if(empty_location.size()<occu_location.size()){//check if we have more empty of more occupied
            //make from empty location
            for(auto unit:empty_location){
                //unit is a 3 element array
                int x=unit[0];//row
                int y=unit[1];//col

                //check all around
                //upper left
                if(((x-1)>=0)&&(y-1>=0)&&(board[x-1][y-1]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    //my_log(string_movement(unit));
                    continue;
                }
                //up
                if(((x-1)>=0)&&(board[x-1][y]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    //my_log(string_movement(unit));
                    continue;

                }
                //upper right
                if(((x-1)>=0)&&(y+1<SIZE)&&(board[x-1][y+1]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    //my_log(string_movement(unit));
                    continue;
                }
                //left
                if((y-1>=0)&&(board[x][y-1]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    //my_log(string_movement(unit));
                    continue;
                }
                //right
                if((y+1<SIZE)&&(board[x][y+1]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    //my_log(string_movement(unit));
                    continue;
                }
                //lower left
                if(((x+1)>=0)&&(y-1>=0)&&(board[x+1][y-1]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    my_log(string_movement(unit));
                    continue;
                }
                //down
                if(((x+1)>=0)&&(board[x+1][y]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    //my_log(string_movement(unit));
                    continue;
                }
                //lower right
                if(((x+1)>=0)&&(y+1<SIZE)&&(board[x+1][y+1]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    //my_log(string_movement(unit));
                    continue;
                }
            }
        }
        else{
            //make from occu location
            for(auto unit:occu_location){
                //unit is a 3 element array
                int x=unit[0];//row
                int y=unit[1];//col

                //check all around
                //upper left
                if(((x-1)>=0)&&(y-1>=0)&&(board[x-1][y-1]==0)){
                    //insert and continuee
                    movement temp={x-1,y-1,0};
                    next_move_set.insert(temp);
                    //my_log("upper left");
                    //my_log(string_movement(temp));
                }
                //up
                if(((x-1)>=0)&&(board[x-1][y]==0)){
                    //insert and continue
                    movement temp={x-1,y,0};
                    next_move_set.insert(temp);
                    //my_log("up");
                    //my_log(string_movement(temp));

                }
                //upper right
                if(((x-1)>=0)&&(y+1<SIZE)&&(board[x-1][y+1]==0)){
                    //insert and continue
                    movement temp={x-1,y+1,0};
                    next_move_set.insert(temp);
                    //my_log("upper right");
                    //my_log(string_movement(temp));
                }
                //left
                if((y-1>=0)&&(board[x][y-1]==0)){
                    //insert and continue
                    movement temp={x,y-1,0};
                    next_move_set.insert(temp);
                    //my_log("left");
                    //my_log(string_movement(temp));
                }
                //right
                if((y+1<SIZE)&&(board[x][y+1]==0)){
                    //insert and continue
                    movement temp={x,y+1,0};
                    next_move_set.insert(temp);
                    //my_log("right");
                    //my_log(string_movement(temp));
                }
                //lower left
                if(((x+1)>=0)&&(y-1>=0)&&(board[x+1][y-1]==0)){
                    //insert and continue
                    movement temp={x+1,y-1,0};
                    next_move_set.insert(temp);
                    //my_log("lower left");
                    //my_log(string_movement(temp));
                }
                //down
                if(((x+1)>=0)&&(board[x+1][y]==0)){
                    //insert and continue
                    movement temp={x+1,y,0};
                    next_move_set.insert(temp);
                    //my_log("down");
                    //my_log(string_movement(temp));
                }
                //lower right
                if(((x+1)>=0)&&(y+1<SIZE)&&(board[x+1][y+1]==0)){
                    //insert and continue
                    movement temp={x+1,y+1,0};
                    next_move_set.insert(temp);
                    //my_log("lower right");
                    //my_log(string_movement(temp));
                }
            }
        }

        //the set of next moves printf
        std::string log;

        if(MM_DEBUG){
            log+="-------\n";
            for(auto log_test:next_move_set){
                log+=("row:"+std::to_string(log_test[0])+" col:"+std::to_string(log_test[1])+"\n");
            }
            log+="-------\n";
            my_log(log);
        }


        if (maximizingPlayer){

            value.minimax_value=std::numeric_limits<int>::max()*(-1);
            for(auto unit:next_move_set){
                //set to player
                unit[2]=player;

                //make the list to carry to the next layer of tree
                list_of_movement carry_on=given_list;carry_on.push_back(unit);

                //function call
                minimax_result temp_result=minimax(carry_on, depth - 1, false);
                //compare values
                if((value.minimax_value)<(temp_result.minimax_value)){
                    if(MM_DEBUG){
                        std::string log="original maximizing value max:";
                        log+=std::to_string(value.minimax_value);
                        my_log(log);
                    }
                    value.minimax_value=temp_result.minimax_value;
                    value.chosen_movement=temp_result.chosen_movement;
                    //log
                    if(MM_DEBUG){
                        std::string log="changed the maximizing value max:";

                        log+=std::to_string(value.minimax_value);
                        my_log(log);
                    }
                }
            }


            ///after exploring everything: restore the board
            //restore
            for(auto single_movement:given_list){
                if(single_movement[2]!=0){board[single_movement[0]][single_movement[1]]=EMPTY;}
            }
            return value;
        }

        else{ /* minimizing player */
            value.minimax_value=std::numeric_limits<int>::max();
            for(auto unit:next_move_set){
                //set to player
                unit[2]=3-player;

                //make the list to carry to the next layer of tree
                list_of_movement carry_on=given_list;carry_on.push_back(unit);

                //function call
                minimax_result temp_result=minimax(carry_on, depth - 1, true);

                //compare values
                if((value.minimax_value)>(temp_result.minimax_value)){
                    value.minimax_value=temp_result.minimax_value;
                    value.chosen_movement=temp_result.chosen_movement;
                }
            }

            ///after exploring everything: restore the board
            for(auto single_movement:given_list){
                if(single_movement[2]!=0){board[single_movement[0]][single_movement[1]]=EMPTY;}
            }
            return value;
        }
    }
}

///alpha beta pruning
minimax_result alphabeta(list_of_movement given_list, int depth,long long int alpha,long long int beta, bool maximizingPlayer){
    if(MM_DEBUG){
        std::string log="depth "+std::to_string(depth)+" given movement row:"+std::to_string(given_list.back()[0])+" col:"+std::to_string(given_list.back()[1])+" type:"+std::to_string(given_list.back()[2]);

        if(maximizingPlayer){log+=" maximizing\n";my_log(log);}
        else{log+=" minimizing\n";my_log(log);}
    }

    //return value
    minimax_result value;

    //append and check for gameover
    //append
    for(auto single_movement:given_list){
        if(single_movement[2]!=0){board[single_movement[0]][single_movement[1]]=single_movement[2];}
    }

    //after modification check the board
    if(MM_DEBUG){my_log(encode_output());}

    //check
    svf_result svf_evaluation_result=state_value_function(given_list);
    int evaluation_result=svf_evaluation_result.svf_value;

    //base case
    if((depth ==0)||(svf_evaluation_result.gameover==-1)||(svf_evaluation_result.gameover==1)){//depth==0 or gameover
        //evaluate
        value.minimax_value=evaluation_result;
        value.chosen_movement=given_list.back();

        //return
        //restore
        for(auto single_movement:given_list){
            if(single_movement[2]!=0){board[single_movement[0]][single_movement[1]]=EMPTY;}
        }

        return value;
    }

    //recursive cases
    else{


        ///generate all the possible next moves //can be improved by reusing the result
        std::set<movement> next_move_set;
        //my_log("inserting for the movement "+string_movement(given_movement));
        if(empty_location.size()<occu_location.size()){//check if we have more empty of more occupied
            //make from empty location
            for(auto unit:empty_location){
                //unit is a 3 element array
                int x=unit[0];//row
                int y=unit[1];//col

                //check all around
                //upper left
                if(((x-1)>=0)&&(y-1>=0)&&(board[x-1][y-1]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    //my_log(string_movement(unit));
                    continue;
                }
                //up
                if(((x-1)>=0)&&(board[x-1][y]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    //my_log(string_movement(unit));
                    continue;

                }
                //upper right
                if(((x-1)>=0)&&(y+1<SIZE)&&(board[x-1][y+1]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    //my_log(string_movement(unit));
                    continue;
                }
                //left
                if((y-1>=0)&&(board[x][y-1]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    //my_log(string_movement(unit));
                    continue;
                }
                //right
                if((y+1<SIZE)&&(board[x][y+1]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    //my_log(string_movement(unit));
                    continue;
                }
                //lower left
                if(((x+1)>=0)&&(y-1>=0)&&(board[x+1][y-1]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    my_log(string_movement(unit));
                    continue;
                }
                //down
                if(((x+1)>=0)&&(board[x+1][y]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    //my_log(string_movement(unit));
                    continue;
                }
                //lower right
                if(((x+1)>=0)&&(y+1<SIZE)&&(board[x+1][y+1]!=0)){
                    //insert and continue
                    next_move_set.insert(unit);
                    //my_log(string_movement(unit));
                    continue;
                }
            }
        }
        else{
            //make from occu location
            for(auto unit:occu_location){
                //unit is a 3 element array
                int x=unit[0];//row
                int y=unit[1];//col

                //check all around
                //upper left
                if(((x-1)>=0)&&(y-1>=0)&&(board[x-1][y-1]==0)){
                    //insert and continuee
                    movement temp={x-1,y-1,0};
                    next_move_set.insert(temp);
                    //my_log("upper left");
                    //my_log(string_movement(temp));
                }
                //up
                if(((x-1)>=0)&&(board[x-1][y]==0)){
                    //insert and continue
                    movement temp={x-1,y,0};
                    next_move_set.insert(temp);
                    //my_log("up");
                    //my_log(string_movement(temp));

                }
                //upper right
                if(((x-1)>=0)&&(y+1<SIZE)&&(board[x-1][y+1]==0)){
                    //insert and continue
                    movement temp={x-1,y+1,0};
                    next_move_set.insert(temp);
                    //my_log("upper right");
                    //my_log(string_movement(temp));
                }
                //left
                if((y-1>=0)&&(board[x][y-1]==0)){
                    //insert and continue
                    movement temp={x,y-1,0};
                    next_move_set.insert(temp);
                    //my_log("left");
                    //my_log(string_movement(temp));
                }
                //right
                if((y+1<SIZE)&&(board[x][y+1]==0)){
                    //insert and continue
                    movement temp={x,y+1,0};
                    next_move_set.insert(temp);
                    //my_log("right");
                    //my_log(string_movement(temp));
                }
                //lower left
                if(((x+1)>=0)&&(y-1>=0)&&(board[x+1][y-1]==0)){
                    //insert and continue
                    movement temp={x+1,y-1,0};
                    next_move_set.insert(temp);
                    //my_log("lower left");
                    //my_log(string_movement(temp));
                }
                //down
                if(((x+1)>=0)&&(board[x+1][y]==0)){
                    //insert and continue
                    movement temp={x+1,y,0};
                    next_move_set.insert(temp);
                    //my_log("down");
                    //my_log(string_movement(temp));
                }
                //lower right
                if(((x+1)>=0)&&(y+1<SIZE)&&(board[x+1][y+1]==0)){
                    //insert and continue
                    movement temp={x+1,y+1,0};
                    next_move_set.insert(temp);
                    //my_log("lower right");
                    //my_log(string_movement(temp));
                }
            }
        }

        //the set of next moves printf
        std::string log;

        if(MM_DEBUG){
            log+="-------\n";
            for(auto log_test:next_move_set){
                log+=("row:"+std::to_string(log_test[0])+" col:"+std::to_string(log_test[1])+"\n");
            }
            log+="-------\n";
            my_log(log);
        }


        if (maximizingPlayer){

            value.minimax_value=std::numeric_limits<int>::max()*(-1);
            //for every child node
            for(auto unit:next_move_set){
                //set to player
                unit[2]=player;

                //make the list to carry to the next layer of tree
                list_of_movement carry_on=given_list;carry_on.push_back(unit);

                //function call
                minimax_result temp_result=minimax(carry_on, depth - 1, false);
                //compare values
                if((value.minimax_value)<(temp_result.minimax_value)){
                    if(MM_DEBUG){
                        std::string log="original maximizing value max:";
                        log+=std::to_string(value.minimax_value);
                        my_log(log);
                    }
                    value.minimax_value=temp_result.minimax_value;
                    value.chosen_movement=temp_result.chosen_movement;
                    //log
                    if(MM_DEBUG){
                        std::string log="changed the maximizing value max:";

                        log+=std::to_string(value.minimax_value);
                        my_log(log);
                    }
                }

                //alpha beta pruning extension
                alpha=std::max(alpha,value.minimax_value);
                if (value.minimax_value>=beta){
                    break; //beta cutoff
                }

            }


            ///after exploring everything: restore the board
            //restore
            for(auto single_movement:given_list){
                if(single_movement[2]!=0){board[single_movement[0]][single_movement[1]]=EMPTY;}
            }
            return value;
        }

        else{ /* minimizing player */
            value.minimax_value=std::numeric_limits<int>::max();
            //for every child node
            for(auto unit:next_move_set){
                //set to player
                unit[2]=3-player;

                //make the list to carry to the next layer of tree
                list_of_movement carry_on=given_list;carry_on.push_back(unit);

                //function call
                minimax_result temp_result=minimax(carry_on, depth - 1, true);

                //compare values
                if((value.minimax_value)>(temp_result.minimax_value)){
                    value.minimax_value=temp_result.minimax_value;
                    value.chosen_movement=temp_result.chosen_movement;
                }

                beta =std::min(beta, value.minimax_value);
                if (value.minimax_value<=alpha){
                    break; //alpha cutoff
                }
            }

            ///after exploring everything: restore the board
            for(auto single_movement:given_list){
                if(single_movement[2]!=0){board[single_movement[0]][single_movement[1]]=EMPTY;}
            }
            return value;
        }
    }
}





int main(int, char** argv) {

    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    //win array generate
    win_array_gen();


    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
