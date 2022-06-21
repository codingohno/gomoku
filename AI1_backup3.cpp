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
std::array <std::array<std::array<bool,5>,SIZE>,SIZE>winning_methods;


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

std::string string_movement (std::array<int,3>a_movement){
    std::string ret_str;
    ret_str="row:"+std::to_string(a_movement[0])+" col:"+std::to_string(a_movement[0])+" type:"+std::to_string(a_movement[2]);
    return ret_str;
}

//structs
//for minimax
typedef std::array<int,3> movement;//x,y,by who
typedef struct minimax_result_{
    int long long minimax_value;
    movement chosen_movement;
}minimax_result;

//declaration of functions
minimax_result minimax(movement given_movement, int depth, bool maximizingPlayer);
void win_array_gen();
long long int state_value_function(state check_state);
void write_valid_spot(std::ofstream& fout);

///generate winninng array
void win_array_gen(){
    int count=0;
    //horizontal wins
    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 11; j++) {
            for (int k = 0; k < 5; k++) {
                winning_methods[i][j+k][count] = true;
            }
            count++;
        }
    }

    //vertical wins
    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 15; j++) {
            for (int k = 0; k < 5; k++) {
                winning_methods[i+k][j][count] = true;
            }
            count++;
        }
    }

    //ltr wins
    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {
            for (int k = 0; k < 5; k++) {
                winning_methods[i + k][j + k][count] = true;
            }
            count++;
        }
    }

    //rtl wins
    for (int i = 14; i >=4; i--) {
        for (int j = 0; j < 11; j++) {
            for (int k = 0; k < 5; k++) {
                winning_methods[i - k][j + k][count] = true;
            }
            count++;
        }
    }

    my_log("we have wininig method of "+std::to_string(count));
    return ;
}

///state value func
long long int state_value_function(state check_state){
    //count our own
    //4 data
    std::array<std::array<int, 2>,SIZE> row;row.fill({});//number and type
    std::array<std::array<int, 2>,SIZE> col={0};col.fill({});
    std::array<std::array<int, 2>,2*SIZE-1> LTR={0};LTR.fill({});//indexing:col-row+14: 0,1,2,...,28
    std::array<std::array<int, 2>,2*SIZE-1> RTL={0};RTL.fill({});//indexing:col+row:0,1,2,...,28

    long long int player_mark=0;

    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){


            // row checking
            if(check_state[i][j]==row[i][1]/*type data is stored at second*/){
                //my_log("helloosdf");
                row[i][0]+=1;
            } else{
                    if((row[i][1]==player)&&(row[i][0]>=3)){//original is player
                        //type one doesn't emphasize 5 in a row
                        if(row[i][0]==5){return 1000000}
                        if(row[i][0]==4){
                            //check for
                            //oxxxxo
                            if((i-5>=0)/*in range*/&&(check_state[i][j]==3-player)&&(check_state[i-5][j]==3-player)){
                                //no value for you
                                player_mark+=0;
                            }
                            //0XXXX.
                            //.XXXXO
                            //one side not in range another side is empty
                            //-XXXX.
                            else if(((i-5<0)&&(check_state[i][j]==EMPTY))||((i-5>=0)/*in range*/&&/*one open end*/(((check_state[i][j]==3-player)&&(check_state[i-5][j]==EMPTY))||((check_state[i][j]==EMPTY)&&(check_state[i-5][j]==3-player))))){
                                //value
                                player_mark+=10000;
                            }
                            else if((i-5>=0)&&(check_state[i-5][j]==EMPTY)&&(check_state[i][j]==EMPTY)){
                                player_mark+=200000;
                            }
                        }
                        if(row[i][0]==3){
                            //check for
                            //oxxxo oxxx.o o.xxxo
                            if((i-4>=0)/*in range*/&&(check_state[i][j]==3-player)&&(check_state[i-4][j]==3-player))||{
                                //no value for you
                                player_mark+=0;
                            }
                            //0XXX..
                            //0XXX.
                            //.XXXXO
                            //one side not in range another side is empty
                            //-XXXX.
                            else if(((i-5<0)&&(check_state[i][j]==EMPTY))||((i-5>=0)/*in range*/&&/*one open end*/(((check_state[i][j]==3-player)&&(check_state[i-5][j]==EMPTY))||((check_state[i][j]==EMPTY)&&(check_state[i-5][j]==3-player))))){
                                //value
                                player_mark+=10000;
                            }
                            if((i-5>=0)&&(check_state[i-5][j]==EMPTY)&&(check_state[i][j]==EMPTY)){
                                player_mark+=200000;
                            }

                        }

                        player_mark+=row[i][0]-2;

                        //type two emphasize 5 in a row
//                        if(row[i][0]!=5){player_mark+=row[i][0]-2;}//formula for marks :3->1 mark,4->2 mark
//                        else{player_mark=std::numeric_limits<long long int>::max();}
                    }
                    else if((row[i][1]==(3-player))&&(row[i][0]>=4)){//original is enemy
                        if(row[i][0]==5){
                            if(SVF_TOGGLE){
                                my_log("enemy going to win");
                            }
                            return std::numeric_limits<int>::max()*(-1);
                        }

                        player_mark-=(row[i][0]-3);
                    }

                //reset
                row[i][0]=1;
                row[i][1]=check_state[i][j];
            }

            // col checking
            if(check_state[i][j]==col[j][1]/*type data is stored at second*/){
                col[j][0]+=1;
            } else{
                    if((col[j][1]==player)&&(col[j][0]>=3)){//original is player
                        if(col[j][0]==5){return std::numeric_limits<int>::max();}
                        //type one doesn't emphasize 5 in a col
                        player_mark+=col[j][0]-2;
                    }
                    else if((col[j][1]==(3-player))&&(col[j][0]>=4)){//original is enemy
                        if(col[j][0]==5){return std::numeric_limits<int>::max()*(-1);}
                        player_mark-=(col[j][0]-3);
                    }

                //reset
                col[j][0]=1;
                col[j][1]=check_state[i][j];
            }


            //LTR checking
            if(check_state[i][j]==LTR[j-i+14][1]/*type data is stored at second*/){
                LTR[j-i+14][0]+=1;
            } else{
                    if((LTR[j-i+14][1]==player)&&(LTR[j-i+14][0]>=3)){//original is player
                        if(LTR[j-i+14][0]==5){return std::numeric_limits<int>::max();}
                        //type one doesn't emphasize 5 in a col
                        player_mark+=LTR[j-i+14][0]-2;
                    }
                    else if((LTR[j-i+14][1]==(3-player))&&(LTR[j-i+14][0]>=4)){//original is enemy
                        if(LTR[j-i+14][0]==5){return std::numeric_limits<int>::max()*(-1);}
                        player_mark-=(LTR[j-i+14][0]-3);
                    }

                //reset
                LTR[j-i+14][0]=1;
                LTR[j-i+14][1]=check_state[i][j];
            }

            //RTL checking
            if(check_state[i][j]==RTL[i+j][1]/*type data is stored at second*/){
                RTL[i+j][0]+=1;
            } else{
                    if((RTL[i+j][1]==player)&&(RTL[i+j][0]>=3)){//original is player
                        if(RTL[i+j][0]==5){return std::numeric_limits<int>::max();}
                        //type one doesn't emphasize 5 in a col
                        player_mark+=RTL[i+j][0]-2;
                    }
                    else if((RTL[i+j][1]==(3-player))&&(RTL[i+j][0]>=4)){//original is enemy
                        if(RTL[i+j][0]==5){return std::numeric_limits<int>::max()*(-1);}
                        player_mark-=(RTL[i+j][0]-3);
                    }

                //reset
                RTL[i+j][0]=1;
                RTL[i+j][1]=check_state[i][j];
            }
        }
    }

    ///end call
    //for col and row
    for(int i=0;i<SIZE;i++){
        //for row
        if((row[i][1]==player)&&(row[i][0]>=3)){//if player
            if(row[i][0]==5){return std::numeric_limits<int>::max();}
            player_mark+=row[i][0]-2;
        }
        else if((row[i][1]==3-player)&&(row[i][0]>=4)){//if enemy
            if(row[i][0]==5){return std::numeric_limits<int>::max()*(-1);}
            player_mark-=(row[i][0]-3);
        }

        //for col
        if((col[i][1]==player)&&(col[i][0]>=3)){//if player
            if(col[i][0]==5){return std::numeric_limits<int>::max();}
            player_mark+=col[i][0]-2;
        }
        else if((col[i][1]==3-player)&&(col[i][0]>=4)){//if enemy
            if(col[i][0]==5){return std::numeric_limits<int>::max()*(-1);}
            player_mark-=(col[i][0]-3);
        }
    }

    //for ltr and rtl
    for(int i=0;i<2*SIZE-1;i++){
        //for ltr
        if((LTR[i][1]==player)&&(LTR[i][0]>=3)){//if player
            if(LTR[i][0]==5){return std::numeric_limits<int>::max();}
            player_mark+=LTR[i][0]-2;
        }
        else if((LTR[i][1]==3-player)&&(LTR[i][0]>=4)){//if enemy
            if(LTR[i][0]==5){return std::numeric_limits<int>::max()*(-1);}
            player_mark-=(LTR[i][0]-3);
        }

        //for rtl
        if((RTL[i][1]==player)&&(RTL[i][0]>=3)){//if player
            if(RTL[i][0]==5){return std::numeric_limits<int>::max();}
            player_mark+=RTL[i][0]-2;
        }
        else if((RTL[i][1]==3-player)&&(RTL[i][0]>=4)){//if enemy
            if(RTL[i][0]==5){return std::numeric_limits<int>::max()*(-1);}
            player_mark-=(RTL[i][0]-3);
        }
    }

    //print board and show the score
    //my_log(encode_output()+"\n the score is:" +std::to_string(player_mark)+"\n");





    return player_mark;
}

///writing result
void write_valid_spot(std::ofstream& fout) {
    srand(time(NULL));
    int init_depth=1;
    long long int best_value=std::numeric_limits<int>::max()*(-1);//best value
    int row=-1;//best row
    int col=-1;//best col





    while(true){
        //call minimax with depth
        movement initial_dummy_move={-1,-1,0};
        //get the best movement in current depth
        minimax_result to_choose_result=minimax(initial_dummy_move,init_depth,true);
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
minimax_result minimax(movement given_movement, int depth, bool maximizingPlayer){
    if(MM_DEBUG){
        std::string log="depth "+std::to_string(depth)+" given movement row:"+std::to_string(given_movement[0])+" col:"+std::to_string(given_movement[1])+" type:"+std::to_string(given_movement[2]);

        if(maximizingPlayer){log+=" maximizing\n";my_log(log);}
        else{log+=" minimizing\n";my_log(log);}
    }

    //return value
    minimax_result value;

    //append and check for gameover
    //append
    if(given_movement[2]!=0){board[given_movement[0]][given_movement[1]]=given_movement[2];}
    //after modification check the board
    if(MM_DEBUG){my_log(encode_output());}

    //check
    int evaluation_result=state_value_function(board);

    //base case
    if((depth ==0)||(evaluation_result==std::numeric_limits<int>::max()*(-1))||(evaluation_result==std::numeric_limits<int>::max())){//depth==0 or gameover
        //evaluate
        value.minimax_value=evaluation_result;
        value.chosen_movement=given_movement;

        //return
        if(given_movement[2]!=0){board[given_movement[0]][given_movement[1]]=EMPTY;}
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

                //function call
                minimax_result temp_result=minimax(unit, depth - 1, false);
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
            if(given_movement[2]!=0){board[given_movement[0]][given_movement[1]]=EMPTY;}
            return value;
        }

        else{ /* minimizing player */
            value.minimax_value=std::numeric_limits<int>::max();
            for(auto unit:next_move_set){
                //set to player
                unit[2]=3-player;

                //function call
                minimax_result temp_result=minimax(unit, depth - 1, true);
                //compare values
                if((value.minimax_value)>(temp_result.minimax_value)){
                    value.minimax_value=temp_result.minimax_value;
                    value.chosen_movement=temp_result.chosen_movement;
                }
            }

            ///after exploring everything: restore the board
            if(given_movement[2]!=0){board[given_movement[0]][given_movement[1]]=EMPTY;}
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
