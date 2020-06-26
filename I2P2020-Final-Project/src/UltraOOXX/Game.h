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
            putToGui("Hello world %d\n", 123);
            updateGuiGame();

            while (!checkGameover()) { // play the game until game over 
                round++;
                AIInterface *first = nullptr;
                AIInterface *second = nullptr;
                BoardInterface::Tag tag = BoardInterface::Tag::O;

                if (!playOneRound(first, tag, second)) {
                    
                }
                updateGuiGame();
            } 
        } 

   private:
        void updateGuiGame()
        {
            gui->updateGame(MainBoard);
        }

        bool playOneRound(AIInterface *user, BoardInterface::Tag tag, AIInterface *enemy) // Todo
        {
            auto pos = call(&AIInterface::queryWhereToPut, user, MainBoard); // calaculate a position
            return true;
        }

        bool checkGameover() // Todo (finish)
        {
            // if there is a player win or the MainBoard is full, return true, else return false
            // if statement also updata wintag if there is a player win
            if (checkPlayerWin(BoardInterface::Tag::O) || checkPlayerWin(BoardInterface::Tag::X)) return true;
            else{
                // check if MainBoard is full. if not, return false, else return true
                int i, j;
                for (i=0; i<3; ++i){
                    for (j=0; j<3; ++j){
                        if (MainBoard.get(i, j).full() == false) return false;
                    }
                }
                return true;
            }
        }

        // This function is used to check if a player win.
        bool checkPlayerWin(BoardInterface::Tag::T){ // Todo(finish)
            int i, j;
            // check row
            for (i=0; i<3; ++i){
                for (j=0; j<3; ++j){
                    if (MainBoard.state(i, j) != BoardInterface::Tag::T) break;
                    if (j == 2){ // if j == 2, means state(i, j) == T for j = 0, 1, 2 since there is no break
                        MainBoard.setWinTag(BoardInterface::Tag::T); 
                        return true;
                    }
                }
            }
            // check column
            for (i=0; i<3; ++i){
                for (j=0; j<3; ++j){
                    if (MainBoard.state(j, i) != BoardInterface::Tag::T) break;
                    if (j == 2){ // if j == 2, means state(j, i) == T for j = 0, 1, 2 since there is no break
                        MainBoard.setWinTag(BoardInterface::Tag::T);
                        return true;                        
                    }
                }
            }
            // check diagonal '\'
            for (i=0; i<3; ++i){
                if (MainBoard.state(i, i) != BoardInterface::Tag::T) break;
                if (i == 2){ // if i == 2, means state(i, i) == T for i = 0, 1, 2 since there is no break;
                    MainBoard.setWinTag(BoardInterface::Tag::T);
                    return true; 
                }
            }
            // check diagonal '/'
            for (i=0; i<3; ++i){
                if (MainBoard.state(i, 2-i) != BoardInterface::Tag::T) break;
                if (i == 2){ // if i == 2, means state(i, 2-i) == T for i = 0, 1, 2 since there is no break;
                    MainBoard.setWinTag(BoardInterface::Tag::T);
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
        std::vector<int> m_ship_size;
        std::chrono::milliseconds m_runtime_limit;

        AIInterface *m_P1;
        AIInterface *m_P2;
        GUIInterface *gui;

        UltraBoard MainBoard;
    };
}
