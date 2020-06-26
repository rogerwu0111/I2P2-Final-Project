#pragma once

#include <UltraOOXX/Wrapper/AI.h>
#include <UltraOOXX/UltraBoard.h>
#include <algorithm>
#include <random>
#include <ctime>

class AI : public AIInterface
{
    // these two variables store the previous step enemy move
    int enemy_x;
    int enemy_y;
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
        enemy_x = x;
        enemy_y = y;
    }

    std::pair<int,int> queryWhereToPut(TA::UltraBoard) override
    {
        return std::make_pair(1,2);
    }
};
