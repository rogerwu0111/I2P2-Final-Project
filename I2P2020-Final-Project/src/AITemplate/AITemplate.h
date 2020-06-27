#pragma once

#include <UltraOOXX/Wrapper/AI.h>
#include <UltraOOXX/UltraBoard.h>
#include <algorithm>
#include <random>
#include <ctime>

#define INFINITY 99999999

class AI : public AIInterface
{
    // these two variables store the previous step enemy move
    std::pair<int,int> enemy_Move;

    TA::BoardInterface::Tag my_tag;
    TA::BoardInterface::Tag enemy_tag;
public:
    void init(bool order) override
    {
        // any way
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
        
    }

    // ultraboard is the state of current board
    // Tag T is the player tag at that level
    // height denotes the tree height we want
    // index[] store the move we choose
    // pre_move store the previous enemy move
    int alpha_beta_algorithm(TA::UltraBoard ultraboard, std::pair<int,int> pre_move, int height, int alpha, int beta, bool IsMaxLevel, int index[]){
        int eval;

        if (height == 0){
            // evaluate funtion
            //...
            // return ...
        }
        
        int i, j;
        if (IsMaxLevel){
            int _max = -INFINITY;
            if (ultraboard.sub(pre_move.first%3, pre_move.second%3).full()){ // we can place anywhere
                // We need to select where to put. There are too many possibilities
                for (i=0; i<9; ++i){
                    for (j=0; j<9; ++j){
                        //...
                        // some selection
                        // recursion call
                    }
                }
            }
            else{ // we only can place a move in corresponding subboard
                TA::Board subboard = ultraboard.sub(pre_move.first%3, pre_move.second%3);
                for (i=0; i<3; ++i){
                    for (j=0; j<3; ++j){
                        if (subboard.state(i, j) == TA::BoardInterface::Tag::None)
                        {
                            TA::BoardInterface::Tag pre_subboard_winTag = subboard.getWinTag();
                            TA::BoardInterface::Tag pre_ultraboard_winTag = ultraboard.getWinTag();

                            subboard.get(i, j) = enemy_tag;                            
                            checkPlayerWin_(my_tag, subboard);
                            checkPlayerWin_(my_tag, ultraboard);
                            if (ultraboard.getWinTag() == my_tag) eval = INFINITY;
                            //...
                        }
                    }
                }
            }
            return _max;
        }
        else{

        }
    }

    // Note we can not use the function in Game.h. We copy one here.
    bool checkPlayerWin_(TA::BoardInterface::Tag T, TA::BoardInterface& board){ // Todo(finish)
        int i, j;

        // check row
        for (i=0; i<3; ++i){
            for (j=0; j<3; ++j){
                if (board.state(i, j) != T) break;
                if (j == 2){ // if j == 2, means state(i, j) == T for j = 0, 1, 2 since there is no break
                    board.setWinTag(T); 
                    return true;
                }
            }
        }

        // check column
        for (i=0; i<3; ++i){
            for (j=0; j<3; ++j){
                if (board.state(j, i) != T) break;
                if (j == 2){ // if j == 2, means state(j, i) == T for j = 0, 1, 2 since there is no break
                    board.setWinTag(T);
                    return true;                        
                }
            }
        }

        // check diagonal '\'
        for (i=0; i<3; ++i){
            if (board.state(i, i) != T) break;
            if (i == 2){ // if i == 2, means state(i, i) == T for i = 0, 1, 2 since there is no break;
                board.setWinTag(T);
                return true; 
            }
        }

        // check diagonal '/'
        for (i=0; i<3; ++i){
            if (board.state(i, 2-i) != T) break;
            if (i == 2){ // if i == 2, means state(i, 2-i) == T for i = 0, 1, 2 since there is no break;
                board.setWinTag(T);
                return true;
            }
        }

        // no Win
        return false;
    }    
};
