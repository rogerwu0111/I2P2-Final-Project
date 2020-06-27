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

    int playFirst_count; // used to count the moves we do when we play first
    bool playFirst_array[3][3]; // used to memorize if the subboard has 'O' when we play first
    std::pair<int,int> playFirst_restrict; // used to memorize the subboard we catch enemy
    std::pair<int,int> playFirst_secondary_restrict; /* if we can not catch enemy in "playFirst_restrict"
                                                        we catch enemy in this subboard*/
public:
    void init(bool order) override
    {
        playFirst_count = 1;
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
                // restrict to secondary subboard
                return std::pair<int,int>((enemy_Move.first%3) * 3 + playFirst_secondary_restrict.first, (enemy_Move.second%3) * 3 + playFirst_secondary_restrict.second);
            }
        }
    }
};
