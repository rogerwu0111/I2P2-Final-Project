#pragma once

#include <UltraOOXX/UltraBoard.h>
#include <UltraOOXX/Wrapper/AI.h>
#include <GUI/GUIInterface.h>

#include <iostream>
#include <cassert>
#include <chrono>
#include <cstdarg>
#include <future>
#include <type_traits>

namespace TA
{
    class UltraOOXX
    {
    public:
        UltraOOXX(
            std::chrono::milliseconds runtime_limit = std::chrono::milliseconds(1000)
        ):
            m_runtime_limit(runtime_limit),
            m_P1(nullptr),
            m_P2(nullptr),
            MainBoard()
        {
            gui = new ASCII;
        }

        void setPlayer1(AIInterface *ptr) { assert(checkAI(ptr)); m_P1 = ptr; }
        void setPlayer2(AIInterface *ptr) { assert(checkAI(ptr)); m_P2 = ptr; }

        void run()
        {
            gui->title();
            int round = 0;
            if( !prepareState() ) return ; // run m_P1 init and m_P2 init

            //Todo: Play Game
            putToGui("Start the Game!!\n");
            updateGuiGame();

            while (!checkGameover()) { // play the game until game over 
                round++;
                
                // Note m_P1 first and m_P2 secoond;
                // Note m_P1 take 'O', m_P2 take 'X'
                AIInterface *first;
                AIInterface *second;               
                BoardInterface::Tag tag;
                if (round%2){
                    first = m_P1;
                    second = m_P2;
                    tag = BoardInterface::Tag::O;
                }
                else{
                    first = m_P2;
                    second = m_P1;
                    tag = BoardInterface::Tag::X;
                }

                if (!playOneRound(first, tag, second)) { // if playOneRound is false, means the "first" player take a illegal move
                    // one player is lose. show result
                    if (first == m_P1) putToGui("player 2 win\n");
                    else if (first == m_P2) putToGui("player 1 win\n");
                    else{}
                    return;
                }
                updateGuiGame();
            }

            // game is end. show result
            if (MainBoard.getWinTag() == BoardInterface::Tag::O) putToGui("player 1 win\n");
            else if (MainBoard.getWinTag() == BoardInterface::Tag::X) putToGui("player 2 win\n");
            else if (MainBoard.getWinTag() == BoardInterface::Tag::Tie) putToGui("Tie\n");
            else{}
        } 

   private:
        void updateGuiGame()
        {
            gui->updateGame(MainBoard);
        }

        bool playOneRound(AIInterface *user, BoardInterface::Tag tag, AIInterface *enemy) // Todo (finish)
        {
            // calaculate a position
            auto pos = call(&AIInterface::queryWhereToPut, user, MainBoard); 

            // check if pos is legal. if not, return false and the user lose
            /* need to check (1) if the position has no tags occupied 
                             (2) the position is in the correct subboard */
            if (MainBoard.get(pos.first, pos.second) != BoardInterface::Tag::None) {
                putToGui("(%d,%d) is illegal\n", pos.first, pos.second);
                return false;
            }
            if (!m_ship_size.empty()){
                std::vector<int>::iterator it = m_ship_size.end() - 2;
                if (MainBoard.sub((*it)%3, (*(it+1))%3).full()){ // in this case, position can be anywhere.
                    // check if the position is in range
                    if (pos.first < 0 || pos.first > 8 || pos.second < 0 || pos.second > 8) {
                        putToGui("(%d,%d) is illegal\n", pos.first, pos.second);
                        return false;
                    }
                }
                else{
                    // check if position is in the correct subboard 
                    if ((*it)%3 != pos.first/3 || (*(it+1))%3 != pos.second/3){
                        putToGui("(%d,%d) is illegal\n", pos.first, pos.second);
                        return false;
                    } 
                }
            }

            // if pos is legal, update MainBoard.
            // m_ship_size is a vector store the previous move
            /*Note we need to (1) update the wintag of subboard first an then
                              (2) update the wintag of MainBoard*/
            m_ship_size.push_back(pos.first);
            m_ship_size.push_back(pos.second);
            m_size += 2;
            MainBoard.get(pos.first, pos.second) = tag;
            checkPlayerWin(tag, MainBoard.sub(pos.first/3, pos.second/3));
            if (MainBoard.sub(pos.first/3, pos.second/3).full() == true &&
                MainBoard.sub(pos.first/3, pos.second/3).getWinTag() == BoardInterface::Tag::None) 
            {
                MainBoard.sub(pos.first/3, pos.second/3).setWinTag(BoardInterface::Tag::Tie);
            }
            checkPlayerWin(tag, MainBoard);

            // tell enemy where you move
            enemy->callbackReportEnemy(pos.first, pos.second);

            return true;
        }

        bool checkGameover() // Todo (finish)
        {
            // if there is a player win or the MainBoard is full, return true, else return false
            if (MainBoard.getWinTag() ==  BoardInterface::Tag::O|| MainBoard.getWinTag() ==  BoardInterface::Tag::X) return true;
            else{
                // check if MainBoard is full. if not, return false, else return true
                int i, j;
                for (i=0; i<3; ++i){
                    for (j=0; j<3; ++j){
                        if (MainBoard.sub(i, j).full() == false) return false;
                    }
                }
                MainBoard.setWinTag(BoardInterface::Tag::Tie);
                return true;
            }
        }

        // This function is used to check if a player win in a board. (can be Board or UltraBoard)
       void checkPlayerWin(BoardInterface::Tag T, BoardInterface& board){ // Todo(finish)
            if (board.getWinTag() != BoardInterface::Tag::None) return;
            
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

        bool prepareState()
        {
            call(&AIInterface::init, m_P1, true);
            call(&AIInterface::init, m_P2, false);
            return true;
        }

        template<typename Func ,typename... Args, 
            std::enable_if_t< std::is_void<
                    std::invoke_result_t<Func, AIInterface, Args...>
                >::value , int> = 0 >
        void call(Func func, AIInterface *ptr, Args... args)
        {
            std::future_status status;
            auto val = std::async(std::launch::async, func, ptr, args...);
            status = val.wait_for(std::chrono::milliseconds(m_runtime_limit));

            if( status != std::future_status::ready )
            {
                exit(-1);
            }
            val.get();
        }

        template<typename Func ,typename... Args, 
            std::enable_if_t< std::is_void<
                    std::invoke_result_t<Func, AIInterface, Args...>
                >::value == false, int> = 0 >
        auto call(Func func, AIInterface *ptr, Args... args)
            -> std::invoke_result_t<Func, AIInterface, Args...>
        {
            std::future_status status;
            auto val = std::async(std::launch::async, func, ptr, args...);
            status = val.wait_for(std::chrono::milliseconds(m_runtime_limit));

            if( status != std::future_status::ready )
            {
                exit(-1);
            }
            return val.get();
        }

        void putToGui(const char *fmt, ...)
        {
            va_list args1;
            va_start(args1, fmt);
            va_list args2;
            va_copy(args2, args1);
            std::vector<char> buf(1+std::vsnprintf(nullptr, 0, fmt, args1));
            va_end(args1);
            std::vsnprintf(buf.data(), buf.size(), fmt, args2);
            va_end(args2);

            if( buf.back() == 0 ) buf.pop_back();
            gui->appendText( std::string(buf.begin(), buf.end()) );
        }

        bool checkAI(AIInterface *ptr) 
        {
            return ptr->abi() == AI_ABI_VER;
        }

        int m_size;
        std::vector<int> m_ship_size; // I guess it is used to store the last move
        std::chrono::milliseconds m_runtime_limit;

        AIInterface *m_P1;
        AIInterface *m_P2;
        GUIInterface *gui;

        UltraBoard MainBoard;
    };
}
