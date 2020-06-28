#pragma once

#include <UltraOOXX/Wrapper/AI.h>
#include <UltraOOXX/UltraBoard.h>
#include <algorithm>
#include <random>
#include <ctime>

class AI : public AIInterface
{
    // the pair store the previous step enemy move
    std::pair<int,int> enemy_Move;
    bool playFirst; // true: first | false: second

    int INF; // playSecond
    int MAX_height;

    int playFirst_count; // used to count the moves we do when we play first
    bool playFirst_array[3][3]; // used to memorize if the subboard has 'O' when we play first
    std::pair<int,int> playFirst_restrict; // used to memorize the subboard we catch enemy
    std::pair<int,int> playFirst_secondary_restrict; /* if we can not catch enemy in "playFirst_restrict"
                                                        we catch enemy in this subboard*/
public:
    void init(bool order) override
    {
        playFirst = order;

        // playFirst initialize
        playFirst_count = 0;
        int i, j;
        for (i=0; i<3; ++i){
            for (j=0; j<3; ++j){
                playFirst_array[i][j] = false;
            }
        }

        // playSecond initialize
        INF = 1000;
        MAX_height = 2;
    }

    void callbackReportEnemy(int x, int y) override
    {
        //(void) x;
        //(void) y;
        // give last step
        enemy_Move.first = x;
        enemy_Move.second = y;
    }

    std::pair<int,int> queryWhereToPut(TA::UltraBoard ultraboard) override
    {
        if (playFirst){
            ++playFirst_count;
            return the_initiative(ultraboard);
        }
        else{
            int index[2] = {0, 0};
            alpha_beta_algorithm(ultraboard, enemy_Move, MAX_height, -INF, INF, true, index);
            return std::pair<int,int>(index[0], index[1]);
        }     
    }

    /********************/
    /*****PLAY FIRST*****/
    /********************/

    // This algorithm is used when we move first.
    std::pair<int,int> the_initiative(TA::UltraBoard ultraboard){
        if (playFirst_count == 1){
            playFirst_array[1][1] = true;
            return std::pair<int,int>(4, 4);
        }
        else if (playFirst_count >= 2 && playFirst_count <= 8){
            playFirst_array[enemy_Move.first%3][enemy_Move.second%3] = true;
            return std::pair<int,int>((enemy_Move.first%3)*3+1, (enemy_Move.second%3)*3+1);
        }
        else if (playFirst_count == 9){
            int i, j;
            for (i=0; i<3; ++i){
                for (j=0; j<3; ++j){
                    if (!playFirst_array[i][j]){
                        playFirst_restrict = std::pair<int,int>(i, j);
                        playFirst_secondary_restrict = std::pair<int,int>(2-i, 2-j);
                        return std::pair<int,int>(i*4, j*4);
                    }
                }
            }
            // default return
            return std::pair<int,int>(0, 0);
        }
        else{
            // Suppose the previous enemy step is (i,j)
            // ((i%3)*3, (j%3)*3) is the upper left corner of the subboard we restrict to move
            std::pair<int,int> target = std::pair<int,int>((enemy_Move.first%3) * 3 + playFirst_restrict.first, (enemy_Move.second%3) * 3 + playFirst_restrict.second);
            if (ultraboard.sub(playFirst_restrict.first, playFirst_restrict.second).full() == false &&
                ultraboard.get(target.first, target.second) == TA::BoardInterface::Tag::None)
            {
                return target;
            }
            else{
                if (enemy_Move.first%3 == 1 && enemy_Move.second%3 == 1){
                    target = std::pair<int,int>(playFirst_secondary_restrict.first*3+playFirst_restrict.first, playFirst_secondary_restrict.second*3+playFirst_restrict.second);
                    if (ultraboard.get(target.first, target.second) == TA::BoardInterface::Tag::None) return target; // restrict to playFirst_restrict
                    else return std::pair<int,int>(playFirst_secondary_restrict.first*4, playFirst_secondary_restrict.second*4); // restrict to playFirst_secondary_restrict
                }
                return std::pair<int,int>((enemy_Move.first%3) * 3 + playFirst_secondary_restrict.first, (enemy_Move.second%3) * 3 + playFirst_secondary_restrict.second);
            }
        }
    }

    /*********************/
    /*****PLAY SECOND*****/
    /*********************/


    // ultraboard is the state of current board
    // Tag T is the player tag at that level
    // height denotes the tree height we want
    // index[] store the move we choose
    // pre_move store the previous enemy move
    int alpha_beta_algorithm(TA::UltraBoard ultraboard, std::pair<int,int> pre_move, int height, int alpha, int beta, bool IsMaxLevel, int index[]){
        int eval;

        if (height == 0){
            return evaluate(ultraboard);
        }
        TA::BoardInterface::Tag pre_subboard_winTag;
        TA::BoardInterface::Tag pre_ultraboard_winTag;
        TA::Board subboard;
        int i, j;
        if (IsMaxLevel){
            int _max = -INF;
            if (ultraboard.sub(pre_move.first%3, pre_move.second%3).full()){ // we can place anywhere
                // We need to select where to put. There are too many possibilities
                for (i=0; i<9; ++i){
                    for (j=0; j<9; ++j){
                        if (ultraboard.get(i, j) == TA::BoardInterface::Tag::None){
                            pre_subboard_winTag = ultraboard.sub(i/3, j/3).getWinTag();
                            pre_ultraboard_winTag = ultraboard.getWinTag();

                            ultraboard.get(i, j) = TA::BoardInterface::Tag::X;                            
                            checkPlayerWin_(TA::BoardInterface::Tag::X, ultraboard.sub(i/3, j/3));
                            checkPlayerWin_(TA::BoardInterface::Tag::X, ultraboard);

                            if (ultraboard.getWinTag() == TA::BoardInterface::Tag::X) eval = INF;
                            else eval = alpha_beta_algorithm(ultraboard, std::pair<int,int>(i, j), height-1, alpha, beta, false, index);

                            ultraboard.get(i, j) = TA::BoardInterface::Tag::None;
                            ultraboard.sub(i/3, j/3).setWinTag(pre_subboard_winTag);
                            ultraboard.setWinTag(pre_ultraboard_winTag);

                            if (eval > _max){
                                _max = eval;
                                if (height == MAX_height){
                                    index[0] = i;
                                    index[1] = j;
                                }
                            }
                            alpha = std::max(alpha, _max);
                            if (alpha >= beta) break;
                        }
                    }
                    if (j < 9) break;
                }
            }
            else{ // we only can place a move in corresponding subboard
                subboard = ultraboard.sub(pre_move.first%3, pre_move.second%3);
                for (i=0; i<3; ++i){
                    for (j=0; j<3; ++j){
                        if (subboard.state(i, j) == TA::BoardInterface::Tag::None)
                        {
                            pre_subboard_winTag = subboard.getWinTag();
                            pre_ultraboard_winTag = ultraboard.getWinTag();

                            subboard.get(i, j) = TA::BoardInterface::Tag::X;                            
                            checkPlayerWin_(TA::BoardInterface::Tag::X, subboard);
                            checkPlayerWin_(TA::BoardInterface::Tag::X, ultraboard);

                            if (ultraboard.getWinTag() == TA::BoardInterface::Tag::X) eval = INF;
                            else eval = alpha_beta_algorithm(ultraboard, std::pair<int,int>((pre_move.first%3*3)+i, (pre_move.second%3)*3+j), height-1, alpha, beta, false, index);

                            subboard.get(i, j) = TA::BoardInterface::Tag::None;
                            subboard.setWinTag(pre_subboard_winTag);
                            ultraboard.setWinTag(pre_ultraboard_winTag);

                            if (eval > _max){
                                _max = eval;
                                if (height == MAX_height){
                                    index[0] = (pre_move.first%3)*3 + i;
                                    index[1] = (pre_move.second%3)*3 + j;
                                }
                            }
                            alpha = std::max(alpha, _max);
                            if (alpha >= beta) break;
                        }
                    }
                    if (j < 3) break;
                }
            }
            return _max;
        }
        else{
            int _min = INF;
            if (ultraboard.sub(pre_move.first%3, pre_move.second%3).full()){
                for (i=0; i<9; ++i){
                    for (j=0; j<9; ++j){
                        if (ultraboard.get(i, j) == TA::BoardInterface::Tag::None){
                            pre_subboard_winTag = ultraboard.sub(i/3, j/3).getWinTag();
                            pre_ultraboard_winTag = ultraboard.getWinTag();

                            ultraboard.get(i, j) = TA::BoardInterface::Tag::O;                            
                            checkPlayerWin_(TA::BoardInterface::Tag::O, ultraboard.sub(i/3, j/3));
                            checkPlayerWin_(TA::BoardInterface::Tag::O, ultraboard);

                            if (ultraboard.getWinTag() == TA::BoardInterface::Tag::O) eval = -INF;
                            else eval = alpha_beta_algorithm(ultraboard, std::pair<int,int>(i, j), height-1, alpha, beta, true, index);

                            ultraboard.get(i, j) = TA::BoardInterface::Tag::None;
                            ultraboard.sub(i/3, j/3).setWinTag(pre_subboard_winTag);
                            ultraboard.setWinTag(pre_ultraboard_winTag);

                            _min = std::min(_min, eval);
                            beta = std::min(beta, _min);
                            if (alpha >= beta) break;
                        }
                    }
                    if (j < 9) break;
                }
            }
            else{
                subboard = ultraboard.sub(pre_move.first%3, pre_move.second%3);
                for (i=0; i<3; ++i){
                    for (j=0; j<3; ++j){
                        if (subboard.state(i, j) == TA::BoardInterface::Tag::None)
                        {
                            pre_subboard_winTag = subboard.getWinTag();
                            pre_ultraboard_winTag = ultraboard.getWinTag();

                            subboard.get(i, j) = TA::BoardInterface::Tag::O;                            
                            checkPlayerWin_(TA::BoardInterface::Tag::O, subboard);
                            checkPlayerWin_(TA::BoardInterface::Tag::O, ultraboard);

                            if (ultraboard.getWinTag() == TA::BoardInterface::Tag::O) eval = -INF;
                            else eval = alpha_beta_algorithm(ultraboard, std::pair<int,int>((pre_move.first%3)*3+i, (pre_move.second%3)*3+j), height-1, alpha, beta, true, index);

                            subboard.get(i, j) = TA::BoardInterface::Tag::None;
                            subboard.setWinTag(pre_subboard_winTag);
                            ultraboard.setWinTag(pre_ultraboard_winTag);

                            _min = std::min(_min, eval);
                            beta = std::min(beta, _min);
                            if (alpha >= beta) break;
                        }
                    }
                    if (j < 3) break;
                }
            }
            return _min;
        }
    }

   // Note we can not use the function in Game.h. We copy one here.
    void checkPlayerWin_(TA::BoardInterface::Tag T, TA::BoardInterface& board){
        if (board.getWinTag() != TA::BoardInterface::Tag::None) return;
            
        int i, j;

        // check row
        for (i=0; i<3; ++i){
            for (j=0; j<3; ++j){
                if (board.state(i, j) != T) break;
                if (j == 2){ // if j == 2, means state(i, j) == T for j = 0, 1, 2 since there is no break
                    board.setWinTag(T); 
                    return;
                }
            }
        }

        // check column
        for (i=0; i<3; ++i){
            for (j=0; j<3; ++j){
                if (board.state(j, i) != T) break;
                if (j == 2){ // if j == 2, means state(j, i) == T for j = 0, 1, 2 since there is no break
                    board.setWinTag(T);
                    return;                        
                }
            }
        }

        // check diagonal '\'
        for (i=0; i<3; ++i){
            if (board.state(i, i) != T) break;
            if (i == 2){ // if i == 2, means state(i, i) == T for i = 0, 1, 2 since there is no break;
                board.setWinTag(T);
                return; 
            }
        }

        // check diagonal '/'
        for (i=0; i<3; ++i){
            if (board.state(i, 2-i) != T) break;
            if (i == 2){ // if i == 2, means state(i, 2-i) == T for i = 0, 1, 2 since there is no break;
                board.setWinTag(T);
                return;
            }
        }

        // no Win
        return;
    }

    int evaluate(TA::UltraBoard &B){
        
        int value = 0;
        for(int i = 0 ; i < 3; i++){
            for(int j = 0 ; j < 3; j++){
                
                //The ultra wintag 
                if( B.sub(i,j).getWinTag() == TA::BoardInterface::Tag::X){
                    if( i == 1 && j == 1 ){
                        value += 10;
                    }
                    else value += 5;
                }
                else if( B.sub(i,j).getWinTag() == TA::BoardInterface::Tag::O){
                    if( i == 1 && j == 1 ){
                        value -= 10;
                    }
                    else value -= 5;
                }
                else{
                    
                    if( B.sub(i,j).get(1,1) == TA::BoardInterface::Tag::X ){
                        value += 2;
                    }
                    if( B.sub(i,j).get(1,1) == TA::BoardInterface::Tag::O ){
                        value -= 2;
                    }    

                }
                
            }
        }
    return value;
    }
};
