#pragma once

#include <iostream>
#include <memory>

// 전방 선언
class Context;

// 상태 인터페이스
class State {
public:
    virtual ~State() = default;
    virtual void enter(Context& ctx) = 0;
    virtual void update(Context& ctx) = 0;
    virtual void exit(Context& ctx) = 0;
};

// Context (상태 머신 본체)
class Context {
    std::unique_ptr<State> currentState;

public:
    void changeState(std::unique_ptr<State> newState) {
        if (currentState) {
            currentState->exit(*this);
        }
        currentState = std::move(newState);
        currentState->enter(*this);
    }

    void update() {
        if (currentState) {
            currentState->update(*this);
        }
    }

    void init(std::unique_ptr<State> startState) {
        changeState(std::move(startState));
    }
};

// 구체적인 상태 구현
class RunningState : public State {
public:
    void enter(Context& ctx) override { std::cout << "달리기 시작" << std::endl; }
    void update(Context& ctx) override { std::cout << "달리는 중..." << std::endl; }
    void exit(Context& ctx) override { std::cout << "달리기 멈춤" << std::endl; }
};

class IdleState : public State {
public:
    void enter(Context& ctx) override { std::cout << "대기 시작" << std::endl; }
    void update(Context& ctx) override {
        std::cout << "대기 중..." << std::endl;
        // 예시: 조건 만족 시 상태 변경
        // ctx.changeState(std::make_unique<RunningState>());
    }
    void exit(Context& ctx) override { std::cout << "대기 종료" << std::endl; }
};

